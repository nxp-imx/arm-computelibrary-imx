/*
 * Copyright (c) 2018-2022 Arm Limited.
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
#ifndef ARM_COMPUTE_CPU_COPY_KERNEL_H
#define ARM_COMPUTE_CPU_COPY_KERNEL_H

#include "src/core/common/Macros.h"
#include "src/cpu/ICpuKernel.h"

namespace arm_compute
{
namespace cpu
{
namespace kernels
{
/** Kernel to perform a copy between two tensors */
class CpuCopyKernel : public ICpuKernel<CpuCopyKernel>
{
public:
    CpuCopyKernel() = default;
    ARM_COMPUTE_DISALLOW_COPY_ALLOW_MOVE(CpuCopyKernel);
    /** Configure kernel for a given list of arguments
     *
     * @param[in]  src     Source tensor. Data types supported: All
     * @param[out] dst     Destination tensor. Data types supported: same as @p src.
     * @param[in]  padding (Optional) Padding to be applied to the input tensor
     */
    void configure(const ITensorInfo *src, ITensorInfo *dst, const PaddingList &padding = PaddingList());
    /** Static function to check if given info will lead to a valid configuration
     *
     * Similar to @ref CpuCopyKernel::configure()
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *src, const ITensorInfo *dst, const PaddingList &padding = PaddingList());

    // Inherited methods overridden:
    void        run_op(ITensorPack &tensors, const Window &window, const ThreadInfo &info) override;
    const char *name() const override;

private:
    PaddingList _padding{};
};
} // namespace kernels
} // namespace cpu
} // namespace arm_compute
#endif /* ARM_COMPUTE_CPU_COPY_KERNEL_H */
