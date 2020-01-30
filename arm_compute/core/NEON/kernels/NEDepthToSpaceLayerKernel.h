/*
 * Copyright (c) 2019 ARM Limited.
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
#ifndef __ARM_COMPUTE_NEDEPTHTOSPACELAYERKERNEL_H__
#define __ARM_COMPUTE_NEDEPTHTOSPACELAYERKERNEL_H__

#include "arm_compute/core/NEON/INEKernel.h"

namespace arm_compute
{
class ITensor;

/** Interface for the depth to space kernel */
class NEDepthToSpaceLayerKernel : public INEKernel
{
public:
    const char *name() const override
    {
        return "NEDepthToSpaceLayerKernel";
    }
    /** Default constructor */
    NEDepthToSpaceLayerKernel();
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    NEDepthToSpaceLayerKernel(const NEDepthToSpaceLayerKernel &) = delete;
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    NEDepthToSpaceLayerKernel &operator=(const NEDepthToSpaceLayerKernel &) = delete;
    /** Allow instances of this class to be moved */
    NEDepthToSpaceLayerKernel(NEDepthToSpaceLayerKernel &&) = default;
    /** Allow instances of this class to be moved */
    NEDepthToSpaceLayerKernel &operator=(NEDepthToSpaceLayerKernel &&) = default;
    /** Default destructor */
    ~NEDepthToSpaceLayerKernel() = default;
    /** Initialise the kernel's inputs and output.
     *
     * @param[in]  input       Tensor input. Supported tensor rank: 4. Data types supported: U8/S8/QASYMM8/U16/S16/F16/U32/S32/F32.
     * @param[out] output      Tensor output. Data types supported: same as @p input
     * @param[in]  block_shape Block shape x value.
     */
    void configure(const ITensor *input, ITensor *output, int32_t block_shape);
    /** Static function to check if given info will lead to a valid configuration of @ref NEDepthToSpaceLayerKernel.
     *
     * @param[in] input       Tensor input info. Supported tensor rank: 4. Data types supported: U8/S8/QASYMM8/U16/S16/F16/U32/S32/F32.
     * @param[in] output      Tensor output info. Data types supported: same as @p input
     * @param[in] block_shape Block shape value.
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *input, const ITensorInfo *output, int32_t block_shape);

    // Inherited methods overridden:
    void run(const Window &window, const ThreadInfo &info) override;

private:
    const ITensor *_input;       /**< Source tensor */
    ITensor       *_output;      /**< Destination tensor */
    int32_t        _block_shape; /**< Block shape */
    DataLayout     _data_layout; /**< Data layout of the operation */
};
} // namespace arm_compute
#endif /* __ARM_COMPUTE_NEDEPTHTOSPACELAYERKERNEL_H__ */
