/*
 * Copyright (c) 2021, 2023 Arm Limited.
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

#if defined(__aarch64__)

#include "arm_gemm.hpp"
#include <cstddef>
#include <cstdint>

namespace arm_conv {
namespace depthwise {

void a64_u8q_nhwc_generic_output9_mla_depthfirst_impl(
  const uint8_t *const *const inptrs,
  uint8_t *const *const outptrs,
  const void *params,
  const arm_gemm::Requantize32& qp,
  const unsigned int n_points,
  const unsigned int n_channels
)
{
  __asm__ __volatile__(
    "lsr x12, %x[n_channels], #0x2\n"
    "add x20, %x[qp], %[offsetof_Requantize32_minval]\n"
    "ld1r { v8.4s }, [x20]\n"
    "add x20, %x[qp], %[offsetof_Requantize32_maxval]\n"
    "ld1r { v7.4s }, [x20]\n"
    "add x20, %x[qp], %[offsetof_Requantize32_a_offset]\n"
    "ld1r { v6.16b }, [x20]\n"
    "add x20, %x[qp], %[offsetof_Requantize32_b_offset]\n"
    "ld1r { v5.16b }, [x20]\n"
    "add x20, %x[qp], %[offsetof_Requantize32_c_offset]\n"
    "ld1r { v4.4s }, [x20]\n"
    "add x20, %x[qp], %[offsetof_Requantize32_per_layer_left_shift]\n"
    "ld1r { v3.4s }, [x20]\n"
    "add x20, %x[qp], %[offsetof_Requantize32_per_layer_mul]\n"
    "ld1r { v2.4s }, [x20]\n"
    "add x20, %x[qp], %[offsetof_Requantize32_per_layer_right_shift]\n"
    "ld1r { v1.4s }, [x20]\n"
    "mov x11, #0x0\n"
    "cbz x12, 6f\n"
    "1:"  // Channel loop
    "movi v23.4s, #0x0\n"
    "cbz %x[bias], 2f\n"
    "lsl x20, x11, #0x2\n"
    "ldr q23, [%x[bias], x20]\n"
    "2:"  // Channel loop: Load bias: Done
    "ldr s0, [%x[params]], #0x4\n"
    "mov x21, %x[inptrs]\n"
    "ldp x10, x9, [x21], #0x10\n"
    "subs x20, %x[n_points], #0x1\n"
    "ldr s14, [x10, x11]\n"
    "ldr s15, [x9, x11]\n"
    "mov v24.16b, v23.16b\n"
    "mov v25.16b, v23.16b\n"
    "ldp x28, x27, [x21], #0x10\n"
    "ldr s16, [x28, x11]\n"
    "mov v26.16b, v23.16b\n"
    "mov v27.16b, v23.16b\n"
    "ldr s17, [x27, x11]\n"
    "ldp x26, x25, [x21], #0x10\n"
    "mov v28.16b, v23.16b\n"
    "mov v29.16b, v23.16b\n"
    "ldr s18, [x26, x11]\n"
    "ldr s19, [x25, x11]\n"
    "mov v30.16b, v23.16b\n"
    "mov v31.16b, v23.16b\n"
    "ldp x24, x23, [x21], #0x10\n"
    "ldr s20, [x24, x11]\n"
    "usubl v0.8h, v0.8b, v5.8b\n"
    "usubl v14.8h, v14.8b, v6.8b\n"
    "ldr s21, [x23, x11]\n"
    "ldr x22, [x21], #0x8\n"
    "usubl v15.8h, v15.8b, v6.8b\n"
    "usubl v16.8h, v16.8b, v6.8b\n"
    "ldr s22, [x22, x11]\n"
    "usubl v17.8h, v17.8b, v6.8b\n"
    "usubl v18.8h, v18.8b, v6.8b\n"
    "usubl v19.8h, v19.8b, v6.8b\n"
    "usubl v20.8h, v20.8b, v6.8b\n"
    "usubl v21.8h, v21.8b, v6.8b\n"
    "usubl v22.8h, v22.8b, v6.8b\n"
    "ble 4f\n"
    "3:"  // Channel loop: Planar loop
    "ldp x10, x9, [x21], #0x10\n"
    "ldp x28, x27, [x21], #0x10\n"
    "smlal v23.4s, v14.4h, v0.4h\n"
    "smlal v24.4s, v15.4h, v0.4h\n"
    "ldr s14, [x10, x11]\n"
    "ldr s15, [x9, x11]\n"
    "smlal v25.4s, v16.4h, v0.4h\n"
    "smlal v26.4s, v17.4h, v0.4h\n"
    "ldr s16, [x28, x11]\n"
    "ldr s17, [x27, x11]\n"
    "smlal v27.4s, v18.4h, v0.4h\n"
    "smlal v28.4s, v19.4h, v0.4h\n"
    "ldp x26, x25, [x21], #0x10\n"
    "ldr s18, [x26, x11]\n"
    "smlal v29.4s, v20.4h, v0.4h\n"
    "smlal v30.4s, v21.4h, v0.4h\n"
    "ldr s19, [x25, x11]\n"
    "ldp x24, x23, [x21], #0x10\n"
    "smlal v31.4s, v22.4h, v0.4h\n"
    "subs x20, x20, #0x1\n"
    "ldr s0, [%x[params]], #0x4\n"
    "ldr s20, [x24, x11]\n"
    "usubl v0.8h, v0.8b, v5.8b\n"
    "usubl v14.8h, v14.8b, v6.8b\n"
    "ldr s21, [x23, x11]\n"
    "ldr x22, [x21], #0x8\n"
    "usubl v15.8h, v15.8b, v6.8b\n"
    "usubl v16.8h, v16.8b, v6.8b\n"
    "ldr s22, [x22, x11]\n"
    "usubl v17.8h, v17.8b, v6.8b\n"
    "usubl v18.8h, v18.8b, v6.8b\n"
    "usubl v19.8h, v19.8b, v6.8b\n"
    "usubl v20.8h, v20.8b, v6.8b\n"
    "usubl v21.8h, v21.8b, v6.8b\n"
    "usubl v22.8h, v22.8b, v6.8b\n"
    "bgt 3b\n"
    "4:"  // Channel loop: Planar tail
    "smlal v23.4s, v14.4h, v0.4h\n"
    "smlal v24.4s, v15.4h, v0.4h\n"
    "smlal v25.4s, v16.4h, v0.4h\n"
    "smlal v26.4s, v17.4h, v0.4h\n"
    "smlal v27.4s, v18.4h, v0.4h\n"
    "smlal v28.4s, v19.4h, v0.4h\n"
    "smlal v29.4s, v20.4h, v0.4h\n"
    "smlal v30.4s, v21.4h, v0.4h\n"
    "smlal v31.4s, v22.4h, v0.4h\n"
    "cbz %x[rq_mul_ptr], 5f\n"
    "lsl x20, x11, #0x2\n"
    "ldr q2, [%x[rq_mul_ptr], x20]\n"
    "ldr q1, [%x[rq_right_shift_ptr], x20]\n"
    "cbz %x[rq_left_shift_ptr], 5f\n"
    "ldr q3, [%x[rq_left_shift_ptr], x20]\n"
    "5:"  // Channel loop: Load quantisation parameters: Done
    "sshl v23.4s, v23.4s, v3.4s\n"
    "sshl v24.4s, v24.4s, v3.4s\n"
    "ldp x28, x27, [%x[outptrs], #0x0]\n"
    "ldp x26, x25, [%x[outptrs], #0x10]\n"
    "sshl v25.4s, v25.4s, v3.4s\n"
    "sqrdmulh v23.4s, v23.4s, v2.4s\n"
    "ldp x24, x23, [%x[outptrs], #0x20]\n"
    "ldp x22, x21, [%x[outptrs], #0x30]\n"
    "sqrdmulh v24.4s, v24.4s, v2.4s\n"
    "sqrdmulh v25.4s, v25.4s, v2.4s\n"
    "ldr x20, [%x[outptrs], #0x40]\n"
    "and v21.16b, v23.16b, v1.16b\n"
    "and v20.16b, v24.16b, v1.16b\n"
    "and v19.16b, v25.16b, v1.16b\n"
    "sshl v26.4s, v26.4s, v3.4s\n"
    "sshl v27.4s, v27.4s, v3.4s\n"
    "sshl v28.4s, v28.4s, v3.4s\n"
    "sshl v29.4s, v29.4s, v3.4s\n"
    "sshl v30.4s, v30.4s, v3.4s\n"
    "sshl v31.4s, v31.4s, v3.4s\n"
    "sshr v21.4s, v21.4s, #0x1f\n"
    "sshr v20.4s, v20.4s, #0x1f\n"
    "sshr v19.4s, v19.4s, #0x1f\n"
    "sqrdmulh v26.4s, v26.4s, v2.4s\n"
    "sqrdmulh v27.4s, v27.4s, v2.4s\n"
    "sqrdmulh v28.4s, v28.4s, v2.4s\n"
    "sqrdmulh v29.4s, v29.4s, v2.4s\n"
    "sqrdmulh v30.4s, v30.4s, v2.4s\n"
    "sqrdmulh v31.4s, v31.4s, v2.4s\n"
    "sqadd v23.4s, v23.4s, v21.4s\n"
    "sqadd v24.4s, v24.4s, v20.4s\n"
    "sqadd v25.4s, v25.4s, v19.4s\n"
    "and v18.16b, v26.16b, v1.16b\n"
    "and v17.16b, v27.16b, v1.16b\n"
    "and v16.16b, v28.16b, v1.16b\n"
    "and v21.16b, v29.16b, v1.16b\n"
    "and v20.16b, v30.16b, v1.16b\n"
    "and v19.16b, v31.16b, v1.16b\n"
    "sshr v18.4s, v18.4s, #0x1f\n"
    "sshr v17.4s, v17.4s, #0x1f\n"
    "sshr v16.4s, v16.4s, #0x1f\n"
    "sshr v21.4s, v21.4s, #0x1f\n"
    "sshr v20.4s, v20.4s, #0x1f\n"
    "sshr v19.4s, v19.4s, #0x1f\n"
    "sqadd v26.4s, v26.4s, v18.4s\n"
    "sqadd v27.4s, v27.4s, v17.4s\n"
    "sqadd v28.4s, v28.4s, v16.4s\n"
    "sqadd v29.4s, v29.4s, v21.4s\n"
    "sqadd v30.4s, v30.4s, v20.4s\n"
    "sqadd v31.4s, v31.4s, v19.4s\n"
    "srshl v23.4s, v23.4s, v1.4s\n"
    "srshl v24.4s, v24.4s, v1.4s\n"
    "srshl v25.4s, v25.4s, v1.4s\n"
    "srshl v26.4s, v26.4s, v1.4s\n"
    "srshl v27.4s, v27.4s, v1.4s\n"
    "srshl v28.4s, v28.4s, v1.4s\n"
    "srshl v29.4s, v29.4s, v1.4s\n"
    "srshl v30.4s, v30.4s, v1.4s\n"
    "srshl v31.4s, v31.4s, v1.4s\n"
    "add v23.4s, v23.4s, v4.4s\n"
    "add v24.4s, v24.4s, v4.4s\n"
    "add v25.4s, v25.4s, v4.4s\n"
    "add v26.4s, v26.4s, v4.4s\n"
    "add v27.4s, v27.4s, v4.4s\n"
    "add v28.4s, v28.4s, v4.4s\n"
    "add v29.4s, v29.4s, v4.4s\n"
    "add v30.4s, v30.4s, v4.4s\n"
    "add v31.4s, v31.4s, v4.4s\n"
    "smax v23.4s, v23.4s, v8.4s\n"
    "smax v24.4s, v24.4s, v8.4s\n"
    "smax v25.4s, v25.4s, v8.4s\n"
    "smax v26.4s, v26.4s, v8.4s\n"
    "smax v27.4s, v27.4s, v8.4s\n"
    "smax v28.4s, v28.4s, v8.4s\n"
    "smax v29.4s, v29.4s, v8.4s\n"
    "smax v30.4s, v30.4s, v8.4s\n"
    "smax v31.4s, v31.4s, v8.4s\n"
    "smin v23.4s, v23.4s, v7.4s\n"
    "smin v24.4s, v24.4s, v7.4s\n"
    "smin v25.4s, v25.4s, v7.4s\n"
    "smin v26.4s, v26.4s, v7.4s\n"
    "smin v27.4s, v27.4s, v7.4s\n"
    "smin v28.4s, v28.4s, v7.4s\n"
    "smin v29.4s, v29.4s, v7.4s\n"
    "smin v30.4s, v30.4s, v7.4s\n"
    "smin v31.4s, v31.4s, v7.4s\n"
    "uzp1 v23.16b, v23.16b, v23.16b\n"
    "uzp1 v24.16b, v24.16b, v24.16b\n"
    "uzp1 v25.16b, v25.16b, v25.16b\n"
    "uzp1 v26.16b, v26.16b, v26.16b\n"
    "uzp1 v27.16b, v27.16b, v27.16b\n"
    "uzp1 v28.16b, v28.16b, v28.16b\n"
    "uzp1 v29.16b, v29.16b, v29.16b\n"
    "uzp1 v30.16b, v30.16b, v30.16b\n"
    "uzp1 v31.16b, v31.16b, v31.16b\n"
    "uzp1 v23.16b, v23.16b, v23.16b\n"
    "uzp1 v24.16b, v24.16b, v24.16b\n"
    "str s23, [x28, x11]\n"
    "uzp1 v25.16b, v25.16b, v25.16b\n"
    "uzp1 v26.16b, v26.16b, v26.16b\n"
    "str s24, [x27, x11]\n"
    "uzp1 v27.16b, v27.16b, v27.16b\n"
    "uzp1 v28.16b, v28.16b, v28.16b\n"
    "str s25, [x26, x11]\n"
    "uzp1 v29.16b, v29.16b, v29.16b\n"
    "uzp1 v30.16b, v30.16b, v30.16b\n"
    "str s26, [x25, x11]\n"
    "uzp1 v31.16b, v31.16b, v31.16b\n"
    "str s27, [x24, x11]\n"
    "str s28, [x23, x11]\n"
    "str s29, [x22, x11]\n"
    "str s30, [x21, x11]\n"
    "str s31, [x20, x11]\n"
    "add x11, x11, #0x4\n"
    "cmp x11, x12, LSL #2\n"
    "blt 1b\n"
    "6:"  // Oddments
    "tst %x[n_channels], #0x3\n"
    "beq 24f\n"
    "movi v23.4s, #0x0\n"
    "cbz %x[bias], 9f\n"
    "add x20, %x[bias], x11, LSL #2\n"
    "tbz %x[n_channels], #1, 7f\n"
    "ld1 { v23.d }[0], [x20], #0x8\n"
    "tbz %x[n_channels], #0, 8f\n"
    "ld1 { v23.s }[2], [x20], #0x4\n"
    "b 8f\n"
    "7:"  // Oddments: Load bias: Bit 1: Unset
    "ld1 { v23.s }[0], [x20], #0x4\n"
    "8:"  // Oddments: Load bias: Bit 1: End
    "9:"  // Oddments: Load bias: Done
    "ldr s0, [%x[params]], #0x4\n"
    "mov x21, %x[inptrs]\n"
    "ldp x10, x9, [x21], #0x10\n"
    "mov v24.16b, v23.16b\n"
    "ldp x28, x27, [x21], #0x10\n"
    "ldp x26, x25, [x21], #0x10\n"
    "mov v25.16b, v23.16b\n"
    "mov v26.16b, v23.16b\n"
    "ldp x24, x23, [x21], #0x10\n"
    "ldr x22, [x21], #0x8\n"
    "mov v27.16b, v23.16b\n"
    "mov v28.16b, v23.16b\n"
    "mov v29.16b, v23.16b\n"
    "mov v30.16b, v23.16b\n"
    "add x10, x10, x11\n"
    "add x9, x9, x11\n"
    "mov v31.16b, v23.16b\n"
    "usubl v0.8h, v0.8b, v5.8b\n"
    "add x28, x28, x11\n"
    "add x27, x27, x11\n"
    "add x26, x26, x11\n"
    "add x25, x25, x11\n"
    "add x24, x24, x11\n"
    "add x23, x23, x11\n"
    "add x22, x22, x11\n"
    "tbz %x[n_channels], #1, 10f\n"
    "ldr h14, [x10], #0x2\n"
    "ldr h15, [x9], #0x2\n"
    "ldr h16, [x28], #0x2\n"
    "ldr h17, [x27], #0x2\n"
    "ldr h18, [x26], #0x2\n"
    "ldr h19, [x25], #0x2\n"
    "ldr h20, [x24], #0x2\n"
    "ldr h21, [x23], #0x2\n"
    "ldr h22, [x22], #0x2\n"
    "tbz %x[n_channels], #0, 11f\n"
    "ld1 { v14.b }[2], [x10], #0x1\n"
    "ld1 { v15.b }[2], [x9], #0x1\n"
    "ld1 { v16.b }[2], [x28], #0x1\n"
    "ld1 { v17.b }[2], [x27], #0x1\n"
    "ld1 { v18.b }[2], [x26], #0x1\n"
    "ld1 { v19.b }[2], [x25], #0x1\n"
    "ld1 { v20.b }[2], [x24], #0x1\n"
    "ld1 { v21.b }[2], [x23], #0x1\n"
    "ld1 { v22.b }[2], [x22], #0x1\n"
    "b 11f\n"
    "10:"  // Oddments: Load: Bit 1: Unset
    "ldr b14, [x10], #0x1\n"
    "ldr b15, [x9], #0x1\n"
    "ldr b16, [x28], #0x1\n"
    "ldr b17, [x27], #0x1\n"
    "ldr b18, [x26], #0x1\n"
    "ldr b19, [x25], #0x1\n"
    "ldr b20, [x24], #0x1\n"
    "ldr b21, [x23], #0x1\n"
    "ldr b22, [x22], #0x1\n"
    "11:"  // Oddments: Load: Bit 1: End
    "subs x20, %x[n_points], #0x1\n"
    "usubl v14.8h, v14.8b, v6.8b\n"
    "usubl v15.8h, v15.8b, v6.8b\n"
    "usubl v16.8h, v16.8b, v6.8b\n"
    "usubl v17.8h, v17.8b, v6.8b\n"
    "usubl v18.8h, v18.8b, v6.8b\n"
    "usubl v19.8h, v19.8b, v6.8b\n"
    "usubl v20.8h, v20.8b, v6.8b\n"
    "usubl v21.8h, v21.8b, v6.8b\n"
    "usubl v22.8h, v22.8b, v6.8b\n"
    "ble 15f\n"
    "12:"  // Oddments: Planar loop
    "ldp x10, x9, [x21], #0x10\n"
    "ldp x28, x27, [x21], #0x10\n"
    "smlal v23.4s, v14.4h, v0.4h\n"
    "smlal v24.4s, v15.4h, v0.4h\n"
    "ldp x26, x25, [x21], #0x10\n"
    "ldp x24, x23, [x21], #0x10\n"
    "smlal v25.4s, v16.4h, v0.4h\n"
    "smlal v26.4s, v17.4h, v0.4h\n"
    "smlal v27.4s, v18.4h, v0.4h\n"
    "smlal v28.4s, v19.4h, v0.4h\n"
    "ldr x22, [x21], #0x8\n"
    "add x10, x10, x11\n"
    "smlal v29.4s, v20.4h, v0.4h\n"
    "smlal v30.4s, v21.4h, v0.4h\n"
    "add x9, x9, x11\n"
    "add x28, x28, x11\n"
    "smlal v31.4s, v22.4h, v0.4h\n"
    "ldr s0, [%x[params]], #0x4\n"
    "usubl v0.8h, v0.8b, v5.8b\n"
    "add x27, x27, x11\n"
    "add x26, x26, x11\n"
    "add x25, x25, x11\n"
    "add x24, x24, x11\n"
    "add x23, x23, x11\n"
    "add x22, x22, x11\n"
    "tbz %x[n_channels], #1, 13f\n"
    "ldr h14, [x10], #0x2\n"
    "ldr h15, [x9], #0x2\n"
    "ldr h16, [x28], #0x2\n"
    "ldr h17, [x27], #0x2\n"
    "ldr h18, [x26], #0x2\n"
    "ldr h19, [x25], #0x2\n"
    "ldr h20, [x24], #0x2\n"
    "ldr h21, [x23], #0x2\n"
    "ldr h22, [x22], #0x2\n"
    "tbz %x[n_channels], #0, 14f\n"
    "ld1 { v14.b }[2], [x10], #0x1\n"
    "ld1 { v15.b }[2], [x9], #0x1\n"
    "ld1 { v16.b }[2], [x28], #0x1\n"
    "ld1 { v17.b }[2], [x27], #0x1\n"
    "ld1 { v18.b }[2], [x26], #0x1\n"
    "ld1 { v19.b }[2], [x25], #0x1\n"
    "ld1 { v20.b }[2], [x24], #0x1\n"
    "ld1 { v21.b }[2], [x23], #0x1\n"
    "ld1 { v22.b }[2], [x22], #0x1\n"
    "b 14f\n"
    "13:"  // Oddments: Planar loop: Load: Bit 1: Unset
    "ldr b14, [x10], #0x1\n"
    "ldr b15, [x9], #0x1\n"
    "ldr b16, [x28], #0x1\n"
    "ldr b17, [x27], #0x1\n"
    "ldr b18, [x26], #0x1\n"
    "ldr b19, [x25], #0x1\n"
    "ldr b20, [x24], #0x1\n"
    "ldr b21, [x23], #0x1\n"
    "ldr b22, [x22], #0x1\n"
    "14:"  // Oddments: Planar loop: Load: Bit 1: End
    "subs x20, x20, #0x1\n"
    "usubl v14.8h, v14.8b, v6.8b\n"
    "usubl v15.8h, v15.8b, v6.8b\n"
    "usubl v16.8h, v16.8b, v6.8b\n"
    "usubl v17.8h, v17.8b, v6.8b\n"
    "usubl v18.8h, v18.8b, v6.8b\n"
    "usubl v19.8h, v19.8b, v6.8b\n"
    "usubl v20.8h, v20.8b, v6.8b\n"
    "usubl v21.8h, v21.8b, v6.8b\n"
    "usubl v22.8h, v22.8b, v6.8b\n"
    "bgt 12b\n"
    "15:"  // Oddments: Planar tail
    "smlal v23.4s, v14.4h, v0.4h\n"
    "smlal v24.4s, v15.4h, v0.4h\n"
    "smlal v25.4s, v16.4h, v0.4h\n"
    "smlal v26.4s, v17.4h, v0.4h\n"
    "smlal v27.4s, v18.4h, v0.4h\n"
    "smlal v28.4s, v19.4h, v0.4h\n"
    "smlal v29.4s, v20.4h, v0.4h\n"
    "smlal v30.4s, v21.4h, v0.4h\n"
    "smlal v31.4s, v22.4h, v0.4h\n"
    "cbz %x[rq_mul_ptr], 21f\n"
    "add x22, %x[rq_mul_ptr], x11, LSL #2\n"
    "add x21, %x[rq_right_shift_ptr], x11, LSL #2\n"
    "add x20, %x[rq_left_shift_ptr], x11, LSL #2\n"
    "tbz %x[n_channels], #1, 18f\n"
    "ld1 { v2.d }[0], [x22], #0x8\n"
    "ld1 { v1.d }[0], [x21], #0x8\n"
    "cbz %x[rq_left_shift_ptr], 16f\n"
    "ld1 { v3.d }[0], [x20], #0x8\n"
    "16:"  // Oddments: Load quantisation parameters: Bit 1: Load left shift: Done
    "tbz %x[n_channels], #0, 20f\n"
    "ld1 { v2.s }[2], [x22], #0x4\n"
    "ld1 { v1.s }[2], [x21], #0x4\n"
    "cbz %x[rq_left_shift_ptr], 17f\n"
    "ld1 { v3.s }[2], [x20], #0x4\n"
    "17:"  // Oddments: Load quantisation parameters: Bit 1: Bit 0: Load left shift: Done
    "b 20f\n"
    "18:"  // Oddments: Load quantisation parameters: Bit 1: Unset
    "ld1 { v2.s }[0], [x22], #0x4\n"
    "ld1 { v1.s }[0], [x21], #0x4\n"
    "cbz %x[rq_left_shift_ptr], 19f\n"
    "ld1 { v3.s }[0], [x20], #0x4\n"
    "19:"  // Oddments: Load quantisation parameters: Bit 1: Unset: Bit 0: Load left shift: Done

    "20:"  // Oddments: Load quantisation parameters: Bit 1: End

    "21:"  // Oddments: Load quantisation parameters: Done
    "sshl v23.4s, v23.4s, v3.4s\n"
    "sshl v24.4s, v24.4s, v3.4s\n"
    "ldp x28, x27, [%x[outptrs], #0x0]\n"
    "ldp x26, x25, [%x[outptrs], #0x10]\n"
    "sshl v25.4s, v25.4s, v3.4s\n"
    "sqrdmulh v23.4s, v23.4s, v2.4s\n"
    "ldp x24, x23, [%x[outptrs], #0x20]\n"
    "ldp x22, x21, [%x[outptrs], #0x30]\n"
    "sqrdmulh v24.4s, v24.4s, v2.4s\n"
    "sqrdmulh v25.4s, v25.4s, v2.4s\n"
    "ldr x20, [%x[outptrs], #0x40]\n"
    "add x28, x28, x11\n"
    "and v21.16b, v23.16b, v1.16b\n"
    "and v20.16b, v24.16b, v1.16b\n"
    "add x27, x27, x11\n"
    "add x26, x26, x11\n"
    "and v19.16b, v25.16b, v1.16b\n"
    "sshl v26.4s, v26.4s, v3.4s\n"
    "add x25, x25, x11\n"
    "add x24, x24, x11\n"
    "sshl v27.4s, v27.4s, v3.4s\n"
    "sshl v28.4s, v28.4s, v3.4s\n"
    "add x23, x23, x11\n"
    "add x22, x22, x11\n"
    "sshl v29.4s, v29.4s, v3.4s\n"
    "sshl v30.4s, v30.4s, v3.4s\n"
    "add x21, x21, x11\n"
    "add x20, x20, x11\n"
    "sshl v31.4s, v31.4s, v3.4s\n"
    "sshr v21.4s, v21.4s, #0x1f\n"
    "sshr v20.4s, v20.4s, #0x1f\n"
    "sshr v19.4s, v19.4s, #0x1f\n"
    "sqrdmulh v26.4s, v26.4s, v2.4s\n"
    "sqrdmulh v27.4s, v27.4s, v2.4s\n"
    "sqrdmulh v28.4s, v28.4s, v2.4s\n"
    "sqrdmulh v29.4s, v29.4s, v2.4s\n"
    "sqrdmulh v30.4s, v30.4s, v2.4s\n"
    "sqrdmulh v31.4s, v31.4s, v2.4s\n"
    "sqadd v23.4s, v23.4s, v21.4s\n"
    "sqadd v24.4s, v24.4s, v20.4s\n"
    "sqadd v25.4s, v25.4s, v19.4s\n"
    "and v18.16b, v26.16b, v1.16b\n"
    "and v17.16b, v27.16b, v1.16b\n"
    "and v16.16b, v28.16b, v1.16b\n"
    "and v21.16b, v29.16b, v1.16b\n"
    "and v20.16b, v30.16b, v1.16b\n"
    "and v19.16b, v31.16b, v1.16b\n"
    "sshr v18.4s, v18.4s, #0x1f\n"
    "sshr v17.4s, v17.4s, #0x1f\n"
    "sshr v16.4s, v16.4s, #0x1f\n"
    "sshr v21.4s, v21.4s, #0x1f\n"
    "sshr v20.4s, v20.4s, #0x1f\n"
    "sshr v19.4s, v19.4s, #0x1f\n"
    "sqadd v26.4s, v26.4s, v18.4s\n"
    "sqadd v27.4s, v27.4s, v17.4s\n"
    "sqadd v28.4s, v28.4s, v16.4s\n"
    "sqadd v29.4s, v29.4s, v21.4s\n"
    "sqadd v30.4s, v30.4s, v20.4s\n"
    "sqadd v31.4s, v31.4s, v19.4s\n"
    "srshl v23.4s, v23.4s, v1.4s\n"
    "srshl v24.4s, v24.4s, v1.4s\n"
    "srshl v25.4s, v25.4s, v1.4s\n"
    "srshl v26.4s, v26.4s, v1.4s\n"
    "srshl v27.4s, v27.4s, v1.4s\n"
    "srshl v28.4s, v28.4s, v1.4s\n"
    "srshl v29.4s, v29.4s, v1.4s\n"
    "srshl v30.4s, v30.4s, v1.4s\n"
    "srshl v31.4s, v31.4s, v1.4s\n"
    "add v23.4s, v23.4s, v4.4s\n"
    "add v24.4s, v24.4s, v4.4s\n"
    "add v25.4s, v25.4s, v4.4s\n"
    "add v26.4s, v26.4s, v4.4s\n"
    "add v27.4s, v27.4s, v4.4s\n"
    "add v28.4s, v28.4s, v4.4s\n"
    "add v29.4s, v29.4s, v4.4s\n"
    "add v30.4s, v30.4s, v4.4s\n"
    "add v31.4s, v31.4s, v4.4s\n"
    "smax v23.4s, v23.4s, v8.4s\n"
    "smax v24.4s, v24.4s, v8.4s\n"
    "smax v25.4s, v25.4s, v8.4s\n"
    "smax v26.4s, v26.4s, v8.4s\n"
    "smax v27.4s, v27.4s, v8.4s\n"
    "smax v28.4s, v28.4s, v8.4s\n"
    "smax v29.4s, v29.4s, v8.4s\n"
    "smax v30.4s, v30.4s, v8.4s\n"
    "smax v31.4s, v31.4s, v8.4s\n"
    "smin v23.4s, v23.4s, v7.4s\n"
    "smin v24.4s, v24.4s, v7.4s\n"
    "smin v25.4s, v25.4s, v7.4s\n"
    "smin v26.4s, v26.4s, v7.4s\n"
    "smin v27.4s, v27.4s, v7.4s\n"
    "smin v28.4s, v28.4s, v7.4s\n"
    "smin v29.4s, v29.4s, v7.4s\n"
    "smin v30.4s, v30.4s, v7.4s\n"
    "smin v31.4s, v31.4s, v7.4s\n"
    "uzp1 v23.16b, v23.16b, v23.16b\n"
    "uzp1 v24.16b, v24.16b, v24.16b\n"
    "uzp1 v25.16b, v25.16b, v25.16b\n"
    "uzp1 v26.16b, v26.16b, v26.16b\n"
    "uzp1 v27.16b, v27.16b, v27.16b\n"
    "uzp1 v28.16b, v28.16b, v28.16b\n"
    "uzp1 v29.16b, v29.16b, v29.16b\n"
    "uzp1 v30.16b, v30.16b, v30.16b\n"
    "uzp1 v31.16b, v31.16b, v31.16b\n"
    "uzp1 v23.16b, v23.16b, v23.16b\n"
    "uzp1 v24.16b, v24.16b, v24.16b\n"
    "uzp1 v25.16b, v25.16b, v25.16b\n"
    "uzp1 v26.16b, v26.16b, v26.16b\n"
    "uzp1 v27.16b, v27.16b, v27.16b\n"
    "uzp1 v28.16b, v28.16b, v28.16b\n"
    "uzp1 v29.16b, v29.16b, v29.16b\n"
    "uzp1 v30.16b, v30.16b, v30.16b\n"
    "uzp1 v31.16b, v31.16b, v31.16b\n"
    "tbz %x[n_channels], #1, 22f\n"
    "st1 { v23.h }[0], [x28], #0x2\n"
    "st1 { v24.h }[0], [x27], #0x2\n"
    "st1 { v25.h }[0], [x26], #0x2\n"
    "st1 { v26.h }[0], [x25], #0x2\n"
    "st1 { v27.h }[0], [x24], #0x2\n"
    "st1 { v28.h }[0], [x23], #0x2\n"
    "st1 { v29.h }[0], [x22], #0x2\n"
    "st1 { v30.h }[0], [x21], #0x2\n"
    "st1 { v31.h }[0], [x20], #0x2\n"
    "tbz %x[n_channels], #0, 23f\n"
    "st1 { v23.b }[2], [x28], #0x1\n"
    "st1 { v24.b }[2], [x27], #0x1\n"
    "st1 { v25.b }[2], [x26], #0x1\n"
    "st1 { v26.b }[2], [x25], #0x1\n"
    "st1 { v27.b }[2], [x24], #0x1\n"
    "st1 { v28.b }[2], [x23], #0x1\n"
    "st1 { v29.b }[2], [x22], #0x1\n"
    "st1 { v30.b }[2], [x21], #0x1\n"
    "st1 { v31.b }[2], [x20], #0x1\n"
    "b 23f\n"
    "22:"  // Oddments: Store: Bit 1: Unset
    "st1 { v23.b }[0], [x28], #0x1\n"
    "st1 { v24.b }[0], [x27], #0x1\n"
    "st1 { v25.b }[0], [x26], #0x1\n"
    "st1 { v26.b }[0], [x25], #0x1\n"
    "st1 { v27.b }[0], [x24], #0x1\n"
    "st1 { v28.b }[0], [x23], #0x1\n"
    "st1 { v29.b }[0], [x22], #0x1\n"
    "st1 { v30.b }[0], [x21], #0x1\n"
    "st1 { v31.b }[0], [x20], #0x1\n"
    "23:"  // Oddments: Store: Bit 1: End

    "24:"  // End

    : [params] "+&r" (params)
    : [bias] "r" (qp.bias), [inptrs] "r" (inptrs), [n_channels] "r" ((uint64_t) n_channels), [n_points] "r" ((uint64_t) n_points), [offsetof_Requantize32_a_offset] "I" (offsetof(arm_gemm::Requantize32, a_offset)), [offsetof_Requantize32_b_offset] "I" (offsetof(arm_gemm::Requantize32, b_offset)), [offsetof_Requantize32_c_offset] "I" (offsetof(arm_gemm::Requantize32, c_offset)), [offsetof_Requantize32_maxval] "I" (offsetof(arm_gemm::Requantize32, maxval)), [offsetof_Requantize32_minval] "I" (offsetof(arm_gemm::Requantize32, minval)), [offsetof_Requantize32_per_layer_left_shift] "I" (offsetof(arm_gemm::Requantize32, per_layer_left_shift)), [offsetof_Requantize32_per_layer_mul] "I" (offsetof(arm_gemm::Requantize32, per_layer_mul)), [offsetof_Requantize32_per_layer_right_shift] "I" (offsetof(arm_gemm::Requantize32, per_layer_right_shift)), [outptrs] "r" (outptrs), [qp] "r" (&qp), [rq_left_shift_ptr] "r" (qp.per_channel_left_shifts), [rq_mul_ptr] "r" (qp.per_channel_muls), [rq_right_shift_ptr] "r" (qp.per_channel_right_shifts)
    : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31", "x9", "x10", "x11", "x12", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28"
  );
}

}  // namespace depthwise
}  // namespace arm_conv
#endif // defined(__aarch64__)
