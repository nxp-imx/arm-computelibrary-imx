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
#include "src/gpu/cl/kernels/ClDirectConv3dKernel.h"

#include "arm_compute/core/CL/ICLTensor.h"
#include "arm_compute/core/utils/misc/ShapeCalculator.h"
#include "src/core/CL/CLValidate.h"
#include "src/core/helpers/WindowHelpers.h"
#include "support/Cast.h"

namespace arm_compute
{
namespace opencl
{
namespace kernels
{
namespace
{
Status validate_arguments(const ITensorInfo *src, const ITensorInfo *weights, const ITensorInfo *biases, const ITensorInfo *dst, const Conv3dInfo &conv3d_info)
{
    ARM_COMPUTE_ERROR_ON_MISMATCHING_DATA_LAYOUT(src, weights, dst);
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(src->data_layout() != DataLayout::NDHWC, "Only NDHWC layout supported");
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(conv3d_info.act_info.enabled(), "Fused activation not supported");

    ARM_COMPUTE_RETURN_ERROR_ON_F16_UNSUPPORTED(src);
    ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(src, 1, DataType::F16, DataType::F32);
    ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(src, weights);

    ARM_COMPUTE_RETURN_ERROR_ON_MSG(weights->dimension(1) != src->dimension(0), "Weights feature map dimension should match the respective src's one");
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(weights->num_dimensions() > 5, "Weights can be at most 5 dimensional");

    ARM_COMPUTE_RETURN_ERROR_ON(weights->dimension(2) > (src->dimension(1) + conv3d_info.padding.left + conv3d_info.padding.right));
    ARM_COMPUTE_RETURN_ERROR_ON(weights->dimension(3) > (src->dimension(2) + conv3d_info.padding.top + conv3d_info.padding.bottom));
    ARM_COMPUTE_RETURN_ERROR_ON(weights->dimension(4) > (src->dimension(3) + conv3d_info.padding.front + conv3d_info.padding.back));

    if(biases != nullptr)
    {
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(weights, biases);
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(biases->dimension(0) != weights->dimension(0), "Biases size and number of dst feature maps should match");
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(biases->num_dimensions() > 1, "Biases should be one dimensional");
    }

    // Checks performed when dst is configured
    if(dst->total_size() != 0)
    {
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(dst->dimension(0) != weights->dimension(0), "Weights and dst OFMs should match");
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DIMENSIONS(dst->tensor_shape(), misc::shape_calculator::compute_conv3d_shape(src->tensor_shape(), weights->tensor_shape(), conv3d_info));
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(src, dst);
    }

    return Status{};
}
} // namespace

ClDirectConv3dKernel::ClDirectConv3dKernel()
{
    _type = CLKernelType::DIRECT;
}

void ClDirectConv3dKernel::configure(const CLCompileContext &compile_context, const ITensorInfo *src, const ITensorInfo *weights, const ITensorInfo *biases, ITensorInfo *dst,
                                     const Conv3dInfo &conv3d_info)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(src, weights, dst);

    // Perform validation
    ARM_COMPUTE_ERROR_THROW_ON(validate_arguments(src, weights, biases, dst, conv3d_info));

    // Create window and update padding
    const DataType data_type      = src->data_type();
    const size_t   src_width      = src->dimension(1);
    const size_t   src_height     = src->dimension(2);
    const size_t   src_depth      = src->dimension(3);
    const size_t   src_channels   = src->dimension(0);
    const size_t   dst_width      = dst->dimension(1);
    const size_t   dst_height     = dst->dimension(2);
    const size_t   dst_depth      = dst->dimension(3);
    const size_t   dst_channels   = dst->dimension(0);
    const size_t   weights_width  = weights->dimension(2);
    const size_t   weights_height = weights->dimension(3);
    const size_t   weights_depth  = weights->dimension(4);
    const size_t   pad_left       = conv3d_info.padding.left;
    const size_t   pad_top        = conv3d_info.padding.top;
    const size_t   pad_front      = conv3d_info.padding.front;
    const size_t   conv_stride_x  = conv3d_info.stride.x();
    const size_t   conv_stride_y  = conv3d_info.stride.y();
    const size_t   conv_stride_z  = conv3d_info.stride.z();

    const size_t n0               = std::min(dst->dimension(0), static_cast<size_t>(4u));
    const size_t m0               = (dst->tensor_shape()[0] > 16) ? ((data_type == DataType::F32) ? 2U : 4U) : 1U;
    const size_t k0               = adjust_vec_size(8u, src->dimension(0));
    const size_t partial_store_n0 = dst->dimension(0) % n0;

