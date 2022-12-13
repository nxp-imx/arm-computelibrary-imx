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
#ifndef TESTS_VALIDATION_FIXTURES_DYNAMIC_FUSION_GPU_CL_ELEMENTWISEBINARYFIXTURE
#define TESTS_VALIDATION_FIXTURES_DYNAMIC_FUSION_GPU_CL_ELEMENTWISEBINARYFIXTURE

#include "arm_compute/core/CL/CLKernelLibrary.h"
#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/core/Types.h"
#include "arm_compute/dynamic_fusion/runtime/gpu/cl/ClWorkloadRuntime.h"
#include "arm_compute/dynamic_fusion/sketch/gpu/GpuWorkloadSketch.h"
#include "arm_compute/dynamic_fusion/sketch/gpu/operators/GpuOutput.h"

#include "tests/CL/CLAccessor.h"
#include "tests/framework/Fixture.h"
#include "tests/framework/Macros.h"
#include "tests/validation/Validation.h"
#include "tests/validation/reference/ElementwiseOperations.h"
#include "tests/validation/reference/Permute.h"

using namespace arm_compute::experimental::dynamic_fusion;

namespace arm_compute
{
namespace test
{
namespace validation
{
template <typename TensorType, typename AccessorType, typename FunctionType, typename T>
class DynamicFusionGpuElementwiseBinaryValidationGenericFixture : public framework::Fixture
{
public:
    template <typename...>
    void setup(ArithmeticOperation op, TensorShape shape0, TensorShape shape1, TensorShape shape2, const DataType data_type, const bool is_inplace)
    {
        _op         = op;
        _is_inplace = is_inplace;
        _data_type  = data_type;
        _fuse       = shape2.total_size() != 0;
        ARM_COMPUTE_ERROR_ON_MSG(_fuse && _is_inplace, "In place for fusing case not supported yet.");
        _target    = compute_target(shape0, shape1, shape2);
        _reference = compute_reference(shape0, shape1, shape2);
    }

protected:
    template <typename U>
    void fill(U &&tensor, int i)
    {
        if(is_data_type_float(tensor.data_type()))
        {
            switch(_op)
            {
                case ArithmeticOperation::DIV:
                    library->fill_tensor_uniform_ranged(tensor, i, { std::pair<float, float>(-0.001f, 0.001f) });
                    break;
                case ArithmeticOperation::POWER:
                    library->fill_tensor_uniform(tensor, i, 0.0f, 5.0f);
                    break;
                default:
                    library->fill_tensor_uniform(tensor, i);
            }
        }
        else if(tensor.data_type() == DataType::S32)
        {
            switch(_op)
            {
                case ArithmeticOperation::DIV:
                    library->fill_tensor_uniform_ranged(tensor, i, { std::pair<int32_t, int32_t>(-1U, 1U) });
                    break;
                default:
                    library->fill_tensor_uniform(tensor, i);
            }
        }
        else
        {
            library->fill_tensor_uniform(tensor, i);
        }
    }

    TensorType compute_target(TensorShape shape0, TensorShape shape1, TensorShape shape2)
    {
        // Create a new workload sketch
        auto              cl_compile_ctx = CLKernelLibrary::get().get_compile_context();
        auto              gpu_ctx        = GpuWorkloadContext{ &cl_compile_ctx };
        GpuWorkloadSketch sketch{ &gpu_ctx };

        // Fuse first element wise binary Op
        auto       lhs_info = sketch.create_tensor_info(shape0, 1, _data_type);
        auto       rhs_info = sketch.create_tensor_info(TensorInfo(shape1, 1, _data_type));

        auto ans_info = sketch.create_tensor_info();
        auto dst_info = sketch.create_tensor_info();

        TensorInfo rhs_info_fuse;
        TensorInfo ans2_info;

        FunctionType::create_op(sketch, &lhs_info, &rhs_info, &ans_info);

        if(_fuse)
        {
            rhs_info_fuse = sketch.create_tensor_info(shape2, 1, _data_type);
            ans2_info = sketch.create_tensor_info();

            FunctionType::create_op(sketch, &ans_info, &rhs_info_fuse, &ans2_info);
            GpuOutput::create_op(sketch, &ans2_info, &dst_info);
        }
        else
        {
            GpuOutput::create_op(sketch, &ans_info, &dst_info);
        }

        // Configure runtime
        ClWorkloadRuntime runtime;
        runtime.configure(sketch);

        // (Important) Allocate auxiliary tensor memory if there are any
        for(auto &data : runtime.get_auxiliary_tensors())
        {
            TensorType   *tensor      = data.first;
            AuxMemoryInfo aux_mem_req = data.second;
            tensor->allocator()->init(*data.first->info(), aux_mem_req.alignment);
            tensor->allocator()->allocate();
        }

        // Construct user tensors
        TensorType t_lhs{};
        TensorType t_rhs{};
        TensorType t_rhs_fuse{};
        TensorType t_dst{};

        // Initialize user tensors
        t_lhs.allocator()->init(lhs_info);
        t_rhs.allocator()->init(rhs_info);
        t_dst.allocator()->init(dst_info);
        if(_fuse)
        {
            t_rhs_fuse.allocator()->init(rhs_info_fuse);
        }

        // Allocate and fill user tensors
        // Instead of using ACL allocator, the user can choose to import memory into the tensors
        t_lhs.allocator()->allocate();
        t_rhs.allocator()->allocate();
        t_dst.allocator()->allocate();
        if(_fuse)
        {
            t_rhs_fuse.allocator()->allocate();
        }

        fill(AccessorType(t_lhs), 0);
        fill(AccessorType(t_rhs), 1);
        if(_fuse)
        {
            fill(AccessorType(t_rhs_fuse), 2);
        }

        // Run runtime
        if(_fuse)
        {
            runtime.run({ &t_lhs, &t_rhs, &t_rhs_fuse, &t_dst });
        }
        else
        {
            runtime.run({ &t_lhs, &t_rhs, &t_dst });
        }

        return t_dst;
    }

