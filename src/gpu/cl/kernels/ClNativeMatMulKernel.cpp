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
#include "src/gpu/cl/kernels/ClNativeMatMulKernel.h"
#include "arm_compute/core/CL/ICLTensor.h"
#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/core/utils/misc/ShapeCalculator.h"
#include "src/core/helpers/AutoConfiguration.h"

#include "arm_compute/core/ITensorPack.h"
#include "src/common/utils/Log.h"
#include "src/core/helpers/WindowHelpers.h"
#include "support/Cast.h"
#include "utils/TypePrinter.h"

namespace arm_compute
{
namespace opencl
{
namespace kernels
{
namespace
{
Status validate_matmul_kernel_info(const MatMulKernelInfo &matmul_kernel_info)
{
    const bool adj_lhs = matmul_kernel_info.adj_lhs;
    const bool adj_rhs = matmul_kernel_info.adj_rhs;
    const int  m0      = matmul_kernel_info.m0;
    const int  n0      = matmul_kernel_info.n0;
    const int  k0      = matmul_kernel_info.k0;

    // Validate M0
    if(!adj_lhs)
    {
        // We support any positive integer, but will test & benchmark only 1 to 8 because > 8 will not efficient
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(m0 < 1, "Only positive integers are supported for M0 for Lhs non-transposed");
    }
    else
    {
        ARM_COMPUTE_RETURN_ERROR_ON_MSG((m0 & (m0 - 1)) && (m0 != 3) && (m0 > 16), "Only 1,2,3,4,8,16 are supported for N0 for Lhs transposed");
    }

    // Validate N0
    ARM_COMPUTE_RETURN_ERROR_ON_MSG((n0 & (n0 - 1)) && (n0 != 3) && (n0 > 16), "Only 1,2,3,4,8,16 are supported for N0");

    // Validate K0
    if(adj_lhs && !adj_rhs)
    {
        // We support any positive integer, but will test & benchmark only 1 to 8 because > 8 will not efficient
        ARM_COMPUTE_RETURN_ERROR_ON_MSG(k0 < 1, "Only positive integers are supported for K0 for Lhs transposed & Rhs non-transposed");
    }
    else
    {
        ARM_COMPUTE_RETURN_ERROR_ON_MSG((k0 & (k0 - 1)) && (k0 != 3) && (k0 > 16), "Only 1,2,3,4,8,16 are supported for K0");
    }

    return Status{};
}
}
ClNativeMatMulKernel::ClNativeMatMulKernel()
{
    _type = CLKernelType::GEMM;
}
Status ClNativeMatMulKernel::validate(const ITensorInfo *lhs, const ITensorInfo *rhs, const ITensorInfo *output, const MatMulKernelInfo &matmul_kernel_info)
{
    ARM_COMPUTE_RETURN_ERROR_ON_NULLPTR(lhs, rhs, output);
    ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(lhs, 1, DataType::F32, DataType::F16);
    ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(lhs, rhs);
    ARM_COMPUTE_RETURN_ON_ERROR(validate_matmul_kernel_info(matmul_kernel_info));

    if(output->total_size() != 0)
    {
        const TensorInfo tensor_info_output = output->clone()->set_tensor_shape(misc::shape_calculator::compute_batchmatmul_shape(lhs->tensor_shape(), rhs->tensor_shape(), matmul_kernel_info));
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_SHAPES(output, &tensor_info_output);
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(lhs, output);
    }
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(matmul_kernel_info.adj_lhs && matmul_kernel_info.adj_rhs, "LHS T and RHS T not implemented");
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(matmul_kernel_info.adj_lhs && !matmul_kernel_info.adj_rhs, "LHS T and RHS NT not implemented");

    return Status{};
}
void ClNativeMatMulKernel::configure(const ClCompileContext &compile_context, ITensorInfo *lhs, ITensorInfo *rhs, ITensorInfo *output, const MatMulKernelInfo &matmul_kernel_info)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(lhs, rhs, output, &compile_context, &matmul_kernel_info);
    ARM_COMPUTE_LOG_PARAMS(lhs, rhs, output, matmul_kernel_info);

    // output tensor auto initialization if not yet initialized
    auto_init_if_empty(*output, lhs->clone()->set_tensor_shape(misc::shape_calculator::compute_batchmatmul_shape(lhs->tensor_shape(), rhs->tensor_shape(), matmul_kernel_info)));
    ARM_COMPUTE_ERROR_THROW_ON(validate(lhs, rhs, output, matmul_kernel_info));

