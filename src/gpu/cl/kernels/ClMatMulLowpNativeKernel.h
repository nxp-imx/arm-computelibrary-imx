/*
 * Copyright (c) 2023 Arm Limited.
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
#ifndef ACL_SRC_GPU_CL_KERNELS_CLMATMULLOWPNATIVEKERNEL
#define ACL_SRC_GPU_CL_KERNELS_CLMATMULLOWPNATIVEKERNEL

#include "arm_compute/core/ActivationLayerInfo.h"
#include "src/core/common/Macros.h"
#include "src/gpu/cl/ClCompileContext.h"
#include "src/gpu/cl/IClKernel.h"

namespace arm_compute
{
// Forward declerations
struct MatMulKernelInfo;
namespace opencl
{
namespace kernels
{
class ClMatMulLowpNativeKernel : public IClKernel
{
public:
    ClMatMulLowpNativeKernel();
    ARM_COMPUTE_DISALLOW_COPY_ALLOW_MOVE(ClMatMulLowpNativeKernel);
    /** Initialise the kernel's input and output.
     *
     * @param[in]  compile_context    The compile context to be used.
     * @param[in]  lhs                Input tensor info for the LHS matrix. Data type supported: QASYMM8_SIGNED/QASYMM8.
     *                                Dimensions above 2 are collapsed onto dimension 2 and represent the batch.
     * @param[in]  rhs                Input tensor info for the RHS matrix. Data type supported: same as @p lhs.
     *                                Dimensions above 2 are collapsed onto dimension 2 and represent the batch.
     * @param[in]  bias               Bias tensor info. Can be nullptr. Data type supported: S32.
     * @param[out] dst                Output tensor info. Data type supported: same as @p lhs
     * @param[in]  matmul_kernel_info Attributes for Batch MatMul Kernel
     * @param[in]  act_info           (Optional) Class containing information about fused activation function.
     */
    void configure(const ClCompileContext &compile_context, ITensorInfo *lhs, ITensorInfo *rhs, ITensorInfo *bias, ITensorInfo *dst, const MatMulKernelInfo &matmul_kernel_info,
                   const ActivationLayerInfo &act_info = ActivationLayerInfo());
    /** Static function to check if given info will lead to a valid configuration
     *
     * Similar to @ref ClMatMulLowpNativeKernel::configure()
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *lhs, const ITensorInfo *rhs, const ITensorInfo *bias, const ITensorInfo *dst, const MatMulKernelInfo &matmul_kernel_info,
                           const ActivationLayerInfo &act_info = ActivationLayerInfo());

    // Inherited methods overridden:
    void run_op(ITensorPack &tensors, const Window &window, cl::CommandQueue &queue) override;
};
} // namespace kernels
} // namespace opencl
} // namespace arm_compute
#endif /* ACL_SRC_GPU_CL_KERNELS_CLMATMULLOWPNATIVEKERNEL */
