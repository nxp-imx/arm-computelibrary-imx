/*
 * Copyright (c) 2020 Arm Limited.
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
#ifndef UTILS_CORE_SCALEUTILS_H
#define UTILS_CORE_SCALEUTILS_H

#include "arm_compute/core/Types.h"

#include <cstdint>
#include <cstdlib>

namespace arm_compute
{
namespace scale_utils
{
/** Returns resize ratio between input and output with consideration of aligned corners
 *
 * @param[in] input_size    The input size
 * @param[in] output_size   the output size
 * @param[in] align_corners True to align corners of input and output. Defaults to false.
 *
 * @return The ratio between input and output (i.e., the input size divided by the output size)
 */
float calculate_resize_ratio(size_t input_size, size_t output_size, bool align_corners = false);

/** Returns if aligned corners are allowed for the given sampling policy
 *
 * @param[in] sampling_policy The sampling policy to consider
 *
 * @return True if aligned corners are allowed
 */
inline bool is_align_corners_allowed_sampling_policy(SamplingPolicy sampling_policy)
{
    return sampling_policy != SamplingPolicy::CENTER;
}

/** Returns if aligned corners are allowed for the given output shape
 *
 * @param[in] output_shape The shape of the scaled output tensor
 * @param[in] layout       The data layout of the output tensor
 *
 * @return True if aligned corners are allowed
 */
bool is_align_corners_allowed_output_shape(const TensorShape &output_shape, DataLayout layout);
} // namespace scale_utils
} // namespace arm_compute
#endif /* UTILS_CORE_SCALEUTILS_H */