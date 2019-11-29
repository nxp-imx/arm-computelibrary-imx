/*
 * Copyright (c) 2017-2019 ARM Limited.
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
#ifndef ARM_COMPUTE_NEDEPTHWISECONVOLUTION_H
#define ARM_COMPUTE_NEDEPTHWISECONVOLUTION_H

#include "arm_compute/core/NEON/kernels/NEDepthwiseConvolutionLayer3x3Kernel.h"
#include "arm_compute/core/NEON/kernels/NEDepthwiseConvolutionLayerNativeKernel.h"
#include "arm_compute/core/NEON/kernels/NEDirectConvolutionLayerOutputStageKernel.h"
#include "arm_compute/core/NEON/kernels/NEFillBorderKernel.h"
#include "arm_compute/runtime/NEON/functions/NEActivationLayer.h"
#include "arm_compute/runtime/NEON/functions/NEPermute.h"
#include "arm_compute/runtime/NEON/functions/assembly/NEDepthwiseConvolutionAssemblyDispatch.h"

namespace arm_compute
{
// Forward declarations
class ITensor;

/** Function to execute a depthwise convolution.
 */
class NEDepthwiseConvolutionLayer : public IFunction
{
public:
    /** Default constructor */
    NEDepthwiseConvolutionLayer(std::shared_ptr<IMemoryManager> memory_manager = nullptr);
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    NEDepthwiseConvolutionLayer(const NEDepthwiseConvolutionLayer &) = delete;
    /** Default move constructor */
    NEDepthwiseConvolutionLayer(NEDepthwiseConvolutionLayer &&) = default;
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    NEDepthwiseConvolutionLayer &operator=(const NEDepthwiseConvolutionLayer &) = delete;
    /** Default move assignment operator */
    NEDepthwiseConvolutionLayer &operator=(NEDepthwiseConvolutionLayer &&) = default;
    /** Initialize the function's source, destination, weights and convolution information.
     *
     * @param[in, out] input            Source tensor. Data type supported: QASYMM8/F16/F32
     * @param[out]     output           Destination tensor. Data type supported: same as @p input.
     * @param[in]      weights          Weights tensor. These are 3D tensors with shape [kernel_x, kernel_y, IFM].
     *                                  Data type supported: Same as @p input or QASYMM8/QSYMM8_PER_CHANNEL when @p input is QASYMM8.
     * @param[in]      biases           Biases tensor. A 1D tensor with shape [IFM]. Must be nullptr if not needed.
     *                                  Data type supported: Same as @p input, S32 when input is QASYMM8.
     * @param[in]      conv_info        Padding and stride information to use for the convolution.
     * @param[in]      depth_multiplier (Optional) Multiplier to apply to the input's depth in order to retrieve the output's depth. Defaults to 1.
     * @param[in]      act_info         (Optional) Activation layer information in case of a fused activation.
     * @param[in]      dilation         (Optional) Dilation, in elements, across x and y. Defaults to (1, 1).
     */
    void configure(ITensor *input, const ITensor *weights, const ITensor *biases, ITensor *output, const PadStrideInfo &conv_info,
                   unsigned int depth_multiplier = 1, const ActivationLayerInfo &act_info = ActivationLayerInfo(), const Size2D &dilation = Size2D(1U, 1U));

