/*
 * Copyright (c) 2017-2018 ARM Limited.
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
#include "arm_compute/runtime/NEON/functions/NEDeconvolutionLayer.h"

#include "arm_compute/core/Helpers.h"
#include "arm_compute/core/Utils.h"
#include "arm_compute/core/Validate.h"
#include "arm_compute/core/utils/misc/ShapeCalculator.h"

using namespace arm_compute;
using namespace arm_compute::misc::shape_calculator;

NEDeconvolutionLayer::NEDeconvolutionLayer(std::shared_ptr<IMemoryManager> memory_manager) // NOLINT
    : _memory_group(std::move(memory_manager)),
      _conv_f(),
      _upsample_f(),
      _scaled_output(),
      _input(nullptr),
      _info(),
      _inner_border(),
      _is_prepared(false)
{
}

Status NEDeconvolutionLayer::validate(const ITensorInfo *input, const ITensorInfo *weights, const ITensorInfo *bias, const ITensorInfo *output, const PadStrideInfo &info,
                                      unsigned int inner_border_right, unsigned int inner_border_top)
{
    ARM_COMPUTE_RETURN_ERROR_ON_NULLPTR(input, weights, output);
    ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(input, 1, DataType::F32);
    ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(weights, 1, DataType::F32);
    ARM_COMPUTE_RETURN_ERROR_ON(weights->dimension(0) != weights->dimension(1));
    ARM_COMPUTE_RETURN_ERROR_ON(weights->dimension(0) < 1);
    ARM_COMPUTE_RETURN_ERROR_ON(!info.padding_is_symmetric());

    const unsigned int stride_x = info.stride().first;
    const unsigned int stride_y = info.stride().second;

    ARM_COMPUTE_RETURN_ERROR_ON_MSG(inner_border_right > stride_x - 1, "inner_border_right must be smaller than stride_x");
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(inner_border_top > stride_y - 1, "inner_border_top must be smaller than stride_y");

    auto out_dims = deconvolution_output_dimensions(input->dimension(0), input->dimension(1), weights->dimension(0), weights->dimension(1),
                                                    info.pad().first, info.pad().second, inner_border_right, inner_border_top, stride_x, stride_y);

    ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(input, weights, bias);
    ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_FIXED_POINT(input, weights, bias);

    if(bias != nullptr)
    {
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(input, bias);
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_FIXED_POINT(input, bias);
    }

    if(output->tensor_shape().total_size() > 0)
    {
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(input, output);
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_FIXED_POINT(input, output);

        const TensorShape output_shape = deconvolution_output_shape(out_dims, input->tensor_shape(), weights->tensor_shape());

        ARM_COMPUTE_RETURN_ERROR_ON_MSG(output->dimension(Window::DimX) != output_shape.x(), "Output's width is invalid.");
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(output->dimension(Window::DimY) != output_shape.y(), "Output's height is invalid.");
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(output->dimension(Window::DimZ) != output_shape.z(), "Output's depth is invalid.");
    }

    TensorInfo scale_out_info(input->clone()->set_is_resizable(true).reset_padding().set_tensor_shape(compute_deconvolution_shape(*input, stride_x, stride_y, inner_border_right, inner_border_top,
                                                                                                      info)));
    const PadStrideInfo conv_info(1, 1, 0, 0, 0, 0, DimensionRoundingType::CEIL);

    for(size_t i = 2; i < Coordinates::num_max_dimensions; ++i)
    {
        ARM_COMPUTE_RETURN_ERROR_ON(input->dimension(i) != scale_out_info.dimension(i));
    }

    ARM_COMPUTE_RETURN_ON_ERROR(NEConvolutionLayer::validate(&scale_out_info, weights, bias, output, conv_info, WeightsInfo()));

    return Status{};
}

void NEDeconvolutionLayer::configure(ITensor *input, const ITensor *weights, const ITensor *bias, ITensor *output, const PadStrideInfo &info,
                                     unsigned int inner_border_right, unsigned int inner_border_top)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(input, weights, output);

    _input        = input;
    _info         = info;
    _inner_border = std::make_pair(inner_border_right, inner_border_top);
    _is_prepared  = false;

    const unsigned int stride_x = info.stride().first;
    const unsigned int stride_y = info.stride().second;

    // Perform validation step
    ARM_COMPUTE_ERROR_THROW_ON(NEDeconvolutionLayer::validate(input->info(), weights->info(), bias == nullptr ? nullptr : bias->info(), output->info(), info, inner_border_right, inner_border_top));

    _memory_group.manage(&_scaled_output);

    // configure scale function
    // Init and allocate intermmidiate tensor for output, same size as input but the first two axis are the same as the output tensor
    const TensorInfo scale_out_info(compute_deconvolution_shape(*input->info(), stride_x, stride_y, inner_border_right, inner_border_top, info), 1, input->info()->data_type(),
                                    input->info()->fixed_point_position());
    _scaled_output.allocator()->init(scale_out_info);

    // setup the function to convolve the upscaled output
    const PadStrideInfo conv_info(1, 1, 0, 0, 0, 0, DimensionRoundingType::CEIL);
    _conv_f.configure(&_scaled_output, weights, bias, output, conv_info);

    // Allocate auxiliary tensors
    _scaled_output.allocator()->allocate();

    // configure upsample function
    _upsample_f.configure(input, &_scaled_output, info, inner_border_right, inner_border_top);
}

void NEDeconvolutionLayer::run()
{
    prepare();

    _memory_group.acquire();

    _upsample_f.run();
    _conv_f.run();

    _memory_group.release();
}

void NEDeconvolutionLayer::prepare()
{
    if(!_is_prepared)
    {
        _conv_f.prepare();
        _is_prepared = true;
    }
}