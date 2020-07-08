/*
 * Copyright (c) 2017-2020 Arm Limited.
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
#ifndef ARM_COMPUTE_NESOFTMAXLAYER_H
#define ARM_COMPUTE_NESOFTMAXLAYER_H

#include "arm_compute/core/NEON/kernels/NEFillBorderKernel.h"
#include "arm_compute/core/NEON/kernels/NESoftmaxLayerKernel.h"
#include "arm_compute/runtime/IFunction.h"
#include "arm_compute/runtime/MemoryGroup.h"
#include "arm_compute/runtime/NEON/functions/NEFlattenLayer.h"
#include "arm_compute/runtime/NEON/functions/NEReshapeLayer.h"
#include "arm_compute/runtime/Tensor.h"

namespace arm_compute
{
class ITensor;

/** Basic function to compute a SoftmaxLayer and a Log SoftmaxLayer.
 *
 * Softmax is calculated by :
 * @f[ out = exp((x - max(x)) * beta) / sum(exp((x - max(x)) * beta)) @f]
 *
 * Log Softmax is calculated by :
 * @f[ out = (x - max(x) * beta) - log(\sum{e^{x - max(x) * beta}}) @f]
 *
 * This function runs the following kernels:
 * -# @ref NEFillBorderKernel
 * -# @ref NELogits1DMaxKernel
 * -# @ref NELogits1DSoftmaxKernel
 * And if the reduce_end_axis is not 0 or -input_num_dimensions, the function will use one of the the following kernels
 * to reshape the input and perform softmax on the reshaped input:
 * -# @ref NEFlattenLayerKernel
 * -# @ref NEReshapeLayerKernel
 */
template <bool IS_LOG = false>
class NESoftmaxLayerGeneric : public IFunction
{
public:
    /** Constructor */
    NESoftmaxLayerGeneric(std::shared_ptr<IMemoryManager> memory_manager = nullptr);
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    NESoftmaxLayerGeneric(const NESoftmaxLayerGeneric &) = delete;
    /** Default move constructor */
    NESoftmaxLayerGeneric(NESoftmaxLayerGeneric &&) = default;
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    NESoftmaxLayerGeneric &operator=(const NESoftmaxLayerGeneric &) = delete;
    /** Default move assignment operator */
    NESoftmaxLayerGeneric &operator=(NESoftmaxLayerGeneric &&) = default;
    /** Set the input and output tensors.
     *
     * @param[in,out] input           Source tensor. Data types supported: QASYMM8/QASYMM8_SIGNED/F16/F32. If the width is not a
     *                                multiple of the internal processing block size, @ref NEFillBorderKernel replicates the
     *                                last value of each row to the nearest multiple.
     * @param[out]    output          Destination tensor. Data types supported: same as @p input.
     * @param[in]     beta            (Optional) A scaling factor for the exponent.
     * @param[in]     reduce_end_axis (Optional) The last axis of the first n dimensions (inclusive)to reduce. Defaults to 0.
     *                   It has the purpose of squashing together the first n dimensions till (including) the @p reduce_end_axis. For instance, given a [2x3x4x5] image,
     *                   when @p reduce_end_axis is 1, the reduction will be applied to axes 0 and 1, and the Softmax op will be applied on each of the [2x3] planes of the input image.
     *                   Negative index is used to specify axis from the end (e.g. -1 for the last axis).
     *                   Must be in range [-input_num_dimensions, input_num_dimensions).
     */
    void configure(ITensor *input, ITensor *output, float beta = 1.0f, int32_t reduce_end_axis = 0);
    /** Static function to check if given info will lead to a valid configuration of @ref NESoftmaxLayer
     *
     * @param[in] input           Source tensor info. Data types supported: QASYMM8/QASYMM8_SIGNED/F16/F32.
     * @param[in] output          Destination tensor info. Data types supported: same as @p input
     * @param[in] beta            (Optional) A scaling factor for the exponent.
     * @param[in] reduce_end_axis (Optional) The last axis of the first n dimensions (inclusive)to reduce. Defaults to 0.
     *                   It has the purpose of squashing together the first n dimensions till (including) the @p reduce_end_axis. For instance, given a [2x3x4x5] image,
     *                   when @p reduce_end_axis is 1, the reduction will be applied to axes 0 and 1, and the Softmax op will be applied on each of the [2x3] planes of the input image.
     *                   Negative index is used to specify axis from the end (e.g. -1 for the last axis).
     *                   Must be in range [-input_num_dimensions, input_num_dimensions).
     * @return a status
     */
    static Status validate(const ITensorInfo *input, const ITensorInfo *output, float beta = 1.0f, int32_t reduce_end_axis = 0);

    // Inherited methods overridden:
    void run() override;

private:
    /** Utility method to configure the kernels needed to flatten the input
     * tensor.
     *
     * @note This function changes the internal state of this class. In particular,
     * it initializes the kernel @p _flatten_kernel and the tensors @p _input_flat and
     * @p _output_flat
     *
     * @param[in] input           Original source tensor.
     * @param[in] output          Original destination tensor.
     * @param[in] reduce_end_axis (Optional) The last axis of the first n dimensions (inclusive)to reduce. Defaults to 0.
     *                   It has the purpose of squashing together the first n dimensions till (including) the @p reduce_end_axis. For instance, given a [2x3x4x5] image,
     *                   when @p reduce_end_axis is 1, the reduction will be applied to axes 0 and 1, and the Softmax op will be applied on each of the [2x3] planes of the input image.
     *                   Negative index is used to specify axis from the end (e.g. -1 for the last axis).
     *                   Must be in range [-input_num_dimensions, input_num_dimensions).
     */
    void configure_reshape_input_kernel(const ITensor *input, const ITensor *output, int32_t reduce_end_axis);

    MemoryGroup                     _memory_group;
    NELogits1DMaxKernel             _max_kernel;
    NELogits1DSoftmaxKernel<IS_LOG> _softmax_kernel;
    std::unique_ptr<IFunction>      _flat_or_reshape_ptr;
    NEFillBorderKernel              _fill_border_kernel;
    NEReshapeLayer                  _reshape;
    Tensor                          _max;
    Tensor                          _tmp;
    Tensor                          _input_flattened;
    Tensor                          _output_flattened;
    bool                            _needs_flattening;
};

using NESoftmaxLayer    = NESoftmaxLayerGeneric<false>;
using NELogSoftmaxLayer = NESoftmaxLayerGeneric<true>;

} // namespace arm_compute
#endif /* ARM_COMPUTE_NESOFTMAXLAYER_H */
