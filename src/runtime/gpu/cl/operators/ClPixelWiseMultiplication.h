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
#ifndef ARM_COMPUTE_CL_PIXELWISE_MULTIPLICATION_H
#define ARM_COMPUTE_CL_PIXELWISE_MULTIPLICATION_H

#include "src/core/gpu/cl/ClCompileContext.h"
#include "src/runtime/gpu/cl/IClOperator.h"

#include <memory>

namespace arm_compute
{
namespace opencl
{
/** Basic function to run @ref opencl::ClPixelWiseMultiplication. */
class ClPixelWiseMultiplication : public IClOperator
{
public:
    /** Default Constructor */
    ClPixelWiseMultiplication() = default;
    /** Initialise the kernel's sources, dst and convertion policy.
     *
     * Valid configurations (src1,src2) -> Output :
     *
     *   - (U8,U8)                         -> U8
     *   - (U8,U8)                         -> S16
     *   - (U8,S16)                        -> S16
     *   - (S16,U8)                        -> S16
     *   - (S16,S16)                       -> S16
     *   - (F16,F16)                       -> F16
     *   - (F32,F32)                       -> F32
     *   - (QASYMM8,QASYMM8)               -> QASYMM8
     *   - (QASYMM8_SIGNED,QASYMM8_SIGNED) -> QASYMM8_SIGNED
     *   - (QSYMM16,QSYMM16)               -> QSYMM16
     *   - (QSYMM16,QSYMM16)               -> S32
     *
     * @param[in]      compile_context The compile context to be used.
     * @param[in, out] src1            An src tensor info. Data types supported: U8/QASYMM8/QASYMM8_SIGNED/S16/QSYMM16/F16/F32.
     *                                 The src tensor is [in, out] because its TensorInfo might be modified inside the kernel in case of broadcasting of dimension 0.
     * @param[in, out] src2            An src tensor info. Data types supported: U8/QASYMM8/QASYMM8_SIGNED/S16/QSYMM16/F16/F32.
     *                                 The src tensor is [in, out] because its TensorInfo might be modified inside the kernel in case of broadcasting of dimension 0.
     * @param[out]     dst             The dst tensor info. Data types supported: U8/QASYMM8/QASYMM8_SIGNED/S16/QSYMM16/F16/F32.
     * @param[in]      scale           Scale to apply after multiplication.
     *                                 Scale must be positive and its value must be either 1/255 or 1/2^n where n is between 0 and 15.
     * @param[in]      overflow_policy Overflow policy. Supported overflow policies: Wrap, Saturate
     * @param[in]      rounding_policy Rounding policy. Supported rounding modes: to zero, to nearest even.
     * @param[in]      act_info        (Optional) Activation layer information in case of a fused activation.
     */
    void configure(const CLCompileContext &compile_context, ITensorInfo *src1, ITensorInfo *src2, ITensorInfo *dst, float scale,
                   ConvertPolicy overflow_policy, RoundingPolicy rounding_policy, const ActivationLayerInfo &act_info = ActivationLayerInfo());
    /** Static function to check if given info will lead to a valid configuration of @ref ClPixelWiseMultiplication
     *
     * Valid configurations (src1,src2) -> Output :
     *
     *   - (U8,U8)                         -> U8
     *   - (U8,U8)                         -> S16
     *   - (U8,S16)                        -> S16
     *   - (S16,U8)                        -> S16
     *   - (S16,S16)                       -> S16
     *   - (F16,F16)                       -> F16
     *   - (F32,F32)                       -> F32
     *   - (QASYMM8,QASYMM8)               -> QASYMM8
     *   - (QASYMM8_SIGNED,QASYMM8_SIGNED) -> QASYMM8_SIGNED
     *   - (QSYMM16,QSYMM16)               -> QSYMM16
     *   - (QSYMM16,QSYMM16)               -> S32
     *
     *
     * @param[in] src1            An src tensor info. Data types supported: U8/QASYMM8/QASYMM8_SIGNED/S16/QSYMM16/F16/F32.
     * @param[in] src2            An src tensor info. Data types supported: U8/QASYMM8/QASYMM8_SIGNED/S16/QSYMM16/F16/F32.
     * @param[in] dst             The dst tensor info. Data types supported: U8/QASYMM8/QASYMM8_SIGNED/S16/QSYMM16/F16/F32.
     * @param[in] scale           Scale to apply after multiplication.
     *                            Scale must be positive and its value must be either 1/255 or 1/2^n where n is between 0 and 15.
     * @param[in] overflow_policy Overflow policy. Supported overflow policies: Wrap, Saturate
     * @param[in] rounding_policy Rounding policy. Supported rounding modes: to zero, to nearest even.
     * @param[in] act_info        (Optional) Activation layer information in case of a fused activation.
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *src1, const ITensorInfo *src2, const ITensorInfo *dst, float scale,
                           ConvertPolicy overflow_policy, RoundingPolicy rounding_policy, const ActivationLayerInfo &act_info = ActivationLayerInfo());

    // Inherited methods overridden:
    void run(ITensorPack &tensors) override;

private:
    std::unique_ptr<ICLKernel> _border_handler{ nullptr };
};

/** Basic function to run @ref opencl::ClComplexPixelWiseMultiplication. */
class ClComplexPixelWiseMultiplication : public IClOperator
{
public:
    /** Default Constructor */
    ClComplexPixelWiseMultiplication() = default;
    /** Initialise the kernel's sources, dst.
     *
     * @param[in]      compile_context The compile context to be used.
     * @param[in, out] src1            An src tensor info. Data types supported: F16/F32. Number of channels supported: 2.
     *                                 The src tensor is [in, out] because its TensorInfo might be modified inside the kernel in case of broadcasting of dimension 0.
     * @param[in, out] src2            An src tensor info. Data types supported: same as @p src1. Number of channels supported: same as @p src1.
     *                                 The src tensor is [in, out] because its TensorInfo might be modified inside the kernel in case of broadcasting of dimension 0.
     * @param[out]     dst             The dst tensor info, Data types supported: same as @p src1. Number of channels supported: same as @p src1.
     * @param[in]      act_info        (Optional) Activation layer information in case of a fused activation.
     */
    void configure(const CLCompileContext &compile_context, ITensorInfo *src1, ITensorInfo *src2, ITensorInfo *dst, const ActivationLayerInfo &act_info = ActivationLayerInfo());
    /** Static function to check if given info will lead to a valid configuration of @ref ClComplexPixelWiseMultiplication
     *
     * @param[in] src1     An src tensor info. Data types supported: F16/F32. Number of channels supported: 2.
     * @param[in] src2     An src tensor info. Data types supported: same as @p src1. Number of channels supported: same as @p src1.
     * @param[in] dst      The dst tensor info, Data types supported: same as @p src1. Number of channels supported: same as @p src1.
     * @param[in] act_info (Optional) Activation layer information in case of a fused activation.
     */
    static Status validate(const ITensorInfo *src1, const ITensorInfo *src2, const ITensorInfo *dst, const ActivationLayerInfo &act_info = ActivationLayerInfo());

    // Inherited methods overridden:
    void run(ITensorPack &tensors) override;

private:
    std::unique_ptr<ICLKernel> _border_handler{ nullptr };
};
} // namespace opencl
} // namespace arm_compute
#endif /* ARM_COMPUTE_CL_PIXELWISE_MULTIPLICATION_H */