    SimpleTensor<T> compute_reference(TensorShape shape0, TensorShape shape1, TensorShape shape2)
    {
        const TensorShape out_shape      = TensorShape::broadcast_shape(shape0, shape1);
        const TensorShape out_shape_fuse = TensorShape::broadcast_shape(out_shape, shape1);

        // Create reference
        SimpleTensor<T> ref_lhs{ shape0, _data_type, 1, QuantizationInfo() };
        SimpleTensor<T> ref_rhs{ shape1, _data_type, 1, QuantizationInfo() };
        SimpleTensor<T> ref_rhs_fuse{ shape2, _data_type, 1, QuantizationInfo() };
        SimpleTensor<T> ref_dst{ out_shape, _data_type, 1, QuantizationInfo() };
        SimpleTensor<T> ref_dst_fuse{ out_shape_fuse, _data_type, 1, QuantizationInfo() };
        // Fill reference
        fill(ref_lhs, 0);
        fill(ref_rhs, 1);

        reference::arithmetic_operation<T>(_op, ref_lhs, ref_rhs, ref_dst, ConvertPolicy::WRAP);
        if(_fuse)
        {
            fill(ref_rhs_fuse, 2);
            reference::arithmetic_operation<T>(_op, ref_dst, ref_rhs_fuse, ref_dst_fuse, ConvertPolicy::WRAP);
        }
        SimpleTensor<T> *ret = _fuse ? &ref_dst_fuse : &ref_dst;
        return *ret;
    }

    ArithmeticOperation _op{ ArithmeticOperation::ADD };
    TensorType          _target{};
    SimpleTensor<T>     _reference{};
    DataType            _data_type{};
    DataLayout          _data_layout{};
    bool                _is_inplace{ false };
    bool                _fuse{ false };
};

template <typename TensorType, typename AccessorType, typename FunctionType, typename T>
class DynamicFusionGpuElementwiseBinaryOneOpValidationFixture : public DynamicFusionGpuElementwiseBinaryValidationGenericFixture<TensorType, AccessorType, FunctionType, T>
{
public:
    template <typename...>
    void setup(ArithmeticOperation op, TensorShape shape, const DataType data_type, const bool is_inplace)
    {
        DynamicFusionGpuElementwiseBinaryValidationGenericFixture<TensorType, AccessorType, FunctionType, T>::setup(op, shape, shape, TensorShape(), data_type, is_inplace);
    }
};

template <typename TensorType, typename AccessorType, typename FunctionType, typename T>
class DynamicFusionGpuElementwiseBinaryBroadcastOneOpValidationFixture : public DynamicFusionGpuElementwiseBinaryValidationGenericFixture<TensorType, AccessorType, FunctionType, T>
{
public:
    template <typename...>
    void setup(ArithmeticOperation op, TensorShape shape0, TensorShape shape1, const DataType data_type, const bool is_inplace)
    {
        DynamicFusionGpuElementwiseBinaryValidationGenericFixture<TensorType, AccessorType, FunctionType, T>::setup(op, shape0, shape1, TensorShape(), data_type, is_inplace);
    }
};

template <typename TensorType, typename AccessorType, typename FunctionType, typename T>
class DynamicFusionGpuElementwiseBinaryTwoOpsValidationFixture : public DynamicFusionGpuElementwiseBinaryValidationGenericFixture<TensorType, AccessorType, FunctionType, T>
{
public:
    template <typename...>
    void setup(ArithmeticOperation op, TensorShape shape0, TensorShape shape1, TensorShape shape2, const DataType data_type, const bool is_inplace)
    {
        DynamicFusionGpuElementwiseBinaryValidationGenericFixture<TensorType, AccessorType, FunctionType, T>::setup(op, shape0, shape1, shape2, data_type, is_inplace);
    }
};

} // namespace validation
} // namespace test
} // namespace arm_compute
#endif /* TESTS_VALIDATION_FIXTURES_DYNAMIC_FUSION_GPU_CL_ELEMENTWISEBINARYFIXTURE */