    /** Static function to check if given info will lead to a valid configuration of @ref NEDepthwiseConvolutionLayer
     *
     * @param[in] input            Source tensor. Data type supported: QASYMM8/F16/F32
     * @param[in] output           Destination tensor. Data type supported: same as @p input.
     * @param[in] weights          Weights tensor. These are 3D tensors with shape [kernel_x, kernel_y, IFM].
     *                             Data type supported: Same as @p input or QASYMM8/QSYMM8_PER_CHANNEL when @p input is QASYMM8.
     * @param[in] biases           Biases tensor. A 1D tensor with shape [IFM]. Must be nullptr if not needed.
     *                             Data type supported: Same as @p input, S32 when input is QASYMM8.
     * @param[in] conv_info        Padding and stride information to use for the convolution.
     * @param[in] depth_multiplier (Optional) Multiplier to apply to the input's depth in order to retrieve the output's depth. Defaults to 1.
     * @param[in] act_info         (Optional) Activation layer information in case of a fused activation.
     * @param[in] dilation         (Optional) Dilation, in elements, across x and y. Defaults to (1, 1).
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *input, const ITensorInfo *weights, const ITensorInfo *biases, const ITensorInfo *output, const PadStrideInfo &conv_info,
                           unsigned int depth_multiplier = 1, const ActivationLayerInfo &act_info = ActivationLayerInfo(), const Size2D &dilation = Size2D(1U, 1U));

    // Inherited methods overriden:
    void run() override;
    void prepare() override;

private:
    /** Static function to choose the best depthwise convolution function for @ref NEDepthwiseConvolutionLayer
     *
     * @param[in] input            Source tensor info. Data type supported: QASYMM8/F16/F32
     * @param[in] weights          Weights tensor info. These are 3D tensors with shape [kernel_x, kernel_y, IFM].
     *                             Data type supported: Same as @p input or QASYMM8/QSYMM8_PER_CHANNEL when @p input is QASYMM8.
     * @param[in] biases           Biases tensor info. A 1D tensor with shape [IFM]. Must be nullptr if not needed.
     *                             Data type supported: Same as @p input, S32 when input is QASYMM8.
     * @param[in] output           Destination tensor. Data type supported: same as @p input.
     * @param[in] conv_info        Padding and stride information to use for the convolution.
     * @param[in] depth_multiplier (Optional) Multiplier to apply to the input's depth in order to retrieve the output's depth. Defaults to 1.
     * @param[in] act_info         (Optional) Activation layer information in case of a fused activation. Only RELU, BOUNDED_RELU and LU_BOUNDED_RELU for 3x3 QASYMM8 supported.
     * @param[in] dilation         (Optional) Dilation, in elements, across x and y. Defaults to (1, 1).
     *
     * @return a Depthwise Convolution Function
     */
    static DepthwiseConvolutionFunction get_depthwiseconvolution_function(const ITensorInfo *input, const ITensorInfo *weights, const ITensorInfo *biases, const ITensorInfo *output,
                                                                          const PadStrideInfo &conv_info, unsigned int depth_multiplier = 1,
                                                                          ActivationLayerInfo act_info = ActivationLayerInfo(), const Size2D &dilation = Size2D(1U, 1U));

    /** Basic function to execute optimized depthwise convolution routines. This function calls the following NEON kernels:
    *
    * @note At the moment 3x3 and 5x5 convolution of stride 1, 2 are supported
    *
    * -# @ref NEFillBorderKernel (if pad_x or pad_y > 0) and no assembly kernel implementation is present
    * -# @ref NEDepthwiseConvolutionLayer3x3Kernel if 3x3 and no assembly kernel implementation is present
    * -# @ref NEDepthwiseConvolutionAssemblyDispatch if assembly kernel implementation is present
    * -# @ref NEDirectConvolutionLayerOutputStageKernel if re-quantization of output is required
    * -# @ref NEActivationLayer if fused activation is required
    *
    */
    class NEDepthwiseConvolutionLayerOptimizedInternal : public IFunction
    {
    public:
        /** Default constructor */
        NEDepthwiseConvolutionLayerOptimizedInternal(std::shared_ptr<IMemoryManager> memory_manager = nullptr);
        /** Prevent instances of this class from being copied (As this class contains pointers) */
        NEDepthwiseConvolutionLayerOptimizedInternal(const NEDepthwiseConvolutionLayerOptimizedInternal &) = delete;
        /** Default move constructor */
        NEDepthwiseConvolutionLayerOptimizedInternal(NEDepthwiseConvolutionLayerOptimizedInternal &&) = default;
        /** Prevent instances of this class from being copied (As this class contains pointers) */
        NEDepthwiseConvolutionLayerOptimizedInternal &operator=(const NEDepthwiseConvolutionLayerOptimizedInternal &) = delete;
        /** Default move assignment operator */
        NEDepthwiseConvolutionLayerOptimizedInternal &operator=(NEDepthwiseConvolutionLayerOptimizedInternal &&) = default;
        /** Initialize the function's source, destination, kernels and border_size.
         *
         * @param[in, out] input            Source tensor. Data type supported: QASYMM8/F16/F32. (Written to only for border filling).
         * @param[in]      weights          Weights tensor. These are 3D tensors with shape [kernel_x, kernel_y, IFM]. Data type supported: Same as @p input.
         * @param[in]      biases           Biases tensor. A 1D tensor with shape [IFM]. Must be nullptr if not needed.
         *                                  Data type supported: Same as @p input, S32 when input is QASYMM8.
         * @param[out]     output           Destination tensor. Data type supported: same as @p input.
         * @param[in]      conv_info        Padding and stride information to use for the convolution.
         * @param[in]      depth_multiplier (Optional) Multiplier to apply to the input's depth in order to retrieve the output's depth. Defaults to 1.
         * @param[in]      act_info         (Optional) Activation layer information in case of a fused activation.
         * @param[in]      dilation         (Optional) Dilation, in elements, across x and y. Defaults to (1, 1).
         */
        void configure(ITensor *input, const ITensor *weights, const ITensor *biases, ITensor *output, const PadStrideInfo &conv_info,
                       unsigned int depth_multiplier = 1, const ActivationLayerInfo &act_info = ActivationLayerInfo(), const Size2D &dilation = Size2D(1U, 1U));

