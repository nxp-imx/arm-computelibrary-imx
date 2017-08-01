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
#ifndef __ARM_COMPUTE_TEST_DEPTHWISE_SEPARABLE_CONVOLUTION_LAYER_H__
#define __ARM_COMPUTE_TEST_DEPTHWISE_SEPARABLE_CONVOLUTION_LAYER_H__

#include "tests/SimpleTensor.h"
#include "tests/validation_new/Helpers.h"

namespace arm_compute
{
namespace test
{
namespace validation
{
namespace reference
{
template <typename T>
SimpleTensor<T> depthwise_separable_convolution_layer(const SimpleTensor<T> &src, const SimpleTensor<T> &depthwise_weights, const TensorShape &depthwise_out_shape,
                                                      const SimpleTensor<T> &pointwise_weights,
                                                      const SimpleTensor<T> &biases, const TensorShape &dst_shape, const PadStrideInfo &depthwise_conv_info, const PadStrideInfo &pointwise_conv_info);
} // namespace reference
} // namespace validation
} // namespace test
} // namespace arm_compute
#endif /* __ARM_COMPUTE_TEST_DEPTHWISE_SEPARABLE_CONVOLUTION_LAYER_H__ */
