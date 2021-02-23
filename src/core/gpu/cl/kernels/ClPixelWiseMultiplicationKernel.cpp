/*
 * Copyright (c) 2016-2021 Arm Limited.
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
#include "src/core/gpu/cl/kernels/ClPixelWiseMultiplicationKernel.h"

#include "arm_compute/core/CL/CLHelpers.h"
#include "arm_compute/core/CL/CLKernelLibrary.h"
#include "arm_compute/core/CL/ICLTensor.h"
#include "arm_compute/core/CL/OpenCL.h"
#include "arm_compute/core/TensorInfo.h"
#include "src/core/CL/CLValidate.h"
#include "src/core/helpers/AutoConfiguration.h"
#include "src/core/helpers/WindowHelpers.h"
#include "support/Cast.h"
#include "support/StringSupport.h"

namespace arm_compute
{
namespace opencl
{
namespace kernels
{
namespace
{
constexpr unsigned int num_elems_processed_per_iteration = 16;

Status validate_arguments(const ITensorInfo *src1, const ITensorInfo *src2, const ITensorInfo *dst, float scale,
                          ConvertPolicy overflow_policy, RoundingPolicy rounding_policy, const ActivationLayerInfo &act_info)
{
    ARM_COMPUTE_UNUSED(overflow_policy);
    ARM_COMPUTE_UNUSED(rounding_policy);

    ARM_COMPUTE_RETURN_ERROR_ON_NULLPTR(src1, src2, dst);
    ARM_COMPUTE_RETURN_ERROR_ON_F16_UNSUPPORTED(src1);
    ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(src1,
                                                         1,
                                                         DataType::U8, DataType::QASYMM8, DataType::QASYMM8_SIGNED,
                                                         DataType::S16, DataType::QSYMM16, DataType::F16,
                                                         DataType::F32);
    ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(src2,
                                                         1,
                                                         DataType::U8, DataType::QASYMM8, DataType::QASYMM8_SIGNED,
                                                         DataType::S16, DataType::QSYMM16, DataType::F16,
                                                         DataType::F32);
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(scale < 0, "Scale cannot be negative.");
    ARM_COMPUTE_RETURN_ERROR_ON(act_info.enabled() && !is_data_type_float(dst->data_type()));

    const TensorShape &out_shape = TensorShape::broadcast_shape(src1->tensor_shape(), src2->tensor_shape());

    ARM_COMPUTE_RETURN_ERROR_ON_MSG(out_shape.total_size() == 0, "Inputs are not broadcast compatible");

    // Validate in case of configured dst
    if(dst->total_size() > 0)
    {
        ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(dst,
                                                             1,
                                                             DataType::U8, DataType::QASYMM8, DataType::QASYMM8_SIGNED,
                                                             DataType::S16, DataType::QSYMM16, DataType::F16,
                                                             DataType::S32, DataType::F32);
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(dst->data_type() == DataType::U8 && (src1->data_type() != DataType::U8 || src2->data_type() != DataType::U8),
                                        "Dst can only be U8 if both src are U8");
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(dst->data_type() == DataType::QASYMM8 && (src1->data_type() != DataType::QASYMM8 || src2->data_type() != DataType::QASYMM8),
                                        "Dst can only be QASYMM8 if both src are QASYMM8");
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(dst->data_type() == DataType::QASYMM8_SIGNED && (src1->data_type() != DataType::QASYMM8_SIGNED || src2->data_type() != DataType::QASYMM8_SIGNED),
                                        "Dst can only be QASYMM8_SIGNED if both src are QASYMM8_SIGNED");
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(dst->data_type() == DataType::QSYMM16 && (src1->data_type() != DataType::QSYMM16 || src2->data_type() != DataType::QSYMM16),
                                        "Dst can only be QSYMM16 if both src are QSYMM16");
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(dst->data_type() == DataType::S32 && (src1->data_type() != DataType::QSYMM16 || src2->data_type() != DataType::QSYMM16),
                                        "Dst can only be S32 if both src are QSYMM16");
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(detail::have_different_dimensions(out_shape, dst->tensor_shape(), 0), "Wrong shape for dst");
    }

    return Status{};
}

std::pair<Status, Window> validate_and_configure_window(ITensorInfo *src1, ITensorInfo *src2, ITensorInfo *dst)
{
    const TensorShape &out_shape = TensorShape::broadcast_shape(src1->tensor_shape(), src2->tensor_shape());

    // Auto initialize dst if not initialized
    {
        set_shape_if_empty(*dst, out_shape);

        if(src1->data_type() == DataType::S16 || src2->data_type() == DataType::S16)
        {
            set_format_if_unknown(*dst, Format::S16);
        }
        else if(src1->data_type() == DataType::F32 || src2->data_type() == DataType::F32)
        {
            set_format_if_unknown(*dst, Format::F32);
        }
        else if(src1->data_type() == DataType::QASYMM8)
        {
            set_data_type_if_unknown(*dst, DataType::QASYMM8);
        }
        else if(src1->data_type() == DataType::QASYMM8_SIGNED)
        {
            set_data_type_if_unknown(*dst, DataType::QASYMM8_SIGNED);
        }
        else if(src1->data_type() == DataType::QSYMM16)
        {
            set_data_type_if_unknown(*dst, DataType::QSYMM16);
        }
    }

    Window win        = calculate_max_window(out_shape, Steps(num_elems_processed_per_iteration));
    Window win_input1 = win.broadcast_if_dimension_le_one(*src1);
    Window win_input2 = win.broadcast_if_dimension_le_one(*src2);

    AccessWindowHorizontal input1_access(src1, 0, num_elems_processed_per_iteration);
    AccessWindowHorizontal input2_access(src2, 0, num_elems_processed_per_iteration);
    AccessWindowHorizontal output_access(dst, 0, num_elems_processed_per_iteration);

    bool window_changed = update_window_and_padding(win_input1, input1_access)
                          || update_window_and_padding(win_input2, input2_access)
                          || update_window_and_padding(win, output_access);

    Status err = (window_changed) ? ARM_COMPUTE_CREATE_ERROR(ErrorCode::RUNTIME_ERROR, "Insufficient Padding!") : Status{};
    return std::make_pair(err, win);
}

BorderSize calc_border_size(ITensorInfo *src1, ITensorInfo *src2, ITensorInfo *dst)
{
    const unsigned int replicateSize = dst->dimension(0) - std::min(src1->dimension(0), src2->dimension(0));
    const unsigned int border        = std::min<unsigned int>(num_elems_processed_per_iteration - 1U, replicateSize);

    return BorderSize{ 0, border, 0, 0 };
}
} // namespace

void ClPixelWiseMultiplicationKernel::configure(const CLCompileContext &compile_context, ITensorInfo *src1, ITensorInfo *src2, ITensorInfo *dst, float scale,
                                                ConvertPolicy overflow_policy, RoundingPolicy rounding_policy, const ActivationLayerInfo &act_info)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(src1, src2, dst);
    ARM_COMPUTE_ERROR_THROW_ON(validate_arguments(src1, src2, dst,
                                                  scale, overflow_policy, rounding_policy, act_info));

    // Calculate border size
    _border_size = calc_border_size(src1, src2, dst);

    // Configure kernel window
    auto win_config = validate_and_configure_window(src1, src2, dst);
    ARM_COMPUTE_ERROR_THROW_ON(win_config.first);

    int scale_int = -1;
    // Extract sign, exponent and mantissa
    int   exponent            = 0;
    float normalized_mantissa = std::frexp(scale, &exponent);
    // Use int scaling if factor is equal to 1/2^n for 0 <= n <= 15
    // frexp returns 0.5 as mantissa which means that the exponent will be in the range of -1 <= e <= 14
    // Moreover, it will be negative as we deal with 1/2^n
    if((normalized_mantissa == 0.5f) && (-14 <= exponent) && (exponent <= 1))
    {
        // Store the positive exponent. We know that we compute 1/2^n
        // Additionally we need to subtract 1 to compensate that frexp used a mantissa of 0.5
        scale_int = std::abs(exponent - 1);
    }

    std::string acc_type;
    // Check if it has float src and dst
    if(is_data_type_float(src1->data_type()) || is_data_type_float(src2->data_type()))
    {
        scale_int = -1;
        acc_type  = (src1->data_type() == DataType::F32 || src2->data_type() == DataType::F32) ? "float" : "half";
    }
    else
    {
        if(src1->element_size() == 2 || src2->element_size() == 2)
        {
            // Use 32-bit accumulator for 16-bit input
            acc_type = "int";
        }
        else
        {
            // Use 16-bit accumulator for 8-bit input
            acc_type = "ushort";
        }
    }

    const bool is_quantized = is_data_type_quantized(src1->data_type());

    // Set kernel build options
    std::string    kernel_name = "pixelwise_mul";
    CLBuildOptions build_opts;
    build_opts.add_option("-DDATA_TYPE_IN1=" + get_cl_type_from_data_type(src1->data_type()));
    build_opts.add_option("-DDATA_TYPE_IN2=" + get_cl_type_from_data_type(src2->data_type()));
    build_opts.add_option("-DDATA_TYPE_OUT=" + get_cl_type_from_data_type(dst->data_type()));
    build_opts.add_option("-DVEC_SIZE=" + support::cpp11::to_string(num_elems_processed_per_iteration));
    if(is_quantized && (dst->data_type() != DataType::S32))
    {
        const UniformQuantizationInfo iq1_info = src1->quantization_info().uniform();
        const UniformQuantizationInfo iq2_info = src2->quantization_info().uniform();
        const UniformQuantizationInfo oq_info  = dst->quantization_info().uniform();

        build_opts.add_option_if(is_data_type_quantized_asymmetric(src1->data_type()),
                                 "-DOFFSET_IN1=" + support::cpp11::to_string(iq1_info.offset));
        build_opts.add_option_if(is_data_type_quantized_asymmetric(src2->data_type()),
                                 "-DOFFSET_IN2=" + support::cpp11::to_string(iq2_info.offset));
        build_opts.add_option_if(is_data_type_quantized_asymmetric(dst->data_type()),
                                 "-DOFFSET_OUT=" + support::cpp11::to_string(oq_info.offset));
        build_opts.add_option("-DSCALE_IN1=" + float_to_string_with_full_precision(iq1_info.scale));
        build_opts.add_option("-DSCALE_IN2=" + float_to_string_with_full_precision(iq2_info.scale));
        build_opts.add_option("-DSCALE_OUT=" + float_to_string_with_full_precision(oq_info.scale));
        kernel_name += "_quantized";
    }
    else
    {
        kernel_name += (scale_int >= 0) ? "_int" : "_float";
        build_opts.add_option_if_else(overflow_policy == ConvertPolicy::WRAP || is_data_type_float(dst->data_type()), "-DWRAP", "-DSATURATE");
        build_opts.add_option_if_else(rounding_policy == RoundingPolicy::TO_ZERO, "-DROUND=_rtz", "-DROUND=_rte");
        build_opts.add_option("-DACC_DATA_TYPE=" + acc_type);
        if(act_info.enabled())
        {
            build_opts.add_option("-DACTIVATION_TYPE=" + lower_string(string_from_activation_func(act_info.activation())));
            build_opts.add_option("-DA_VAL=" + float_to_string_with_full_precision(act_info.a()));
            build_opts.add_option("-DB_VAL=" + float_to_string_with_full_precision(act_info.b()));
        }
    }

    // Create kernel
    _kernel = create_kernel(compile_context, kernel_name, build_opts.options());

    // Set scale argument
    unsigned int idx = 3 * num_arguments_per_3D_tensor(); // Skip the src and dst parameters

    if(scale_int >= 0 && !is_quantized)
    {
        _kernel.setArg(idx++, scale_int);
    }
    else
    {
        _kernel.setArg(idx++, scale);
    }

    ICLKernel::configure_internal(win_config.second);
}

Status ClPixelWiseMultiplicationKernel::validate(const ITensorInfo *src1, const ITensorInfo *src2, const ITensorInfo *dst, float scale,
                                                 ConvertPolicy overflow_policy, RoundingPolicy rounding_policy, const ActivationLayerInfo &act_info)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(src1, src2, dst);
    ARM_COMPUTE_RETURN_ON_ERROR(validate_arguments(src1, src2, dst, scale, overflow_policy, rounding_policy, act_info));
    ARM_COMPUTE_RETURN_ON_ERROR(validate_and_configure_window(src1->clone().get(), src2->clone().get(), dst->clone().get()).first);

    return Status{};
}

void ClPixelWiseMultiplicationKernel::run_op(ITensorPack &tensors, const Window &window, cl::CommandQueue &queue)
{
    ARM_COMPUTE_ERROR_ON_UNCONFIGURED_KERNEL(this);
    ARM_COMPUTE_ERROR_ON_INVALID_SUBWINDOW(ICLKernel::window(), window);

    const auto src_0 = utils::cast::polymorphic_downcast<const ICLTensor *>(tensors.get_const_tensor(TensorType::ACL_SRC_0));
    const auto src_1 = utils::cast::polymorphic_downcast<const ICLTensor *>(tensors.get_const_tensor(TensorType::ACL_SRC_1));
    auto       dst   = utils::cast::polymorphic_downcast<ICLTensor *>(tensors.get_tensor(TensorType::ACL_DST));

    const TensorShape &in_shape1 = src_0->info()->tensor_shape();
    const TensorShape &in_shape2 = src_1->info()->tensor_shape();
    const TensorShape &out_shape = dst->info()->tensor_shape();

    bool can_collapse = true;
    if(std::min(in_shape1.total_size(), in_shape2.total_size()) > 1)
    {
        can_collapse = (std::min(in_shape1.num_dimensions(), in_shape2.num_dimensions()) > Window::DimZ);
        for(size_t d = Window::DimZ; can_collapse && (d < out_shape.num_dimensions()); ++d)
        {
            can_collapse = (in_shape1[d] == in_shape2[d]);
        }
    }

    bool   has_collapsed = false;
    Window collapsed     = can_collapse ? window.collapse_if_possible(ICLKernel::window(), Window::DimZ, &has_collapsed) : window;

    const TensorShape &in_shape1_collapsed = has_collapsed ? in_shape1.collapsed_from(Window::DimZ) : in_shape1;
    const TensorShape &in_shape2_collapsed = has_collapsed ? in_shape2.collapsed_from(Window::DimZ) : in_shape2;

    Window slice        = collapsed.first_slice_window_3D();
    Window slice_input1 = slice.broadcast_if_dimension_le_one(in_shape1_collapsed);
    Window slice_input2 = slice.broadcast_if_dimension_le_one(in_shape2_collapsed);

    do
    {
        unsigned int idx = 0;
        add_3D_tensor_argument(idx, src_0, slice_input1);
        add_3D_tensor_argument(idx, src_1, slice_input2);
        add_3D_tensor_argument(idx, dst, slice);
        enqueue(queue, *this, slice, lws_hint());

        ARM_COMPUTE_UNUSED(collapsed.slide_window_slice_3D(slice_input1));
        ARM_COMPUTE_UNUSED(collapsed.slide_window_slice_3D(slice_input2));
    }
    while(collapsed.slide_window_slice_3D(slice));
}

BorderSize ClPixelWiseMultiplicationKernel::border_size() const
{
    return _border_size;
}

namespace
{
constexpr unsigned int num_elems_processed_per_iteration_complex = 1;

Status validate_arguments_complex(const ITensorInfo *src1, const ITensorInfo *src2, const ITensorInfo *dst, const ActivationLayerInfo &act_info)
{
    ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(src1, 2, DataType::F16, DataType::F32);
    ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(src2, 2, DataType::F16, DataType::F32);
    ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(src1, src2);

    const TensorShape &out_shape = TensorShape::broadcast_shape(src1->tensor_shape(), src2->tensor_shape());

    ARM_COMPUTE_RETURN_ERROR_ON_MSG(out_shape.total_size() == 0, "Inputs are not broadcast compatible");
    ARM_COMPUTE_RETURN_ERROR_ON(act_info.enabled() && !is_data_type_float(dst->data_type()));

    // Validate in case of configured dst
    if(dst->total_size() > 0)
    {
        ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(dst, 2, DataType::F16, DataType::F32);
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(src1, dst);
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(detail::have_different_dimensions(out_shape, dst->tensor_shape(), 0), "Wrong shape for dst");
    }

    return Status{};
}

std::pair<Status, Window> validate_and_configure_window_complex(ITensorInfo *src1, ITensorInfo *src2, ITensorInfo *dst)
{
    const TensorShape &out_shape = TensorShape::broadcast_shape(src1->tensor_shape(), src2->tensor_shape());

    // Auto initialize dst if not initialized
    const TensorInfo out_info(out_shape, src1->num_channels(), src1->data_type());
    auto_init_if_empty(*dst, out_info);

    Window win        = calculate_max_window(out_shape, Steps(num_elems_processed_per_iteration_complex));
    Window win_input1 = win.broadcast_if_dimension_le_one(*src1);
    Window win_input2 = win.broadcast_if_dimension_le_one(*src2);

    AccessWindowHorizontal input1_access(src1, 0, num_elems_processed_per_iteration_complex);
    AccessWindowHorizontal input2_access(src2, 0, num_elems_processed_per_iteration_complex);
    AccessWindowHorizontal output_access(dst, 0, num_elems_processed_per_iteration_complex);

    bool window_changed = update_window_and_padding(win_input1, input1_access)
                          || update_window_and_padding(win_input2, input2_access)
                          || update_window_and_padding(win, output_access);

    Status err = (window_changed) ? ARM_COMPUTE_CREATE_ERROR(ErrorCode::RUNTIME_ERROR, "Insufficient Padding!") : Status{};
    return std::make_pair(err, win);
}
} // namespace

void ClComplexPixelWiseMultiplicationKernel::configure(const CLCompileContext &compile_context, ITensorInfo *src1, ITensorInfo *src2, ITensorInfo *dst, const ActivationLayerInfo &act_info)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(src1, src2, dst);
    ARM_COMPUTE_ERROR_THROW_ON(validate_arguments_complex(src1, src2, dst, act_info));

    // Calculate border size
    _border_size = calc_border_size(src1, src2, dst);

    // Configure kernel window
    auto win_config = validate_and_configure_window_complex(src1, src2, dst);
    ARM_COMPUTE_ERROR_THROW_ON(win_config.first);

    CLBuildOptions build_opts;
    build_opts.add_option("-DDATA_TYPE=" + get_cl_type_from_data_type(dst->data_type()));
    if(act_info.enabled())
    {
        build_opts.add_option("-DACTIVATION_TYPE=" + lower_string(string_from_activation_func(act_info.activation())));
        build_opts.add_option("-DA_VAL=" + float_to_string_with_full_precision(act_info.a()));
        build_opts.add_option("-DB_VAL=" + float_to_string_with_full_precision(act_info.b()));
    }

    // Create kernel
    _kernel = create_kernel(compile_context, "pixelwise_mul_complex", build_opts.options());

    ICLKernel::configure_internal(win_config.second);
}

Status ClComplexPixelWiseMultiplicationKernel::validate(const ITensorInfo *src1, const ITensorInfo *src2, const ITensorInfo *dst, const ActivationLayerInfo &act_info)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(src1, src2, dst);
    ARM_COMPUTE_RETURN_ON_ERROR(validate_arguments_complex(src1, src2, dst, act_info));
    ARM_COMPUTE_RETURN_ON_ERROR(validate_and_configure_window_complex(src1->clone().get(), src2->clone().get(), dst->clone().get()).first);

    return Status{};
}

void ClComplexPixelWiseMultiplicationKernel::run_op(ITensorPack &tensors, const Window &window, cl::CommandQueue &queue)
{
    ARM_COMPUTE_ERROR_ON_UNCONFIGURED_KERNEL(this);
    ARM_COMPUTE_ERROR_ON_INVALID_SUBWINDOW(ICLKernel::window(), window);

    const auto src_0 = utils::cast::polymorphic_downcast<const ICLTensor *>(tensors.get_const_tensor(TensorType::ACL_SRC_0));
    const auto src_1 = utils::cast::polymorphic_downcast<const ICLTensor *>(tensors.get_const_tensor(TensorType::ACL_SRC_1));
    auto       dst   = utils::cast::polymorphic_downcast<ICLTensor *>(tensors.get_tensor(TensorType::ACL_DST));

    const TensorShape &in_shape1 = src_0->info()->tensor_shape();
    const TensorShape &in_shape2 = src_1->info()->tensor_shape();
    const TensorShape &out_shape = dst->info()->tensor_shape();

    bool can_collapse = true;
    if(std::min(in_shape1.total_size(), in_shape2.total_size()) > 1)
    {
        can_collapse = (std::min(in_shape1.num_dimensions(), in_shape2.num_dimensions()) > Window::DimZ);
        for(size_t d = Window::DimZ; can_collapse && (d < out_shape.num_dimensions()); ++d)
        {
            can_collapse = (in_shape1[d] == in_shape2[d]);
        }
    }

    bool   has_collapsed = false;
    Window collapsed     = can_collapse ? window.collapse_if_possible(ICLKernel::window(), Window::DimZ, &has_collapsed) : window;

    const TensorShape &in_shape1_collapsed = has_collapsed ? in_shape1.collapsed_from(Window::DimZ) : in_shape1;
    const TensorShape &in_shape2_collapsed = has_collapsed ? in_shape2.collapsed_from(Window::DimZ) : in_shape2;

    Window slice        = collapsed.first_slice_window_3D();
    Window slice_input1 = slice.broadcast_if_dimension_le_one(in_shape1_collapsed);
    Window slice_input2 = slice.broadcast_if_dimension_le_one(in_shape2_collapsed);

    do
    {
        unsigned int idx = 0;
        add_3D_tensor_argument(idx, src_0, slice_input1);
        add_3D_tensor_argument(idx, src_1, slice_input2);
        add_3D_tensor_argument(idx, dst, slice);
        enqueue(queue, *this, slice, lws_hint());

        ARM_COMPUTE_UNUSED(collapsed.slide_window_slice_3D(slice_input1));
        ARM_COMPUTE_UNUSED(collapsed.slide_window_slice_3D(slice_input2));
    }
    while(collapsed.slide_window_slice_3D(slice));
}

BorderSize ClComplexPixelWiseMultiplicationKernel::border_size() const
{
    return _border_size;
}
} // namespace kernels
} // namespace opencl
} // namespace arm_compute