        /** Static function to check if given info will lead to a valid configuration of @ref NEDepthwiseConvolutionLayer3x3
         *
         * @param[in] input            Source tensor. Data type supported: QASYMM8/F16/F32. (Written to only for border filling).
         * @param[in] weights          Weights tensor. These are 3D tensors with shape [kernel_x, kernel_y, IFM]. Data type supported: Same as @p input.
         * @param[in] biases           Biases tensor. A 1D tensor with shape [IFM]. Must be nullptr if not needed.
         *                             Data type supported: Same as @p input, S32 when input is QASYMM8.
         * @param[in] output           Destination tensor. Data type supported: same as @p input.
         * @param[in] conv_info        Padding and stride information to use for the convolution.
         * @param[in] depth_multiplier (Optional) Multiplier to apply to the input's depth in order to retrieve the output's depth. Defaults to 1.
         * @param[in] act_info         (Optional) Activation layer information in case of a fused activation.
         * @param[in] dilation         (Optional) Dilation, in elements, across x and y. Defaults to (1, 1).
         *
         * @return a status
         */
        static Status validate(const ITensorInfo *input, const ITensorInfo *weights, const ITensorInfo *biases, const ITensorInfo *output, const PadStrideInfo &conv_info,
                               unsigned int depth_multiplier = 1, const ActivationLayerInfo &act_info = ActivationLayerInfo(), const Size2D &dilation = Size2D(1U, 1U));

        // Inherited methods overriden:
        void run() override;
        void prepare() override;

