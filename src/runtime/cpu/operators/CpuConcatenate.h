/*
 * Copyright (c) 2021 Arm Limited.
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
#ifndef ARM_COMPUTE_CPU_CONCATENATE_H
#define ARM_COMPUTE_CPU_CONCATENATE_H

#include "src/core/cpu/ICpuKernel.h"
#include "src/runtime/cpu/ICpuOperator.h"

#include <vector>

namespace arm_compute
{
namespace cpu
{
/** Basic function to execute concatenate tensors along a given axis. This function calls the following kernels:
 *
 * -# @ref CpuConcatenateWidthKernel (if underlying concatenation axis is 0).
 * -# @ref CpuConcatenateHeightKernel (if underlying concatenation axis is 1).
 * -# @ref CpuConcatenateDepthKernel (if underlying concatenation axis is 2).
 * -# @ref CpuConcatenateBatchKernel (if underlying concatenation axis is 3).
 */
class CpuConcatenate : public ICpuOperator
{
public:
    /** Constructor */
    CpuConcatenate();
    /** Configure operator for a given list of arguments
     *
     * @note Input and output tensor dimensions preconditions defer depending on the concatenation axis.
     * @note Preconditions can be found respectively at @ref CpuConcatenateWidthKernel, @ref CpuConcatenateHeightKernel, @ref CpuConcatenateDepthKernel and @ref CpuConcatenateBatchKernel.
     *
     * @param[in,out] srcs_vector The vectors containing all the tensors to concatenate. Data types supported: QASYMM8/QASYMM8_SIGNED/F16/F32.
     * @param[out]    dst         Output tensor. Data types supported: Same as @p srcs_vector.
     * @param[in]     axis        Concatenation axis. Supported underlying concatenation axis are 0, 1, 2 and 3.
     */
    void configure(const std::vector<const ITensorInfo *> &srcs_vector, ITensorInfo *dst, size_t axis);
    /** Static function to check if given info will lead to a valid configuration of @ref NEConcatenateLayer
     *
     * @note Input and output tensor dimensions preconditions defer depending on the concatenation axis.
     * @note Preconditions can be found respectively at @ref CpuConcatenateWidthKernel, @ref CpuConcatenateHeightKernel, @ref CpuConcatenateDepthKernel and @ref CpuConcatenateBatchKernel.
     *
     * @param[in] srcs_vector The vectors containing all the tensors info to concatenate. Data types supported: QASYMM8/QASYMM8_SIGNED/F16/F32.
     * @param[in] dst         Output tensor info. Data types supported: Same as @p srcs_vector.
     * @param[in] axis        Concatenation axis. Supported underlying concatenation axis are 0, 1, 2 and 3.
     *
     * @return a status
     */
    static Status validate(const std::vector<const ITensorInfo *> &srcs_vector, const ITensorInfo *dst, size_t axis);

    // Inherited methods overridden:
    void run(ITensorPack &tensors) override;

private:
    std::vector<std::unique_ptr<ICpuKernel>> _concat_kernels;
    unsigned int                             _num_srcs;
    unsigned int                             _axis;
};
} // namespace cpu
} // namespace arm_compute
#endif /* ARM_COMPUTE_CPU_CONCATENATE_H */