    const int m = output->dimension(1);
    const int n = output->dimension(0);
    const int k = matmul_kernel_info.adj_lhs ? lhs->tensor_shape().y() : lhs->tensor_shape().x();

    int m0 = std::min(matmul_kernel_info.m0, m);
    int n0 = adjust_vec_size(matmul_kernel_info.n0, n);

    // Configure kernel window
    Window win = calculate_max_window(*output, Steps(n0, m0));
    win        = win.collapse(win, Window::DimZ);
    IClKernel::configure_internal(win);

    // Calculate partial (store instead of load) M0 and partial N0 for the partial blocks at the end of a row/column if any. This is to avoid padding.
    const unsigned int partial_store_m0 = m % m0; // M is output->dimension(1)
    const unsigned int partial_store_n0 = n % n0; // N is output->dimension(0)

    CLBuildOptions build_opts;
    build_opts.add_option("-DDATA_TYPE=" + get_cl_type_from_data_type(lhs->data_type()));
    build_opts.add_option("-DM0=" + support::cpp11::to_string(m0));
    build_opts.add_option("-DN0=" + support::cpp11::to_string(n0));
    build_opts.add_option("-DK0=" + support::cpp11::to_string(matmul_kernel_info.k0));
    build_opts.add_option("-DPARTIAL_STORE_M0=" + support::cpp11::to_string(partial_store_m0));
    build_opts.add_option("-DPARTIAL_STORE_N0=" + support::cpp11::to_string(partial_store_n0));
    build_opts.add_option("-DK=" + support::cpp11::to_string(k));

    std::string kernel_name("mat_mul_native");
    kernel_name += matmul_kernel_info.adj_lhs ? "_t" : "_nt";
    kernel_name += matmul_kernel_info.adj_rhs ? "_t" : "_nt";

    if(matmul_kernel_info.adj_lhs)
    {
        ARM_COMPUTE_ERROR("Only Implemented LHS non-transposed kernels");
    }

    // A macro guard to compile ONLY the kernel of interest
    build_opts.add_option("-D" + upper_string(kernel_name));

    // Create kernel
    _kernel = create_kernel(compile_context, kernel_name, build_opts.options());

    // Set config_id for enabling LWS tuning
    _config_id = kernel_name;
    _config_id += "_";
    _config_id += lower_string(string_from_data_type(lhs->data_type()));
    _config_id += "_";
    _config_id += support::cpp11::to_string(output->dimension(1));
    _config_id += "_";
    _config_id += support::cpp11::to_string(output->dimension(0));
    _config_id += "_";
    _config_id += support::cpp11::to_string(output->dimension(2));
    _config_id += "_";
    _config_id += support::cpp11::to_string(m0);
    _config_id += "_";
    _config_id += support::cpp11::to_string(n0);
    _config_id += "_";
    _config_id += support::cpp11::to_string(matmul_kernel_info.k0);
}

void ClNativeMatMulKernel::run_op(ITensorPack &tensors, const Window &window, cl::CommandQueue &queue)
{
    ARM_COMPUTE_ERROR_ON_UNCONFIGURED_KERNEL(this);
    ARM_COMPUTE_ERROR_ON_INVALID_SUBWINDOW(ICLKernel::window(), window);

    const ICLTensor *lhs    = utils::cast::polymorphic_downcast<const ICLTensor *>(tensors.get_const_tensor(TensorType::ACL_SRC_0));
    const ICLTensor *rhs    = utils::cast::polymorphic_downcast<const ICLTensor *>(tensors.get_const_tensor(TensorType::ACL_SRC_1));
    ICLTensor       *output = utils::cast::polymorphic_downcast<ICLTensor *>(tensors.get_tensor(TensorType::ACL_DST));
    ARM_COMPUTE_ERROR_ON_NULLPTR(lhs, rhs, output);
    ARM_COMPUTE_LOG_PARAMS(lhs, rhs, output);

    unsigned int idx              = 0;
    Window       window_collapsed = window.collapse(ICLKernel::window(), Window::DimZ);

    add_3d_tensor_nhw_argument(idx, lhs);
    add_3d_tensor_nhw_argument(idx, rhs);
    add_3d_tensor_nhw_argument(idx, output);

    enqueue(queue, *this, window_collapsed, lws_hint());
}

} // namespace kernels
} // namespace opencl
} // namespace arm_compute
