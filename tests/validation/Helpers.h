/*
 * Copyright (c) 2017 ARM Limited.
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
#ifndef __ARM_COMPUTE_TEST_VALIDATION_HELPERS_H__
#define __ARM_COMPUTE_TEST_VALIDATION_HELPERS_H__

#include "arm_compute/core/Types.h"
#include "arm_compute/core/Utils.h"
#include "tests/Globals.h"

#include <random>
#include <type_traits>
#include <utility>

namespace arm_compute
{
namespace test
{
namespace validation
{
template <typename T>
struct is_floating_point : public std::is_floating_point<T>
{
};

template <>
struct is_floating_point<half> : public std::true_type
{
};

/** Helper function to get the testing range for each activation layer.
 *
 * @param[in] activation           Activation function to test.
 * @param[in] data_type            Data type.
 * @param[in] fixed_point_position Number of bits for the fractional part. Defaults to 1.
 *
 * @return A pair containing the lower upper testing bounds for a given function.
 */
template <typename T>
std::pair<T, T> get_activation_layer_test_bounds(ActivationLayerInfo::ActivationFunction activation, DataType data_type, int fixed_point_position = 0)
{
    std::pair<T, T> bounds;

    switch(data_type)
    {
        case DataType::F16:
        {
            using namespace half_float::literal;

            switch(activation)
            {
                case ActivationLayerInfo::ActivationFunction::SQUARE:
                case ActivationLayerInfo::ActivationFunction::LOGISTIC:
                case ActivationLayerInfo::ActivationFunction::SOFT_RELU:
                    // Reduce range as exponent overflows
                    bounds = std::make_pair(-10._h, 10._h);
                    break;
                case ActivationLayerInfo::ActivationFunction::SQRT:
                    // Reduce range as sqrt should take a non-negative number
                    bounds = std::make_pair(0._h, 255._h);
                    break;
                default:
                    bounds = std::make_pair(-255._h, 255._h);
                    break;
            }
            break;
        }
        case DataType::F32:
            switch(activation)
            {
                case ActivationLayerInfo::ActivationFunction::LOGISTIC:
                case ActivationLayerInfo::ActivationFunction::SOFT_RELU:
                    // Reduce range as exponent overflows
                    bounds = std::make_pair(-40.f, 40.f);
                    break;
                case ActivationLayerInfo::ActivationFunction::SQRT:
                    // Reduce range as sqrt should take a non-negative number
                    bounds = std::make_pair(0.f, 255.f);
                    break;
                default:
                    bounds = std::make_pair(-255.f, 255.f);
                    break;
            }
            break;
        case DataType::QS8:
        case DataType::QS16:
            switch(activation)
            {
                case ActivationLayerInfo::ActivationFunction::LOGISTIC:
                case ActivationLayerInfo::ActivationFunction::SOFT_RELU:
                case ActivationLayerInfo::ActivationFunction::TANH:
                    // Reduce range as exponent overflows
                    bounds = std::make_pair(-(1 << fixed_point_position), 1 << fixed_point_position);
                    break;
                case ActivationLayerInfo::ActivationFunction::SQRT:
                    // Reduce range as sqrt should take a non-negative number
                    // Can't be zero either as inv_sqrt is used in NEON.
                    bounds = std::make_pair(1, std::numeric_limits<T>::max());
                    break;
                default:
                    bounds = std::make_pair(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max());
                    break;
            }
            break;
        default:
            ARM_COMPUTE_ERROR("Unsupported data type");
    }

    return bounds;
}

/** Fill mask with the corresponding given pattern.
 *
 * @param[in,out] mask    Mask to be filled according to pattern
 * @param[in]     cols    Columns (width) of mask
 * @param[in]     rows    Rows (height) of mask
 * @param[in]     pattern Pattern to fill the mask according to
 */
inline void fill_mask_from_pattern(uint8_t *mask, int cols, int rows, MatrixPattern pattern)
{
    unsigned int                v = 0;
    std::mt19937                gen(library->seed());
    std::bernoulli_distribution dist(0.5);

    for(int r = 0; r < rows; ++r)
    {
        for(int c = 0; c < cols; ++c, ++v)
        {
            uint8_t val = 0;

            switch(pattern)
            {
                case MatrixPattern::BOX:
                    val = 255;
                    break;
                case MatrixPattern::CROSS:
                    val = ((r == (rows / 2)) || (c == (cols / 2))) ? 255 : 0;
                    break;
                case MatrixPattern::DISK:
                    val = (((r - rows / 2.0f + 0.5f) * (r - rows / 2.0f + 0.5f)) / ((rows / 2.0f) * (rows / 2.0f)) + ((c - cols / 2.0f + 0.5f) * (c - cols / 2.0f + 0.5f)) / ((cols / 2.0f) *
                            (cols / 2.0f))) <= 1.0f ? 255 : 0;
                    break;
                case MatrixPattern::OTHER:
                    val = (dist(gen) ? 0 : 255);
                    break;
                default:
                    return;
            }

            mask[v] = val;
        }
    }

    if(pattern == MatrixPattern::OTHER)
    {
        std::uniform_int_distribution<uint8_t> distribution_u8(0, ((cols * rows) - 1));
        mask[distribution_u8(gen)] = 255;
    }
}

/** Calculate output tensor shape give a vector of input tensor to concatenate
 *
 * @param[in] input_shapes Shapes of the tensors to concatenate across depth.
 *
 * @return The shape of output concatenated tensor.
 */
TensorShape calculate_depth_concatenate_shape(const std::vector<TensorShape> &input_shapes);

/** Helper function to fill the Lut random by a ILutAccessor.
 *
 * @param[in,out] table Accessor at the Lut.
 *
 */
template <typename T>
void fill_lookuptable(T &&table)
{
    std::mt19937                                          generator(library->seed());
    std::uniform_int_distribution<typename T::value_type> distribution(std::numeric_limits<typename T::value_type>::min(), std::numeric_limits<typename T::value_type>::max());

    for(int i = std::numeric_limits<typename T::value_type>::min(); i <= std::numeric_limits<typename T::value_type>::max(); i++)
    {
        table[i] = distribution(generator);
    }
}
} // namespace validation
} // namespace test
} // namespace arm_compute
#endif /* __ARM_COMPUTE_TEST_VALIDATION_HELPERS_H__ */
