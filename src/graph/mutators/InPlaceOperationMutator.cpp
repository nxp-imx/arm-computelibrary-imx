/*
 * Copyright (c) 2018-2021 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "arm_compute/graph/mutators/InPlaceOperationMutator.h"

#include "arm_compute/core/Validate.h"
#include "arm_compute/graph/Graph.h"
#include "arm_compute/graph/Logger.h"

namespace arm_compute
{
namespace graph
{
namespace
{
// Check if the output edges of the parent node are separate tensors. If not,
// it means the same output is connected to multiple nodes and computations on
// these nodes cannot be done in-place.
bool output_edges_are_separate_tensors(Graph &g, const Edge *input_edge)
{
    const auto parent_node   = input_edge->producer();
    const auto input_tensor  = input_edge->tensor();
    const auto input_edge_id = input_edge->id();

    if(parent_node == nullptr)
    {
        return false;
    }

    const auto output_edges = parent_node->output_edges();

    // If the output is connected to only one edge, then computations can
    // be done in-place.
    if(output_edges.size() == 1)
    {
        return true;
    }

    return std::all_of(output_edges.begin(),
                       output_edges.end(),
                       [&](const EdgeID & edge_id)
    {
        // Skip check on current input edge
        if(edge_id == input_edge_id)
        {
            return true;
        }

        auto edge = g.edge(edge_id);
        return edge->tensor() != input_tensor;
    });
}

// If do in-place calculation, then need to use the new output and inherit original output's accessor
void set_new_output_and_inherit_accessor(std::unique_ptr<INode> &node, Tensor *orig_output, Tensor *new_output)
{
    ARM_COMPUTE_LOG_GRAPH_INFO("Switching to in-place computation for the node with ID : "
                               << node->id() << " and name : " << node->name() << std::endl);
    // Update accessor
    new_output->set_accessor(orig_output->extract_accessor());
    // Update output
    node->set_output_tensor(new_output->id(), 0);
}

// Try to mutate the node to perform the elementwise in-place calculation
void try_in_place_elementwise(std::unique_ptr<INode> &node)
{
    // Get input edge
    Edge *input0_edge = node->input_edge(0);
    Edge *input1_edge = node->input_edge(1);
    ARM_COMPUTE_ERROR_ON(input0_edge == nullptr || input1_edge == nullptr);

    auto input0_tensor = input0_edge->tensor();
    auto input1_tensor = input1_edge->tensor();
    ARM_COMPUTE_ERROR_ON(input0_tensor == nullptr || input1_tensor == nullptr);

    const auto shape0 = input0_tensor->desc().shape;
    const auto shape1 = input1_tensor->desc().shape;
    const auto qinfo0 = input0_tensor->desc().quant_info;
    const auto qinfo1 = input1_tensor->desc().quant_info;

    const TensorShape out_shape = TensorShape::broadcast_shape(shape0, shape1);
    // Inputs are not broadcast compatible
    if(out_shape.total_size() == 0)
    {
        return;
    }

    // Get current output tensor
    auto current_output_tensor = node->output(0);
    ARM_COMPUTE_ERROR_ON(current_output_tensor == nullptr);
    const auto qinfo_out = current_output_tensor->desc().quant_info;

    // Can do in place, if the input has same shape as output, has same quntisation info as output, and input doesn't have accessor.
    bool input0_can_in_place = !arm_compute::detail::have_different_dimensions(out_shape, shape0, 0) && (qinfo0 == qinfo_out) && (input0_tensor->accessor() == nullptr);
    bool input1_can_in_place = !arm_compute::detail::have_different_dimensions(out_shape, shape1, 0) && (qinfo1 == qinfo_out) && (input1_tensor->accessor() == nullptr);

    if(input0_can_in_place)
    {
        set_new_output_and_inherit_accessor(node, current_output_tensor, input0_tensor);
    }
    else if(input1_can_in_place)
    {
        set_new_output_and_inherit_accessor(node, current_output_tensor, input1_tensor);
    }
    else
    {
        ARM_COMPUTE_LOG_GRAPH_VERBOSE("Prevented in-place operation as there is an accessor bound to the input tensor or the quantization info are different.\n");
    }
}
} // namespace

const char *InPlaceOperationMutator::name()
{
    return "InPlaceOperationMutator";
}

IGraphMutator::MutationType InPlaceOperationMutator::type() const
{
    return IGraphMutator::MutationType::Backend;
}

void InPlaceOperationMutator::mutate(Graph &g)
{
    std::set<NodeType> in_place_nodes =
    {
        NodeType::ActivationLayer,
        NodeType::BatchNormalizationLayer,
        NodeType::EltwiseLayer,
        NodeType::UnaryEltwiseLayer,
        NodeType::PrintLayer
    };

    // Not interested in the order of nodes
    for(auto &node : g.nodes())
    {
        if(node && in_place_nodes.find(node->type()) != std::end(in_place_nodes))
        {
            // Get input edge
            Edge *input_edge = node->input_edge(0);

            // Check if parent has a single output if yes then force in place calculation else not
            if((input_edge != nullptr) && output_edges_are_separate_tensors(g, input_edge))
            {
                if(node->type() == NodeType::EltwiseLayer)
                {
                    try_in_place_elementwise(node);
                }
                else
                {
                    // Get current and new output tensors
                    auto current_output_tensor = node->output(0);
                    auto new_output_tensor     = input_edge->tensor();

                    ARM_COMPUTE_ERROR_ON(current_output_tensor == nullptr || new_output_tensor == nullptr);

                    // Prevent in-place operation if there is an accessor bound to the in-place tensor or quantization info are different
                    if(new_output_tensor->accessor() != nullptr || current_output_tensor->desc().quant_info != new_output_tensor->desc().quant_info)
                    {
                        ARM_COMPUTE_LOG_GRAPH_VERBOSE("Prevented in-place operation as there is an accessor bound to the input tensor or the quantization info are different.\n");
                    }
                    else
                    {
                        set_new_output_and_inherit_accessor(node, current_output_tensor, new_output_tensor);
                    }
                }
            }
        }
    }
}
} // namespace graph
} // namespace arm_compute
