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
#ifndef ARM_COMPUTE_CL_PERMUTE_KERNEL_H
#define ARM_COMPUTE_CL_PERMUTE_KERNEL_H

#include "src/core/common/Macros.h"
#include "src/gpu/cl/ClCompileContext.h"
#include "src/gpu/cl/IClKernel.h"

namespace arm_compute
{
namespace opencl
{
namespace kernels
{
/** OpenCL kernel to perform tensor permutation.
 *
 * Permutes given a permutation vector
 */
class ClPermuteKernel : public IClKernel
{
public:
    ClPermuteKernel();
    ARM_COMPUTE_DISALLOW_COPY_ALLOW_MOVE(ClPermuteKernel);
    /** Set the src and dst of the kernel.
     *
     * @note Arbitrary permutation vectors are supported with rank not greater than 4
     *
     * @param[in] compile_context The compile context to be used.
     * @param[in] src             The src tensor info. Data types supported: All.
     * @param[in] dst             The dst tensor info. Data types supported: Same as @p src
     * @param[in] perm            Permutation vector
     */
    void configure(const CLCompileContext  &compile_context,
                   const ITensorInfo       *src,
                   ITensorInfo             *dst,
                   const PermutationVector &perm);
    /** Static function to check if given info will lead to a valid configuration
     *
     * Similar to @ref ClPermuteKernel::configure()
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *src, const ITensorInfo *dst, const PermutationVector &perm);

    // Inherited methods overridden:
    void run_op(ITensorPack &tensors, const Window &window, cl::CommandQueue &queue) override;

private:
    PermutationVector _perm{};
};
} // namespace kernels
} // namespace opencl
} // namespace arm_compute
#endif /* ARM_COMPUTE_CL_PERMUTE_KERNEL_H */