    private:
        /** Configure the kernels/functions for the generic pipeline.
         *
         * @param[in, out] input            Source tensor. Data type supported: QASYMM8/F16/F32. (Written to only for border filling).
         * @param[in]      weights          Weights tensor. These are 3D tensors with shape [kernel_x, kernel_y, IFM]. Data type supported: Same as @p input.
         * @param[in]      biases           Biases tensor. A 1D tensor with shape [IFM]. Must be nullptr if not needed.
         *                                  Data type supported: Same as @p input, S32 when input is QASYMM8.
         * @param[out]     output           Destination tensor. Data type supported: same as @p input.
         * @param[in]      conv_info        Padding and stride information to use for the convolution.
         * @param[in]      depth_multiplier Multiplier to apply to the input's depth in order to retrieve the output's depth. Defaults to 1.
         * @param[in]      act_info         Activation layer information in case of a fused activation.
         * @param[in]      dilation         (Optional) Dilation, in elements, across x and y. Defaults to (1, 1).
         *
         */
        void configure_generic(ITensor *input, const ITensor *weights, const ITensor *biases, ITensor *output, const PadStrideInfo &conv_info,
                               unsigned int depth_multiplier, const ActivationLayerInfo &act_info, const Size2D &dilation = Size2D(1U, 1U));
        /** Configure the kernels/functions for the optimized pipeline.
         *
         * @param[in]  input            Source tensor. Data type supported: QASYMM8/F16/F32. (Written to only for border filling).
         * @param[in]  weights          Weights tensor. These are 3D tensors with shape [kernel_x, kernel_y, IFM]. Data type supported: Same as @p input.
         * @param[in]  biases           Biases tensor. A 1D tensor with shape [IFM]. Must be nullptr if not needed.
         *                              Data type supported: Same as @p input, S32 when input is QASYMM8.
         * @param[out] output           Destination tensor. Data type supported: same as @p input.
         * @param[in]  conv_info        Padding and stride information to use for the convolution.
         * @param[in]  depth_multiplier Multiplier to apply to the input's depth in order to retrieve the output's depth. Defaults to 1.
         * @param[in]  act_info         Activation layer information in case of a fused activation.
         */
        void configure_optimized(const ITensor *input, const ITensor *weights, const ITensor *biases, ITensor *output, const PadStrideInfo &conv_info,
                                 unsigned int depth_multiplier, const ActivationLayerInfo &act_info, const Size2D &dilation = Size2D(1U, 1U));
        /** Run generic kernel */
        void run_generic();
        /** Run optimized function */
        void run_optimized();

        MemoryGroup                               _memory_group;
        NEDepthwiseConvolutionLayer3x3Kernel      _dwc_kernel;
        NEDepthwiseConvolutionAssemblyDispatch    _dwc_optimized_func;
        NEDirectConvolutionLayerOutputStageKernel _output_stage_kernel;
        NEFillBorderKernel                        _border_handler;
        NEPermute                                 _permute_input;
        NEPermute                                 _permute_weights;
        NEPermute                                 _permute_output;
        NEActivationLayer                         _activationlayer_function;
        Tensor                                    _accumulator;
        Tensor                                    _permuted_input;
        Tensor                                    _permuted_weights;
        Tensor                                    _permuted_output;
        const ITensor                            *_original_weights;
        bool                                      _has_bias;
        bool                                      _is_quantized;
        bool                                      _is_optimized;
        bool                                      _is_nchw;
        bool                                      _permute;
        bool                                      _is_activationlayer_enabled;
        bool                                      _is_prepared;
    };

    /** Basic function to execute a generic depthwise convolution. This function calls the following NEON kernel:
     *
     * -# @ref NEDepthwiseConvolutionLayerNativeKernel
     *
     */
    class NEDepthwiseConvolutionLayerGeneric : public IFunction
    {
    public:
        /** Default constructor */
        NEDepthwiseConvolutionLayerGeneric();
        /** Prevent instances of this class from being copied (As this class contains pointers) */
        NEDepthwiseConvolutionLayerGeneric(const NEDepthwiseConvolutionLayerGeneric &) = delete;
        /** Default move constructor */
        NEDepthwiseConvolutionLayerGeneric(NEDepthwiseConvolutionLayerGeneric &&) = default;
        /** Prevent instances of this class from being copied (As this class contains pointers) */
        NEDepthwiseConvolutionLayerGeneric &operator=(const NEDepthwiseConvolutionLayerGeneric &) = delete;
        /** Default move assignment operator */
        NEDepthwiseConvolutionLayerGeneric &operator=(NEDepthwiseConvolutionLayerGeneric &&) = default;
        /** Initialize the function's source, destination, weights and convolution information.
         *
         * @param[in, out] input            Source tensor. Data type supported: QASYMM8/F16/F32. (Written to only for border filling).
         * @param[out]     output           Destination tensor. Data type supported: same as @p input.
         * @param[in]      weights          Weights tensor. These are 3D tensors with shape [kernel_x, kernel_y, IFM].
         *                                  Data type supported: Same as @p input or QASYMM8/QSYMM8_PER_CHANNEL when @p input is QASYMM8.
         * @param[in]      biases           Biases tensor. A 1D tensor with shape [IFM]. Must be nullptr if not needed.
         *                                  Data type supported: Same as @p input, S32 when input is QASYMM8.
         * @param[in]      conv_info        Padding and stride information to use for the convolution.
         * @param[in]      depth_multiplier (Optional) Multiplier to apply to the input's depth in order to retrieve the output's depth. Defaults to 1.
         * @param[in]      act_info         (Optional) Activation layer information in case of a fused activation.
         * @param[in]      dilation         (Optional) Dilation, in elements, across x and y. Defaults to (1, 1).
         */
        void configure(ITensor *input, const ITensor *weights, const ITensor *biases, ITensor *output, const PadStrideInfo &conv_info,
                       unsigned int depth_multiplier = 1, const ActivationLayerInfo &act_info = ActivationLayerInfo(), const Size2D &dilation = Size2D(1U, 1U));

