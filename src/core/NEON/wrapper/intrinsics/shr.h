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

#ifndef ARM_COMPUTE_WRAPPER_SHR_H
#define ARM_COMPUTE_WRAPPER_SHR_H

#include <type_traits>
#include <arm_neon.h>

namespace arm_compute
{
namespace wrapper
{

#define VQRSHRN_IMPL(half_vtype, vtype, prefix, postfix) \
    template <int b>                                     \
    inline half_vtype vqrshrn(const vtype &a)            \
    {                                                    \
        return prefix##_##postfix(a, b);                 \
    }

VQRSHRN_IMPL(int8x8_t, int16x8_t, vqrshrn_n, s16)
VQRSHRN_IMPL(uint8x8_t, uint16x8_t, vqrshrn_n, u16)
VQRSHRN_IMPL(int16x4_t, int32x4_t, vqrshrn_n, s32)
VQRSHRN_IMPL(uint16x4_t, uint32x4_t, vqrshrn_n, u32)
VQRSHRN_IMPL(int32x2_t, int64x2_t, vqrshrn_n, s64)
VQRSHRN_IMPL(uint32x2_t, uint64x2_t, vqrshrn_n, u64)

#undef VQRSHRN_IMPL

// This function is the mixed version of VQRSHRN and VQRSHRUN.
// The input vector is always signed integer, while the returned vector
// can be either signed or unsigned depending on the signedness of scalar type T.
#define VQRSHRN_EX_IMPL(half_vtype, vtype, prefix_signed, prefix_unsigned, postfix)                              \
    template <int b, typename T>                                                                                 \
    inline typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, half_vtype>::type     \
    vqrshrn_ex(const vtype &a)                                                                                   \
    {                                                                                                            \
        return prefix_signed##_##postfix(a, b);                                                                  \
    }                                                                                                            \
                                                                                                                 \
    template <int b, typename T>                                                                                 \
    inline typename std::enable_if<std::is_integral<T>::value && !std::is_signed<T>::value, u##half_vtype>::type \
    vqrshrn_ex(const vtype &a)                                                                                   \
    {                                                                                                            \
        return prefix_unsigned##_##postfix(a, b);                                                                \
    }

VQRSHRN_EX_IMPL(int8x8_t, int16x8_t, vqrshrn_n, vqrshrun_n, s16)
VQRSHRN_EX_IMPL(int16x4_t, int32x4_t, vqrshrn_n, vqrshrun_n, s32)
VQRSHRN_EX_IMPL(int32x2_t, int64x2_t, vqrshrn_n, vqrshrun_n, s64)

#undef VQRSHRN_EX_IMPL

} // namespace wrapper
} // namespace arm_compute
#endif /* ARM_COMPUTE_WRAPPER_SHR_H */