/*
 * Copyright (c) 2020 ARM Limited.
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
#ifndef ARM_COMPUTE_CLGEMMLOWPQUANTIZEDOWNINT32SCALEKERNEL_H
#define ARM_COMPUTE_CLGEMMLOWPQUANTIZEDOWNINT32SCALEKERNEL_H

#include "arm_compute/core/CL/ICLKernel.h"

namespace arm_compute
{
class ICLTensor;

/** OpenCL kernel used to quantize down the int32 accumulator values of GEMMLowp to QASYMM8/QASYMM8_SIGNED
 *
 * This kernel takes a final int32 accumulator value (the output of @ref CLGEMMLowpMatrixMultiplyKernel), and processes it to obtain the final QASYMM8/QASYMM8_SIGNED value.
 * The following computations will be performed by the kernel:
 *
 *  -# Add offset terms to final result
 *  -# Multiply each entry of result by result_mult_int
 *  -# Add bias to final result if bias tensor is not a nullptr
 *  -# Shift the int32 accumulator by result_shift
 *  -# Clamp the value between the specified min and max bounds
 *  -# Clamp the resulting int32 values:
 *  -#  -to the [0..255] range and cast to QASYMM8.
 *  -#  -to the [-128..127] range and cast to QASYMM8/SIGNED.
 *
 */
class CLGEMMLowpQuantizeDownInt32ScaleKernel : public ICLKernel
{
public:
    /** Constructor */
    CLGEMMLowpQuantizeDownInt32ScaleKernel();
    /** Prevent instances of this class from being copied (As this class contains pointers)*/
    CLGEMMLowpQuantizeDownInt32ScaleKernel(const CLGEMMLowpQuantizeDownInt32ScaleKernel &) = delete;
    /** Prevent instances of this class from being copied (As this class contains pointers)*/
    CLGEMMLowpQuantizeDownInt32ScaleKernel &operator=(const CLGEMMLowpQuantizeDownInt32ScaleKernel &) = delete;
    /** Allow instances of this class to be moved */
    CLGEMMLowpQuantizeDownInt32ScaleKernel(CLGEMMLowpQuantizeDownInt32ScaleKernel &&) = default;
    /** Allow instances of this class to be moved */
    CLGEMMLowpQuantizeDownInt32ScaleKernel &operator=(CLGEMMLowpQuantizeDownInt32ScaleKernel &&) = default;
    /** Initialise the kernel's input and output.
     *
     * @param[in]  input        Input tensor. Data type supported: S32
     * @param[in]  bias         Biases tensor. Only shared biases supported and it can be a nullptr if the biases addition is not required.
     *                          Biases are 1D tensor with dimensions [OFM]. Data type supported: Same as @p input.
     * @param[out] output       Output tensor. Data type supported: Data type supported: QASYMM8/QASYMM8_SIGNED
     * @param[in]  output_stage GEMMLowp output stage metadata.
     */
    void configure(const ICLTensor *input, const ICLTensor *bias, ICLTensor *output, const GEMMLowpOutputStageInfo *output_stage);
    /** Static function to check if given info will lead to a valid configuration of @ref CLGEMMLowpQuantizeDownInt32ScaleKernel
     *
     * @param[in] input        Input tensor. Data type supported: S32
     * @param[in] bias         Biases tensor. Only shared biases supported and it can be a nullptr if the biases addition is not required.
     *                         Biases are 1D tensor with dimensions [OFM]. Data type supported: Same as @p input.
     * @param[in] output       Output tensor. Data type supported: Data type supported: QASYMM8/QASYMM8_SIGNED
     * @param[in] output_stage GEMMLowp output stage metadata.
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *input, const ITensorInfo *bias, const ITensorInfo *output, const GEMMLowpOutputStageInfo *output_stage);

    // Inherited methods overridden:
    void run(const Window &window, cl::CommandQueue &queue) override;

private:
    const ICLTensor               *_input;
    const ICLTensor               *_bias;
    ICLTensor                     *_output;
    const GEMMLowpOutputStageInfo *_output_stage;
};
} // namespace arm_compute

#endif /* ARM_COMPUTE_CLGEMMLOWPQUANTIZEDOWNINT32SCALEKERNEL_H */