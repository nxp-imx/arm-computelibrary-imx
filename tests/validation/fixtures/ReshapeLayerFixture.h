/*
 * Copyright (c) 2017-2021, 2023 Arm Limited.
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
#ifndef ACL_TESTS_VALIDATION_FIXTURES_RESHAPELAYERFIXTURE_H
#define ACL_TESTS_VALIDATION_FIXTURES_RESHAPELAYERFIXTURE_H

#include "arm_compute/core/TensorShape.h"
#include "arm_compute/core/Types.h"
#include "tests/AssetsLibrary.h"
#include "tests/Globals.h"
#include "tests/IAccessor.h"
#include "tests/framework/Asserts.h"
#include "tests/framework/Fixture.h"
#include "tests/validation/Helpers.h"
#include "tests/validation/reference/ReshapeLayer.h"

namespace arm_compute
{
namespace test
{
namespace validation
{
/** [ReshapeLayer fixture] **/
template <typename TensorType, typename AccessorType, typename FunctionType, typename T>
class ReshapeLayerGenericValidationFixture : public framework::Fixture
{
public:
    void setup(TensorShape input_shape, TensorShape output_shape, DataType data_type, bool add_x_padding = false)
    {
        _target    = compute_target(input_shape, output_shape, data_type, add_x_padding);
        _reference = compute_reference(input_shape, output_shape, data_type);
    }

protected:
    template <typename U>
    void fill(U &&tensor, int i)
    {
        library->fill_tensor_uniform(tensor, i);
    }

    TensorType compute_target(const TensorShape &input_shape, const TensorShape &output_shape, DataType data_type, bool add_x_padding = false)
    {
        // Check if indeed the input shape can be reshape to the output one
        ARM_COMPUTE_ASSERT(input_shape.total_size() == output_shape.total_size());

        // Create tensors
        TensorType src = create_tensor<TensorType>(input_shape, data_type);
        TensorType dst = create_tensor<TensorType>(output_shape, data_type);

        // Create and configure function
        FunctionType reshape;

        reshape.configure(&src, &dst);

        ARM_COMPUTE_ASSERT(src.info()->is_resizable());
        ARM_COMPUTE_ASSERT(dst.info()->is_resizable());

        if(add_x_padding)
        {
            // Add random padding in x dimension
            add_padding_x({ &src, &dst });
        }

        // Allocate tensors
        src.allocator()->allocate();
        dst.allocator()->allocate();

        ARM_COMPUTE_ASSERT(!src.info()->is_resizable());
        ARM_COMPUTE_ASSERT(!dst.info()->is_resizable());

        // Fill tensors
        fill(AccessorType(src), 0);

        // Compute function
        reshape.run();

        return dst;
    }

    SimpleTensor<T> compute_reference(const TensorShape &input_shape, const TensorShape &output_shape, DataType data_type)
    {
        // Create reference
        SimpleTensor<T> src{ input_shape, data_type };

        // Fill reference
        fill(src, 0);

        return reference::reshape_layer<T>(src, output_shape);
    }

    TensorType      _target{};
    SimpleTensor<T> _reference{};
};

template <typename TensorType, typename AccessorType, typename FunctionType, typename T>
class ReshapeLayerValidationFixture : public ReshapeLayerGenericValidationFixture<TensorType, AccessorType, FunctionType, T>
{
public:
    void setup(TensorShape input_shape, TensorShape output_shape, DataType data_type)
    {
        ReshapeLayerGenericValidationFixture<TensorType, AccessorType, FunctionType, T>::setup(input_shape, output_shape, data_type);
    }
};
template <typename TensorType, typename AccessorType, typename FunctionType, typename T>
class ReshapeLayerPaddedValidationFixture : public ReshapeLayerGenericValidationFixture<TensorType, AccessorType, FunctionType, T>
{
public:
    void setup(TensorShape input_shape, TensorShape output_shape, DataType data_type)
    {
        ReshapeLayerGenericValidationFixture<TensorType, AccessorType, FunctionType, T>::setup(input_shape, output_shape, data_type, true /* add_x_padding */);
    }
};
/** [ReshapeLayer fixture] **/
} // namespace validation
} // namespace test
} // namespace arm_compute
#endif // ACL_TESTS_VALIDATION_FIXTURES_RESHAPELAYERFIXTURE_H