        /** Static function to check if given info will lead to a valid configuration of @ref NEDepthwiseConvolutionLayerGeneric
         *
         * @param[in] input            Source tensor. Data type supported: QASYMM8/F16/F32. (Written to only for border filling).
         * @param[in] output           Destination tensor. Data type supported: same as @p input.
         * @param[in] weights          Weights tensor. These are 3D tensors with shape [kernel_x, kernel_y, IFM].
         *                             Data type supported: Same as @p input or QASYMM8/QSYMM8_PER_CHANNEL when @p input is QASYMM8.
         * @param[in] biases           Biases tensor. A 1D tensor with shape [IFM]. Must be nullptr if not needed.
         *                             Data type supported: Same as @p input, S32 when input is QASYMM8.
         * @param[in] conv_info        Padding and stride information to use for the convolution.
         * @param[in] depth_multiplier (Optional) Multiplier to apply to the input's depth in order to retrieve the output's depth. Defaults to 1.
         * @param[in] act_info         (Optional) Activation layer information in case of a fused activation.
         * @param[in] dilation         (Optional) Dilation, in elements, across x and y. Defaults to (1, 1).
         *
         * @return a status
         */
        static Status validate(const ITensorInfo *input, const ITensorInfo *weights, const ITensorInfo *biases, const ITensorInfo *output, const PadStrideInfo &conv_info,
                               unsigned int depth_multiplier = 1, const ActivationLayerInfo &act_info = ActivationLayerInfo(), const Size2D &dilation = Size2D(1U, 1U));

        // Inherited methods overriden:
        void run() override;
        void prepare() override;

    private:
        NEDepthwiseConvolutionLayerNativeKernel _depthwise_conv_kernel;
        NEFillBorderKernel                      _fill_border;
        NEPermute                               _permute_input;
        NEPermute                               _permute_weights;
        NEPermute                               _permute_output;
        NEActivationLayer                       _activationlayer_function;
        Tensor                                  _permuted_input;
        Tensor                                  _permuted_weights;
        Tensor                                  _permuted_output;
        bool                                    _is_prepared;
        bool                                    _is_nchw;
        bool                                    _is_activationlayer_enabled;
        const ITensor                          *_original_weights;
    };

    DepthwiseConvolutionFunction                 _depth_conv_func;
    NEDepthwiseConvolutionLayerOptimizedInternal _func_optimized;
    NEDepthwiseConvolutionLayerGeneric           _func_generic;
};

/** Basic function to execute optimized depthwise convolution routines. This function calls the following NEON kernels:
 *
 * @note At the moment 3x3 and 5x5 convolution of stride 1, 2 are supported
 *
 * -# @ref NEFillBorderKernel (if pad_x or pad_y > 0) and no assembly kernel implementation is present
 * -# @ref NEDepthwiseConvolutionLayer3x3Kernel if 3x3 and no assembly kernel implementation is present
 * -# @ref NEDepthwiseConvolutionAssemblyDispatch if assembly kernel implementation is present
 * -# @ref NEDirectConvolutionLayerOutputStageKernel if re-quantization of output is required
 * -# @ref NEActivationLayer if fused activation is required
 *
 */
