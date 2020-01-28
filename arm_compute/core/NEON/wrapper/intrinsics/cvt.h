/*
 * Copyright (c) 2020 ARM Limited.
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
#ifndef ARM_COMPUTE_WRAPPER_CVT_H
#define ARM_COMPUTE_WRAPPER_CVT_H

#include <arm_neon.h>

namespace arm_compute
{
namespace wrapper
{
#define VCVT_TO_F32_IMPL(ptype, vtype, prefix, postfix1, postfix2)                   \
    template <typename T>                                                            \
    inline typename std::enable_if<std::is_same<T, float>::value, float32x4_t>::type \
    vcvt(const vtype &a)                                                             \
    {                                                                                \
        return prefix##_##postfix1##_##postfix2(a);                                  \
    }

VCVT_TO_F32_IMPL(float32x4_t, uint32x4_t, vcvtq, f32, u32)
VCVT_TO_F32_IMPL(float32x4_t, int32x4_t, vcvtq, f32, s32)
#undef VCVT_TO_F32_IMPL

template <typename T>
inline typename std::enable_if<std::is_same<T, uint8_t>::value, uint32x4_t>::type
vcvt(const float32x4_t &a)
{
    return vcvtq_u32_f32(a);
}

template <typename T>
inline typename std::enable_if<std::is_same<T, int8_t>::value, int32x4_t>::type
vcvt(const float32x4_t &a)
{
    return vcvtq_s32_f32(a);
}

} // namespace wrapper
} // namespace arm_compute
#endif /* ARM_COMPUTE_WRAPPER_CVT_H */
