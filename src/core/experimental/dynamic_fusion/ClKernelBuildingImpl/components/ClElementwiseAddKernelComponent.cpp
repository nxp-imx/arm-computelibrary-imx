/*
 * Copyright (c) 2022 Arm Limited.
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
#if defined(ENABLE_EXPERIMENTAL_DYNAMIC_FUSION)

#include "src/core/experimental/dynamic_fusion/ClKernelBuildingImpl/components/ClElementwiseAddKernelComponent.h"
#include "arm_compute/core/Validate.h"
#include "src/core/helpers/AutoConfiguration.h"
#include "src/core/helpers/WindowHelpers.h"

namespace arm_compute
{
namespace experimental
{
namespace dynamic_fusion
{
ComponentType ClElementwiseAddKernelComponent::get_component_type() const
{
    return ComponentType::Simple;
}

std::set<std::string> ClElementwiseAddKernelComponent::get_headers_list() const
{
    return std::set<std::string> { "common/experimental/gemm_fused_post_ops/fp_mixed_precision_helpers.h", "gemm_helpers.h", "repeat.h", "tile_helpers.h" };
}

Window ClElementwiseAddKernelComponent::get_window() const
{
    const ITensorInfo *lhs_info = _blueprint->impl().get_kernel_argument_info(_lhs.arg_id);
    const ITensorInfo *rhs_info = _blueprint->impl().get_kernel_argument_info(_rhs.arg_id);
    ITensorInfo       *dst_info = _blueprint->impl().get_kernel_argument_info(_blueprint->impl().get_dst_id());

    ARM_COMPUTE_ERROR_ON_NULLPTR(lhs_info, rhs_info, dst_info);

    const std::pair<TensorShape, ValidRegion> broadcast_pair = ITensorInfo::broadcast_shape_and_valid_region(*lhs_info, *rhs_info);
    const TensorShape &out_shape = broadcast_pair.first;

    auto_init_if_empty(*dst_info, out_shape, 1, lhs_info->data_type());

    const unsigned int vector_size_byte_opencl           = 16;
    const unsigned int num_elems_processed_per_iteration = adjust_vec_size(vector_size_byte_opencl / dst_info->element_size(), dst_info->dimension(0));
    Window             win                               = calculate_max_window(*dst_info, Steps(num_elems_processed_per_iteration));

    return win;
}

std::string ClElementwiseAddKernelComponent::get_component_code() const
{
    std::string code;
    return R"_(
    //------------------ START KERNEL {{meta_kernel_id}} ELTWISE_ADD ---------------------
    // IN_0(Accumulator)   {{acc}}
    // IN_1(Addend)                {{addend}}

    // c = addend + c (mix-precision, broadcast, boundary aware)
    {
        __global uchar *addend_addr = {{addend}}_ptr + {{addend}}_offset_first_element_in_bytes + (get_global_id(0) * (uint)N0 * sizeof(DATA_TYPE)) + (COMPUTE_M0_START_ROW(g_y, M0, PARTIAL_STORE_M0) * {{addend}}_stride_y) + get_global_id(2) * {{addend}}_stride_z; \
        LOAD_BLOCK_BOUNDARY_AWARE(M0, N0, DATA_TYPE, addend, addend_addr, 0, {{addend}}_stride_y, g_zero, PARTIAL_LOAD_M0, PARTIAL_LOAD_N0, PARTIAL_COND_Y, PARTIAL_COND_X);                                                                                        \
        MIXED_PRECISION_ELTWISE_OP_BLOCK(ADD_X_POS_0, M0, N0, {{acc}}, addend, DATA_TYPE_ACCUMULATOR, addend_hp);
    }

    // Workaround for the discrepancy between tiles and repeats
#if defined(IS_TILED)
    {{acc}}[0].v = {{acc}}0;
#if M0 >= 2
    {{acc}}[1].v = {{acc}}1;
#endif // M0 >= 2
#if M0 >= 3
    {{acc}}[2].v = {{acc}}2;
#endif // M0 >= 3
#if M0 >= 4
    {{acc}}[3].v = {{acc}}3;
#endif // M0 >= 4
#if M0 >= 8
    {{acc}}[4].v = {{acc}}4;
    {{acc}}[5].v = {{acc}}5;
    {{acc}}[6].v = {{acc}}6;
    {{acc}}[7].v = {{acc}}7;
#endif // M0 >= 8
#if M0 == 16
    {{acc}}[8].v = {{acc}}8;
    {{acc}}[9].v = {{acc}}9;
    {{acc}}[10].v = {{acc}}A;
    {{acc}}[11].v = {{acc}}B;
    {{acc}}[12].v = {{acc}}C;
    {{acc}}[13].v = {{acc}}D;
    {{acc}}[14].v = {{acc}}E;
    {{acc}}[15].v = {{acc}}F;
#endif // M0 == 16
#endif // defined(IS_TILED)
    //------------------ END KERNEL {{meta_kernel_id}} ELTWISE_ADD ---------------------

)_";
}

CLBuildOptions ClElementwiseAddKernelComponent::generate_build_options() const
{
    auto t_dst_info = _blueprint->impl().get_kernel_argument_info(_blueprint->impl().get_dst_id());
    auto tile_info  = _blueprint->impl().get_tile_info();

    CLBuildOptions build_opts{};

    build_opts.add_option("-DDATA_TYPE=" + get_cl_type_from_data_type(t_dst_info->data_type()));
    build_opts.add_option("-DM0=" + support::cpp11::to_string(tile_info.tile_dims.y()));
    build_opts.add_option("-DN0=" + support::cpp11::to_string(tile_info.tile_dims.x()));
    build_opts.add_option("-DPARTIAL_STORE_M0=" + support::cpp11::to_string(tile_info.boundaries.y() % tile_info.tile_dims.y()));

    return build_opts;
}

std::string ClElementwiseAddKernelComponent::generate_config_id() const
{
    auto        t_dst_info = _blueprint->impl().get_kernel_argument_info(_blueprint->impl().get_dst_id());
    std::string config_id{};
    config_id += lower_string(string_from_data_type(t_dst_info->data_type()));
    config_id += "_";
    config_id += support::cpp11::to_string(t_dst_info->dimension(0));
    config_id += "_";
    config_id += support::cpp11::to_string(t_dst_info->dimension(1));
    config_id += "_";
    config_id += lower_string(string_from_data_layout(t_dst_info->data_layout()));
    return config_id;
}

ClElementwiseAddKernelComponent::TagLUT ClElementwiseAddKernelComponent::allocate_vars(SharedVarTable &vtable) const
{
    // Determine which argument is the accumulator
    Link accumulator;
    Link addend;
    if(_lhs.group == SharedVarGroup::Automatic)
    {
        accumulator = _lhs;
        addend      = _rhs;
    }
    else if(_rhs.group == SharedVarGroup::Automatic)
    {
        accumulator = _rhs;
        addend      = _lhs;
    }
    else
    {
        ARM_COMPUTE_ERROR("Invalid elementwise component linking");
    }
    return {
        { "meta_kernel_id", id() },
        { "acc", vtable.add(accumulator, ClKernelArgRuntimeDescriptor(accumulator.arg_id, TensorArgType::Image_3D), "add_acc") },
        { "addend", vtable.add(addend, ClKernelArgRuntimeDescriptor(addend.arg_id, TensorArgType::Image_3D), "add_addend") },
        // {"dst", vtable.add(_dst, ClKernelArgRuntimeDescriptor(_dst.arg_id, TensorArgType::Image_3D), "dst")}, // dst is needed for the root version and/or non-inplace version should we need one
    };
}
} // namespace dynamic_fusion
} // namespace experimental
} // namespace arm_compute

#endif // defined(ENABLE_EXPERIMENTAL_DYNAMIC_FUSION)