    CLBuildOptions build_options;
    build_options.add_option("-cl-fast-relaxed-math");
    build_options.add_option("-DDATA_TYPE=" + get_cl_type_from_data_type(data_type));
    build_options.add_option("-DACC_DATA_TYPE=float");
    build_options.add_option("-DSRC_WIDTH=" + support::cpp11::to_string(src_width));
    build_options.add_option("-DSRC_HEIGHT=" + support::cpp11::to_string(src_height));
    build_options.add_option("-DSRC_DEPTH=" + support::cpp11::to_string(src_depth));
    build_options.add_option("-DSRC_CHANNELS=" + support::cpp11::to_string(src_channels));
    build_options.add_option("-DDST_WIDTH=" + support::cpp11::to_string(dst_width));
    build_options.add_option("-DDST_HEIGHT=" + support::cpp11::to_string(dst_height));
    build_options.add_option("-DDST_DEPTH=" + support::cpp11::to_string(dst_depth));
    build_options.add_option("-DDST_CHANNELS=" + support::cpp11::to_string(dst_channels));
    build_options.add_option("-DWEI_WIDTH=" + support::cpp11::to_string(weights_width));
    build_options.add_option("-DWEI_HEIGHT=" + support::cpp11::to_string(weights_height));
    build_options.add_option("-DWEI_DEPTH=" + support::cpp11::to_string(weights_depth));
    build_options.add_option("-DSTRIDE_X=" + support::cpp11::to_string(conv_stride_x));
    build_options.add_option("-DSTRIDE_Y=" + support::cpp11::to_string(conv_stride_y));
    build_options.add_option("-DSTRIDE_Z=" + support::cpp11::to_string(conv_stride_z));
    build_options.add_option("-DPAD_LEFT=" + support::cpp11::to_string(pad_left));
    build_options.add_option("-DPAD_TOP=" + support::cpp11::to_string(pad_top));
    build_options.add_option("-DPAD_FRONT=" + support::cpp11::to_string(pad_front));
    build_options.add_option("-DN0=" + support::cpp11::to_string(n0));
    build_options.add_option("-DM0=" + support::cpp11::to_string(m0));
    build_options.add_option("-DK0=" + support::cpp11::to_string(k0));
    build_options.add_option("-DPARTIAL_N0=" + support::cpp11::to_string(partial_store_n0));
    build_options.add_option_if(biases != nullptr, std::string("-DHAS_BIAS"));

    std::string kernel_name = "direct_convolution3d_ndhwc";
    _kernel                 = create_kernel(compile_context, kernel_name, build_options.options());

    // Configure kernel window
    Window win = calculate_max_window(*dst, Steps(n0, m0));
    ICLKernel::configure_internal(win);

    // Set config_id for enabling LWS tuning
    _config_id = kernel_name;
    _config_id += "_";
    _config_id += lower_string(string_from_data_type(data_type));
    _config_id += "_";
    _config_id += support::cpp11::to_string(weights_width);
    _config_id += "_";
    _config_id += support::cpp11::to_string(weights_height);
    _config_id += "_";
    _config_id += support::cpp11::to_string(weights_depth);
    _config_id += "_";
    _config_id += support::cpp11::to_string(conv_stride_x);
    _config_id += "_";
    _config_id += support::cpp11::to_string(conv_stride_y);
    _config_id += "_";
    _config_id += support::cpp11::to_string(conv_stride_z);
    _config_id += "_";
    _config_id += support::cpp11::to_string(dst_width);
    _config_id += "_";
    _config_id += support::cpp11::to_string(dst_height);
    _config_id += "_";
    _config_id += support::cpp11::to_string(dst_channels);
}

Status ClDirectConv3dKernel::validate(const ITensorInfo *src, const ITensorInfo *weights, const ITensorInfo *biases, const ITensorInfo *dst, const Conv3dInfo &conv3d_info)
{
    ARM_COMPUTE_RETURN_ON_ERROR(validate_arguments(src, weights, biases, dst, conv3d_info));
    return Status{};
}

void ClDirectConv3dKernel::run_op(ITensorPack &tensors, const Window &window, cl::CommandQueue &queue)
{
    ARM_COMPUTE_ERROR_ON_UNCONFIGURED_KERNEL(this);
    ARM_COMPUTE_ERROR_ON_INVALID_SUBWINDOW(IKernel::window(), window);

    const auto src     = utils::cast::polymorphic_downcast<const ICLTensor *>(tensors.get_const_tensor(TensorType::ACL_SRC_0));
    const auto weights = utils::cast::polymorphic_downcast<const ICLTensor *>(tensors.get_const_tensor(TensorType::ACL_SRC_1));
    const auto biases  = utils::cast::polymorphic_downcast<const ICLTensor *>(tensors.get_const_tensor(TensorType::ACL_SRC_2));
    auto       dst     = utils::cast::polymorphic_downcast<ICLTensor *>(tensors.get_tensor(TensorType::ACL_DST));

    // Get initial windows
    Window slice = window.first_slice_window_3D();
    slice.set(Window::DimY, Window::Dimension(0, ceil_to_multiple(dst->info()->dimension(1) * dst->info()->dimension(2) * dst->info()->dimension(3), slice.y().step()), slice.y().step()));
    slice.set(Window::DimZ, Window::Dimension(0, dst->info()->dimension(4), 1));

    unsigned int idx = 0;
    add_4D_tensor_argument(idx, src, slice);
    add_4D_tensor_argument(idx, dst, slice);
    add_4D_tensor_argument(idx, weights, slice);
    if(biases != nullptr)
    {
        add_1D_tensor_argument(idx, biases, slice);
    }
    enqueue(queue, *this, slice, lws_hint());
}
} // namespace kernels
} // namespace opencl
} // namespace arm_compute