class NEDepthwiseConvolutionLayerOptimized : public IFunction
{
public:
    /** Default constructor */
    NEDepthwiseConvolutionLayerOptimized(std::shared_ptr<IMemoryManager> memory_manager = nullptr);
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    NEDepthwiseConvolutionLayerOptimized(const NEDepthwiseConvolutionLayerOptimized &) = delete;
    /** Default move constructor */
    NEDepthwiseConvolutionLayerOptimized(NEDepthwiseConvolutionLayerOptimized &&) = default;
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    NEDepthwiseConvolutionLayerOptimized &operator=(const NEDepthwiseConvolutionLayerOptimized &) = delete;
    /** Default move assignment operator */
    NEDepthwiseConvolutionLayerOptimized &operator=(NEDepthwiseConvolutionLayerOptimized &&) = default;
    /** Initialize the function's source, destination, kernels and border_size.
     *
     * @param[in, out] input            Source tensor. Data type supported: QASYMM8/F16/F32. (Written to only for border filling).
     * @param[in]      weights          Weights tensor. These are 3D tensors with shape [W, H, IFM]. Data type supported: Same as @p input.
     * @param[in]      biases           Biases tensor. A 1D tensor with shape [IFM]. Must be nullptr if not needed.
     *                                  Data type supported: Same as @p input, S32 when input is QASYMM8.
     * @param[out]     output           Destination tensor. Data type supported: same as @p input.
     * @param[in]      conv_info        Padding and stride information to use for the convolution.
     * @param[in]      depth_multiplier (Optional) Multiplier to apply to the input's depth in order to retrieve the output's depth. Defaults to 1.
     * @param[in]      act_info         (Optional) Activation layer information in case of a fused activation.
     * @param[in]      dilation         (Optional) Dilation, in elements, across x and y. Defaults to (1, 1).
     */
    ARM_COMPUTE_DEPRECATED_REL_REPLACE(20.02, NEDepthwiseConvolutionLayer)
    void configure(ITensor *input, const ITensor *weights, const ITensor *biases, ITensor *output, const PadStrideInfo &conv_info,
                   unsigned int depth_multiplier = 1, const ActivationLayerInfo &act_info = ActivationLayerInfo(), const Size2D &dilation = Size2D(1U, 1U));

    /** Static function to check if given info will lead to a valid configuration of @ref NEDepthwiseConvolutionLayerOptimized
     *
     * @param[in] input            Source tensor. Data type supported: QASYMM8/F16/F32. (Written to only for border filling).
     * @param[in] weights          Weights tensor. These are 3D tensors with shape [W, H, IFM]. Data type supported: Same as @p input.
     * @param[in] biases           Biases tensor. A 1D tensor with shape [IFM]. Must be nullptr if not needed.
     *                             Data type supported: Same as @p input, S32 when input is QASYMM8.
     * @param[in] output           Destination tensor. Data type supported: same as @p input.
     * @param[in] conv_info        Padding and stride information to use for the convolution.
     * @param[in] depth_multiplier (Optional) Multiplier to apply to the input's depth in order to retrieve the output's depth. Defaults to 1.
     * @param[in] act_info         (Optional) Activation layer information in case of a fused activation.
     * @param[in] dilation         (Optional) Dilation, in elements, across x and y. Defaults to (1, 1).
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *input, const ITensorInfo *weights, const ITensorInfo *biases, const ITensorInfo *output, const PadStrideInfo &conv_info,
                           unsigned int depth_multiplier = 1, const ActivationLayerInfo &act_info = ActivationLayerInfo(), const Size2D &dilation = Size2D(1U, 1U));

    // Inherited methods overriden:
    void run() override;
    void prepare() override;

private:
    NEDepthwiseConvolutionLayer _func;
};
} // namespace arm_compute
#endif /* ARM_COMPUTE_NEDEPTHWISECONVOLUTION_H */