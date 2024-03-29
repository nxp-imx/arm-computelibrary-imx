/*
 * Copyright (c) 2019-2021 Arm Limited.
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
#ifndef ARM_COMPUTE_CL_GEMM_RESHAPED_ONLY_RHS_KERNEL_CONFIGURATION_H
#define ARM_COMPUTE_CL_GEMM_RESHAPED_ONLY_RHS_KERNEL_CONFIGURATION_H

#include "src/gpu/cl/kernels/gemm/IClGemmKernelConfig.h"
#include "src/gpu/cl/kernels/gemm/reshaped_only_rhs/ClGemmDefaultConfigReshapedRhsOnlyBifrost.h"
#include "src/gpu/cl/kernels/gemm/reshaped_only_rhs/ClGemmDefaultConfigReshapedRhsOnlyValhall.h"

#include <memory>

namespace arm_compute
{
namespace opencl
{
namespace kernels
{
namespace gemm
{
/** CLGEMMReshapedOnlyRHS factory class */
class ClGemmReshapedOnlyRhsKernelConfigurationFactory final
{
public:
    /** Static method to call the CLGEMMReshapedOnlyRHS kernel configuration class accordingly with the GPU target
     *
     * @param[in] gpu GPU target
     *
     * @return CLGEMMReshapedOnlyRHS kernel configuration class
     */
    static std::unique_ptr<IClGemmKernelConfig> create(GPUTarget gpu)
    {
        switch (get_arch_from_target(gpu))
        {
            case GPUTarget::MIDGARD:
            case GPUTarget::BIFROST:
                return std::make_unique<ClGemmDefaultConfigReshapedRhsOnlyBifrost>(gpu);
            case GPUTarget::VALHALL:
                return std::make_unique<ClGemmDefaultConfigReshapedRhsOnlyValhall>(gpu);
            default:
                ARM_COMPUTE_ERROR("Not supported GPU target");
        }
    }
};
} // namespace gemm
} // namespace kernels
} // namespace opencl
} // namespace arm_compute
#endif /* ARM_COMPUTE_CL_GEMM_RESHAPED_ONLY_RHS_KERNEL_CONFIGURATION_H */
