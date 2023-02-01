/*
 * Copyright (c) 2022-2023 Arm Limited.
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

#if defined(ARM_COMPUTE_ENABLE_SME2)

#include <algorithm>
#include <cstddef>
#include "arm_gemm.hpp"

using arm_gemm::Requantize32;

namespace arm_conv {
namespace depthwise {

void sme2_u8q_planar_3x3_s2_4rows_dot_za_impl(
  const uint8_t *inptr,
  size_t ld_in_row,
  size_t ld_in_col,
  size_t ld_in_vl,
  unsigned int pad_top,
  unsigned int valid_input_rows,
  unsigned int pad_left,
  unsigned int valid_input_cols,
  const uint8_t *weights,
  uint8_t **outptrs,
  const size_t *outlds,
  const size_t *outvllds,
  unsigned int output_cols,
  unsigned int start_channel,
  unsigned int valid_channels,
  const arm_gemm::Requantize32 &qp
)
{
  struct Args
  {
    const uint8_t *inptr;
    size_t ld_in_vl;
    long unsigned int pad_top, pad_bottom, pad_left;
    const uint8_t *weights;
    long unsigned int input_cols, output_cols;
    uint8_t **outptrs;
    const size_t *ld_out_cols;
    const size_t *ld_out_vls;
    long unsigned int current_channel, n_channels;
  };

  Args args = { inptr, ld_in_vl, pad_top, 9u - std::min(9u, pad_top + valid_input_rows), pad_left, weights, valid_input_cols, output_cols, outptrs, outlds, outvllds, start_channel, valid_channels };

  __asm__ __volatile__(
    ".inst 0xd503477f  // SMSTART ZA\n"
    "ldr x6, [%x[args], %[offsetof_Args_pad_bottom]]\n"
    "ptrue p2.b\n"
    "mov x20, #0x9\n"
    "ldr x7, [%x[args], %[offsetof_Args_pad_top]]\n"
    "ld1rh { z5.h }, p2/Z, [%x[qp], %[offsetof_Requantize32_a_offset]]\n"
    "sub x20, x20, x6\n"
    ".inst 0x25207812  // ptrue pn10.b\n"
    "ldr x17, [%x[args], %[offsetof_Args_n_channels]]\n"
    "whilelt p1.s, XZR, x17\n"
    "whilelt p9.s, XZR, x20\n"
    "ld1rw { z4.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_c_offset]]\n"
    "whilelt p8.s, XZR, x7\n"
    "addvl SP, SP, #-6\n"
    "ldr x16, [%x[args], %[offsetof_Args_current_channel]]\n"
    "neg z5.h, p2/M, z5.h\n"
    "eor p8.b, p2/Z, p8.b, p9.b\n"
    "ld1rw { z8.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_per_layer_mul]]\n"
    "ld1rw { z7.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_per_layer_right_shift]]\n"
    "ld1rw { z27.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_minval]]\n"
    "ld1rw { z23.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_maxval]]\n"
    "1:"  // Channel loop
    "ldr x20, [%x[qp], %[offsetof_Requantize32_bias]]\n"
    "mov z0.s, #0x0\n"
    "cbz x20, 2f\n"
    "ld1w { z0.s }, p1/Z, [x20, x16, LSL #2]\n"
    "2:"  // Load bias: Done
    "ldr x22, [%x[args], %[offsetof_Args_weights]]\n"
    "mov x20, x22\n"
    "ld1b { z24.s }, p2/Z, [x20]\n"
    "incw x20, ALL, MUL #3\n"
    "ld1rh { z13.h }, p2/Z, [%x[qp], %[offsetof_Requantize32_b_offset]]\n"
    "sub z24.h, z24.h, z13.h\n"
    "incw x22\n"
    "mov z17.h, #0x0\n"
    "ld1b { z25.s }, p2/Z, [x20]\n"
    "incw x20, ALL, MUL #3\n"
    "sub z25.h, z25.h, z13.h\n"
    "trn1 z10.h, z24.h, z25.h\n"
    "ld1b { z16.s }, p2/Z, [x20]\n"
    "sub z16.h, z16.h, z13.h\n"
    "mov x20, x22\n"
    "trn1 z11.h, z16.h, z17.h\n"
    "ld1b { z24.s }, p2/Z, [x20]\n"
    "incw x20, ALL, MUL #3\n"
    "sub z24.h, z24.h, z13.h\n"
    "addvl x21, SP, #6\n"
    "ld1b { z25.s }, p2/Z, [x20]\n"
    "incw x20, ALL, MUL #3\n"
    "sub z25.h, z25.h, z13.h\n"
    "incw x22\n"
    "ld1b { z16.s }, p2/Z, [x20]\n"
    "sub z16.h, z16.h, z13.h\n"
    "addvl x21, x21, #-2\n"
    "mov x20, x22\n"
    "st1h { z10.h }, p2, [x21]\n"
    "trn1 z10.h, z24.h, z25.h\n"
    "ld1b { z24.s }, p2/Z, [x20]\n"
    "incw x20, ALL, MUL #3\n"
    "ld1b { z25.s }, p2/Z, [x20]\n"
    "incw x20, ALL, MUL #3\n"
    "st1h { z11.h }, p2, [x21, #1, MUL VL]\n"
    "trn1 z11.h, z16.h, z17.h\n"
    "ld1b { z16.s }, p2/Z, [x20]\n"
    "sub z24.h, z24.h, z13.h\n"
    "sub z25.h, z25.h, z13.h\n"
    "ldr x20, [%x[qp], %[offsetof_Requantize32_per_channel_muls]]\n"
    "sub z16.h, z16.h, z13.h\n"
    "addvl x21, x21, #-2\n"
    "st1h { z10.h }, p2, [x21]\n"
    "mov z1.d, z0.d\n"
    "st1h { z11.h }, p2, [x21, #1, MUL VL]\n"
    "addvl x21, x21, #-2\n"
    "mov z2.d, z0.d\n"
    "mov z3.d, z0.d\n"
    "trn1 z10.h, z24.h, z25.h\n"
    "st1h { z10.h }, p2, [x21]\n"
    "trn1 z11.h, z16.h, z17.h\n"
    "st1h { z11.h }, p2, [x21, #1, MUL VL]\n"
    "cbz x20, 3f\n"
    "ld1w { z8.s }, p1/Z, [x20, x16, LSL #2]\n"
    "3:"  // Load mul: End
    "ldr x20, [%x[qp], %[offsetof_Requantize32_per_channel_right_shifts]]\n"
    "cbz x20, 4f\n"
    "ld1w { z7.s }, p1/Z, [x20, x16, LSL #2]\n"
    "4:"  // Load right_shift: End
    "ldr x15, [%x[args], %[offsetof_Args_input_cols]]\n"
    "sub x20, x15, #0x1\n"
    "orr x23, x20, %x[ld_in_col], LSL #16\n"
    "ldr x14, [%x[args], %[offsetof_Args_inptr]]\n"
    "orr x23, x17, x23, LSL #22\n"
    "mov x22, #0x9\n"
    "add x21, x7, x6\n"
    "lsl x20, %x[ld_in_row], #0x0\n"
    "ldr x13, [%x[args], %[offsetof_Args_output_cols]]\n"
    "mov x8, #0x0\n"
    "lsl x23, x23, #0x0\n"
    "sub x22, x22, x21\n"
    "madd x20, x20, x7, x14\n"
    "5:"  // Issue prefetches
    "subs x22, x22, #0x1\n"
    ".inst 0xf8b74a9c  // rprfm pldstrm, x23, [x20]\n"
    "add x20, x20, %x[ld_in_col]\n"
    "bgt 5b\n"
    "ldr x25, [%x[args], %[offsetof_Args_outptrs]]\n"
    "lsl x20, %x[ld_in_row], #0x0\n"
    "msub x14, x7, x20, x14\n"
    ".inst 0xc0040c00  // mova za.d[x8, #0], { z0.d-z3.d }\n"
    "ldr x20, [%x[args], %[offsetof_Args_ld_out_cols]]\n"
    ".inst 0xc0040c01  // mova za.d[x8, #1], { z0.d-z3.d }\n"
    "mov x22, #0x2\n"
    "ldp x11, x10, [x25], #0x10\n"
    ".inst 0xc0040c02  // mova za.d[x8, #2], { z0.d-z3.d }\n"
    "ldp x9, x28, [x20], #0x10\n"
    "ldr x21, [%x[args], %[offsetof_Args_pad_left]]\n"
    "ldp x27, x26, [x25], #0x10\n"
    "ldp x25, x24, [x20], #0x10\n"
    "cbz x21, 7f\n"
    "cmp x21, x22\n"
    "csel x20, x21, x22, LT\n"
    "sub x21, x21, x20\n"
    "sub x22, x22, x20\n"
    "cbz x21, 7f\n"
    ".inst 0xc0060c1c  // mova { z28.d-z31.d }, za.d[x8, #0]\n"
    ".inst 0xc1a8ac1c  // sqdmulh { z28.s-z31.s }, { z28.s-z31.s }, z8.s\n"
    "and x22, x21, #0x1\n"
    ".inst 0xc1a7aa3c  // srshl { z28.s-z31.s }, { z28.s-z31.s }, z7.s\n"
    "add x21, x21, #0x1\n"
    "lsr x21, x21, #0x1\n"
    ".inst 0xc1a4ab1c  // add { z28.s-z31.s }, { z28.s-z31.s }, z4.s\n"
    "sub x13, x13, x21\n"
    ".inst 0xc1b7cf7c  // sclamp { z28.s-z31.s }, z27.s, z23.s\n"
    "6:"  // Left padding
    "subs x21, x21, #0x1\n"
    "st1b { z28.s }, p1, [x11]\n"
    "add x11, x11, x9\n"
    "st1b { z29.s }, p1, [x10]\n"
    "add x10, x10, x28\n"
    "st1b { z30.s }, p1, [x27]\n"
    "add x27, x27, x25\n"
    "st1b { z31.s }, p1, [x26]\n"
    "add x26, x26, x24\n"
    "bgt 6b\n"
    "7:"  // Left padding: End
    "adds XZR, x7, x6\n"
    "bne 12f\n"
    "cbz x22, 10f\n"
    "cmp x22, #0x1\n"
    "sub x15, x15, x22\n"
    "beq 9f\n"
    "8:"  // Unpadded: 2 priming loads
    "add x21, x14, %x[ld_in_row]\n"
    "ld1b { z12.s }, p1/Z, [x14]\n"
    "addvl x20, SP, #4\n"
    "ld1b { z20.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z12.h, z12.h, z20.h\n"
    "add z12.h, z12.h, z5.h\n"
    "ld1b { z13.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "add x14, x14, %x[ld_in_col]\n"
    "ld1b { z19.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z13.h, z13.h, z19.h\n"
    "add z13.h, z13.h, z5.h\n"
    "ld1b { z14.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "ld1b { z18.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z14.h, z14.h, z18.h\n"
    "add z14.h, z14.h, z5.h\n"
    "ld1b { z15.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "ld1b { z17.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z15.h, z15.h, z17.h\n"
    "add z15.h, z15.h, z5.h\n"
    "ld1b { z16.s }, p1/Z, [x21]\n"
    "mov z16.d, z16.d\n"
    "add z16.h, z16.h, z5.h\n"
    ".inst 0xa0402a8a  // ld1h { z10.h-z11.h }, pn10.b/Z, [x20]\n"
    ".inst 0xc17a1588  // sdot za.s[x8, 0], { z12.h-z15.h }, z10.h\n"
    ".inst 0xc17b15a8  // sdot za.s[x8, 0], { z13.h-z16.h }, z11.h\n"
    "9:"  // Unpadded: 1 priming loads
    "add x21, x14, %x[ld_in_row]\n"
    "ld1b { z12.s }, p1/Z, [x14]\n"
    "addvl x20, SP, #2\n"
    "ld1b { z20.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z12.h, z12.h, z20.h\n"
    "add z12.h, z12.h, z5.h\n"
    "ld1b { z13.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "add x14, x14, %x[ld_in_col]\n"
    "ld1b { z19.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z13.h, z13.h, z19.h\n"
    "add z13.h, z13.h, z5.h\n"
    "ld1b { z14.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "ld1b { z18.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z14.h, z14.h, z18.h\n"
    "add z14.h, z14.h, z5.h\n"
    "ld1b { z15.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "ld1b { z17.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z15.h, z15.h, z17.h\n"
    "add z15.h, z15.h, z5.h\n"
    "ld1b { z16.s }, p1/Z, [x21]\n"
    "mov z16.d, z16.d\n"
    "add z16.h, z16.h, z5.h\n"
    ".inst 0xa0402a8a  // ld1h { z10.h-z11.h }, pn10.b/Z, [x20]\n"
    ".inst 0xc17a1588  // sdot za.s[x8, 0], { z12.h-z15.h }, z10.h\n"
    ".inst 0xc17b15a8  // sdot za.s[x8, 0], { z13.h-z16.h }, z11.h\n"
    "10:"  // Unpadded: 0 priming loads
    "cmp x15, #0x2\n"
    ".inst 0xa0402bea  // ld1h { z10.h-z11.h }, pn10.b/Z, [SP]\n"
    "blt 18f\n"
    "add x21, x14, %x[ld_in_row]\n"
    "ld1b { z12.s }, p1/Z, [x14]\n"
    "sub x15, x15, #0x2\n"
    "ld1b { z20.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z12.h, z12.h, z20.h\n"
    "sub x13, x13, #0x1\n"
    "ld1b { z13.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "lsr x20, x15, #0x1\n"
    "add z12.h, z12.h, z5.h\n"
    "ld1b { z19.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z13.h, z13.h, z19.h\n"
    "cmp x20, x13\n"
    "ld1b { z14.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "csel x23, x20, x13, LT\n"
    "add z13.h, z13.h, z5.h\n"
    "ld1b { z18.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z14.h, z14.h, z18.h\n"
    "add z14.h, z14.h, z5.h\n"
    "ld1b { z15.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "add x14, x14, %x[ld_in_col]\n"
    "ld1b { z17.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z15.h, z15.h, z17.h\n"
    "add z15.h, z15.h, z5.h\n"
    "ld1b { z16.s }, p1/Z, [x21]\n"
    "mov z16.d, z16.d\n"
    "add z16.h, z16.h, z5.h\n"
    "and x15, x15, #0x1\n"
    "sub x13, x13, x23\n"
    "cbz x23, 17f\n"
    "11:"  // Unpadded: Main loop
    ".inst 0xc17a1588  // sdot za.s[x8, 0], { z12.h-z15.h }, z10.h\n"
    "addvl x20, SP, #4\n"
    "add x22, x14, %x[ld_in_row]\n"
    ".inst 0xc17b15a8  // sdot za.s[x8, 0], { z13.h-z16.h }, z11.h\n"
    ".inst 0xa0402a8a  // ld1h { z10.h-z11.h }, pn10.b/Z, [x20]\n"
    "addvl x21, SP, #2\n"
    "subs x23, x23, #0x1\n"
    ".inst 0xc17a1589  // sdot za.s[x8, 1], { z12.h-z15.h }, z10.h\n"
    "ld1b { z12.s }, p1/Z, [x14]\n"
    "add x14, x14, %x[ld_in_col]\n"
    "add x20, x14, %x[ld_in_row]\n"
    "ld1b { z20.s }, p1/Z, [x22]\n"
    "add x22, x22, %x[ld_in_row]\n"
    ".inst 0xc17b15a9  // sdot za.s[x8, 1], { z13.h-z16.h }, z11.h\n"
    "trn1 z12.h, z12.h, z20.h\n"
    "ld1b { z13.s }, p1/Z, [x22]\n"
    "add x22, x22, %x[ld_in_row]\n"
    "add z12.h, z12.h, z5.h\n"
    ".inst 0xc0060c1c  // mova { z28.d-z31.d }, za.d[x8, #0]\n"
    "ld1b { z19.s }, p1/Z, [x22]\n"
    "add x22, x22, %x[ld_in_row]\n"
    "trn1 z13.h, z13.h, z19.h\n"
    "add z13.h, z13.h, z5.h\n"
    "ld1b { z14.s }, p1/Z, [x22]\n"
    "add x22, x22, %x[ld_in_row]\n"
    "add x8, x8, #0x1\n"
    ".inst 0xc0040c02  // mova za.d[x8, #2], { z0.d-z3.d }\n"
    "ld1b { z18.s }, p1/Z, [x22]\n"
    "add x22, x22, %x[ld_in_row]\n"
    "trn1 z14.h, z14.h, z18.h\n"
    "add z14.h, z14.h, z5.h\n"
    "ld1b { z15.s }, p1/Z, [x22]\n"
    "add x22, x22, %x[ld_in_row]\n"
    ".inst 0xc1a8ac1c  // sqdmulh { z28.s-z31.s }, { z28.s-z31.s }, z8.s\n"
    "ld1b { z17.s }, p1/Z, [x22]\n"
    "add x22, x22, %x[ld_in_row]\n"
    "trn1 z15.h, z15.h, z17.h\n"
    "add z15.h, z15.h, z5.h\n"
    "ld1b { z16.s }, p1/Z, [x22]\n"
    "mov z16.d, z16.d\n"
    "add z16.h, z16.h, z5.h\n"
    ".inst 0xa0402aaa  // ld1h { z10.h-z11.h }, pn10.b/Z, [x21]\n"
    ".inst 0xc17a1588  // sdot za.s[x8, 0], { z12.h-z15.h }, z10.h\n"
    ".inst 0xc1a7aa3c  // srshl { z28.s-z31.s }, { z28.s-z31.s }, z7.s\n"
    "ld1b { z12.s }, p1/Z, [x14]\n"
    ".inst 0xc17b15a8  // sdot za.s[x8, 0], { z13.h-z16.h }, z11.h\n"
    ".inst 0xc1a4ab1c  // add { z28.s-z31.s }, { z28.s-z31.s }, z4.s\n"
    "ld1b { z20.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    "trn1 z12.h, z12.h, z20.h\n"
    ".inst 0xc1b7cf7c  // sclamp { z28.s-z31.s }, z27.s, z23.s\n"
    "ld1b { z13.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    "st1b { z28.s }, p1, [x11]\n"
    "add x11, x11, x9\n"
    "ld1b { z19.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    "trn1 z13.h, z13.h, z19.h\n"
    "st1b { z29.s }, p1, [x10]\n"
    "ld1b { z14.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    "add x10, x10, x28\n"
    "st1b { z30.s }, p1, [x27]\n"
    "ld1b { z18.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    "trn1 z14.h, z14.h, z18.h\n"
    "add x27, x27, x25\n"
    "ld1b { z15.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    "st1b { z31.s }, p1, [x26]\n"
    "add x26, x26, x24\n"
    "ld1b { z17.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    "trn1 z15.h, z15.h, z17.h\n"
    "add z12.h, z12.h, z5.h\n"
    "ld1b { z16.s }, p1/Z, [x20]\n"
    "mov z16.d, z16.d\n"
    "add z13.h, z13.h, z5.h\n"
    "add x14, x14, %x[ld_in_col]\n"
    ".inst 0xa0402bea  // ld1h { z10.h-z11.h }, pn10.b/Z, [SP]\n"
    "add z14.h, z14.h, z5.h\n"
    "add z15.h, z15.h, z5.h\n"
    "add z16.h, z16.h, z5.h\n"
    "bgt 11b\n"
    "b 17f\n"
    "12:"  // Padded
    "cbz x22, 15f\n"
    "cmp x22, #0x1\n"
    "sub x15, x15, x22\n"
    "beq 14f\n"
    "13:"  // Padded: 2 priming loads
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z12.s }, p0/Z, [x14]\n"
    "add z12.h, p0/M, z12.h, z5.h\n"
    "add x20, x14, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1b { z20.s }, p0/Z, [x20]\n"
    "add z20.h, p0/M, z20.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1b { z13.s }, p0/Z, [x20]\n"
    "add z13.h, p0/M, z13.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z19.s }, p0/Z, [x20]\n"
    "add z19.h, p0/M, z19.h, z5.h\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row]\n"
    "trn1 z12.h, z12.h, z20.h\n"
    "trn1 z13.h, z13.h, z19.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z14.s }, p0/Z, [x20]\n"
    "add z14.h, p0/M, z14.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1b { z18.s }, p0/Z, [x20]\n"
    "add z18.h, p0/M, z18.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1b { z15.s }, p0/Z, [x20]\n"
    "add z15.h, p0/M, z15.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z17.s }, p0/Z, [x20]\n"
    "mov x12, #0x8\n"
    "add z17.h, p0/M, z17.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z16.s }, p0/Z, [x20]\n"
    "add z16.h, p0/M, z16.h, z5.h\n"
    "addvl x20, SP, #4\n"
    "trn1 z14.h, z14.h, z18.h\n"
    "trn1 z15.h, z15.h, z17.h\n"
    ".inst 0xa0402a8a  // ld1h { z10.h-z11.h }, pn10.b/Z, [x20]\n"
    "mov z16.d, z16.d\n"
    ".inst 0xc17a1588  // sdot za.s[x8, 0], { z12.h-z15.h }, z10.h\n"
    "add x14, x14, %x[ld_in_col]\n"
    ".inst 0xc17b15a8  // sdot za.s[x8, 0], { z13.h-z16.h }, z11.h\n"
    "14:"  // Padded: 1 priming loads
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z12.s }, p0/Z, [x14]\n"
    "add z12.h, p0/M, z12.h, z5.h\n"
    "add x20, x14, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1b { z20.s }, p0/Z, [x20]\n"
    "add z20.h, p0/M, z20.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1b { z13.s }, p0/Z, [x20]\n"
    "add z13.h, p0/M, z13.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z19.s }, p0/Z, [x20]\n"
    "add z19.h, p0/M, z19.h, z5.h\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row]\n"
    "trn1 z12.h, z12.h, z20.h\n"
    "trn1 z13.h, z13.h, z19.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z14.s }, p0/Z, [x20]\n"
    "add z14.h, p0/M, z14.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1b { z18.s }, p0/Z, [x20]\n"
    "add z18.h, p0/M, z18.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1b { z15.s }, p0/Z, [x20]\n"
    "add z15.h, p0/M, z15.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z17.s }, p0/Z, [x20]\n"
    "mov x12, #0x8\n"
    "add z17.h, p0/M, z17.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z16.s }, p0/Z, [x20]\n"
    "add z16.h, p0/M, z16.h, z5.h\n"
    "addvl x20, SP, #2\n"
    "trn1 z14.h, z14.h, z18.h\n"
    "trn1 z15.h, z15.h, z17.h\n"
    ".inst 0xa0402a8a  // ld1h { z10.h-z11.h }, pn10.b/Z, [x20]\n"
    "mov z16.d, z16.d\n"
    ".inst 0xc17a1588  // sdot za.s[x8, 0], { z12.h-z15.h }, z10.h\n"
    "add x14, x14, %x[ld_in_col]\n"
    ".inst 0xc17b15a8  // sdot za.s[x8, 0], { z13.h-z16.h }, z11.h\n"
    "15:"  // Padded: 0 priming loads
    "cmp x15, #0x2\n"
    ".inst 0xa0402bea  // ld1h { z10.h-z11.h }, pn10.b/Z, [SP]\n"
    "blt 18f\n"
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z12.s }, p0/Z, [x14]\n"
    "add z12.h, p0/M, z12.h, z5.h\n"
    "add x20, x14, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1b { z20.s }, p0/Z, [x20]\n"
    "add z20.h, p0/M, z20.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1b { z13.s }, p0/Z, [x20]\n"
    "add z13.h, p0/M, z13.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z19.s }, p0/Z, [x20]\n"
    "add z19.h, p0/M, z19.h, z5.h\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row]\n"
    "trn1 z12.h, z12.h, z20.h\n"
    "trn1 z13.h, z13.h, z19.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z14.s }, p0/Z, [x20]\n"
    "add z14.h, p0/M, z14.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1b { z18.s }, p0/Z, [x20]\n"
    "add z18.h, p0/M, z18.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1b { z15.s }, p0/Z, [x20]\n"
    "add z15.h, p0/M, z15.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z17.s }, p0/Z, [x20]\n"
    "mov x12, #0x8\n"
    "add z17.h, p0/M, z17.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z16.s }, p0/Z, [x20]\n"
    "add z16.h, p0/M, z16.h, z5.h\n"
    "sub x15, x15, #0x2\n"
    "sub x13, x13, #0x1\n"
    "trn1 z14.h, z14.h, z18.h\n"
    "trn1 z15.h, z15.h, z17.h\n"
    "lsr x20, x15, #0x1\n"
    "cmp x20, x13\n"
    "mov z16.d, z16.d\n"
    "csel x22, x20, x13, LT\n"
    "add x14, x14, %x[ld_in_col]\n"
    "and x15, x15, #0x1\n"
    "sub x13, x13, x22\n"
    "cbz x22, 17f\n"
    "16:"  // Padded: Main loop
    ".inst 0xc17a1588  // sdot za.s[x8, 0], { z12.h-z15.h }, z10.h\n"
    "addvl x20, SP, #4\n"
    "mov x12, #0x0\n"
    ".inst 0xc17b15a8  // sdot za.s[x8, 0], { z13.h-z16.h }, z11.h\n"
    ".inst 0xa0402a8a  // ld1h { z10.h-z11.h }, pn10.b/Z, [x20]\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "add x21, x14, %x[ld_in_row]\n"
    ".inst 0xc17a1589  // sdot za.s[x8, 1], { z12.h-z15.h }, z10.h\n"
    "ld1b { z12.s }, p0/Z, [x14]\n"
    "add z12.h, p0/M, z12.h, z5.h\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1b { z20.s }, p0/Z, [x21]\n"
    "add z20.h, p0/M, z20.h, z5.h\n"
    "add x21, x21, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    ".inst 0xc17b15a9  // sdot za.s[x8, 1], { z13.h-z16.h }, z11.h\n"
    "ld1b { z13.s }, p0/Z, [x21]\n"
    "add z13.h, p0/M, z13.h, z5.h\n"
    "add x21, x21, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z19.s }, p0/Z, [x21]\n"
    "mov x12, #0x4\n"
    "add z19.h, p0/M, z19.h, z5.h\n"
    "add x21, x21, %x[ld_in_row]\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z14.s }, p0/Z, [x21]\n"
    "add z14.h, p0/M, z14.h, z5.h\n"
    "add x21, x21, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1b { z18.s }, p0/Z, [x21]\n"
    "add z18.h, p0/M, z18.h, z5.h\n"
    "add x21, x21, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1b { z15.s }, p0/Z, [x21]\n"
    "add z15.h, p0/M, z15.h, z5.h\n"
    "add x21, x21, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z17.s }, p0/Z, [x21]\n"
    "add z17.h, p0/M, z17.h, z5.h\n"
    "mov x12, #0x8\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z12.h, z12.h, z20.h\n"
    "trn1 z13.h, z13.h, z19.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "addvl x20, SP, #2\n"
    "ld1b { z16.s }, p0/Z, [x21]\n"
    "trn1 z14.h, z14.h, z18.h\n"
    "trn1 z15.h, z15.h, z17.h\n"
    ".inst 0xa0402a8a  // ld1h { z10.h-z11.h }, pn10.b/Z, [x20]\n"
    "mov x12, #0x0\n"
    ".inst 0xc0060c1c  // mova { z28.d-z31.d }, za.d[x8, #0]\n"
    "add x8, x8, #0x1\n"
    "add z16.h, p0/M, z16.h, z5.h\n"
    "add x14, x14, %x[ld_in_col]\n"
    ".inst 0xc17a1588  // sdot za.s[x8, 0], { z12.h-z15.h }, z10.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z12.s }, p0/Z, [x14]\n"
    "add z12.h, p0/M, z12.h, z5.h\n"
    "add x20, x14, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "mov z16.d, z16.d\n"
    "ld1b { z20.s }, p0/Z, [x20]\n"
    "add z20.h, p0/M, z20.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    ".inst 0xc17b15a8  // sdot za.s[x8, 0], { z13.h-z16.h }, z11.h\n"
    "ld1b { z13.s }, p0/Z, [x20]\n"
    "add z13.h, p0/M, z13.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z19.s }, p0/Z, [x20]\n"
    "mov x12, #0x4\n"
    "add z19.h, p0/M, z19.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0xc0040c02  // mova za.d[x8, #2], { z0.d-z3.d }\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z14.s }, p0/Z, [x20]\n"
    "add z14.h, p0/M, z14.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1b { z18.s }, p0/Z, [x20]\n"
    "add z18.h, p0/M, z18.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1b { z15.s }, p0/Z, [x20]\n"
    ".inst 0xc1a8ac1c  // sqdmulh { z28.s-z31.s }, { z28.s-z31.s }, z8.s\n"
    "add z15.h, p0/M, z15.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z17.s }, p0/Z, [x20]\n"
    "mov x12, #0x8\n"
    ".inst 0xc1a7aa3c  // srshl { z28.s-z31.s }, { z28.s-z31.s }, z7.s\n"
    "add x20, x20, %x[ld_in_row]\n"
    "add z17.h, p0/M, z17.h, z5.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z16.s }, p0/Z, [x20]\n"
    "add z16.h, p0/M, z16.h, z5.h\n"
    ".inst 0xc1a4ab1c  // add { z28.s-z31.s }, { z28.s-z31.s }, z4.s\n"
    "subs x22, x22, #0x1\n"
    ".inst 0xa0402bea  // ld1h { z10.h-z11.h }, pn10.b/Z, [SP]\n"
    ".inst 0xc1b7cf7c  // sclamp { z28.s-z31.s }, z27.s, z23.s\n"
    "st1b { z28.s }, p1, [x11]\n"
    "add x11, x11, x9\n"
    "trn1 z12.h, z12.h, z20.h\n"
    "st1b { z29.s }, p1, [x10]\n"
    "add x10, x10, x28\n"
    "trn1 z13.h, z13.h, z19.h\n"
    "trn1 z14.h, z14.h, z18.h\n"
    "st1b { z30.s }, p1, [x27]\n"
    "add x27, x27, x25\n"
    "trn1 z15.h, z15.h, z17.h\n"
    "mov z16.d, z16.d\n"
    "st1b { z31.s }, p1, [x26]\n"
    "add x26, x26, x24\n"
    "add x14, x14, %x[ld_in_col]\n"
    "bgt 16b\n"
    "17:"  // Main loop tail
    ".inst 0xc17a1588  // sdot za.s[x8, 0], { z12.h-z15.h }, z10.h\n"
    "addvl x20, SP, #4\n"
    "mov x12, #0x0\n"
    ".inst 0xc17b15a8  // sdot za.s[x8, 0], { z13.h-z16.h }, z11.h\n"
    ".inst 0xa0402a8a  // ld1h { z10.h-z11.h }, pn10.b/Z, [x20]\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "add x20, x14, %x[ld_in_row]\n"
    ".inst 0xc17a1589  // sdot za.s[x8, 1], { z12.h-z15.h }, z10.h\n"
    "ld1b { z12.s }, p0/Z, [x14]\n"
    "add z12.h, p0/M, z12.h, z5.h\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1b { z20.s }, p0/Z, [x20]\n"
    "add z20.h, p0/M, z20.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    ".inst 0xc17b15a9  // sdot za.s[x8, 1], { z13.h-z16.h }, z11.h\n"
    "ld1b { z13.s }, p0/Z, [x20]\n"
    "add z13.h, p0/M, z13.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z19.s }, p0/Z, [x20]\n"
    "mov x12, #0x4\n"
    "add z19.h, p0/M, z19.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z14.s }, p0/Z, [x20]\n"
    "add z14.h, p0/M, z14.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1b { z18.s }, p0/Z, [x20]\n"
    "add z18.h, p0/M, z18.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1b { z15.s }, p0/Z, [x20]\n"
    ".inst 0xc0060c1c  // mova { z28.d-z31.d }, za.d[x8, #0]\n"
    "add z15.h, p0/M, z15.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z17.s }, p0/Z, [x20]\n"
    "mov x12, #0x8\n"
    ".inst 0xc1a8ac1c  // sqdmulh { z28.s-z31.s }, { z28.s-z31.s }, z8.s\n"
    "add x20, x20, %x[ld_in_row]\n"
    "add z17.h, p0/M, z17.h, z5.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z16.s }, p0/Z, [x20]\n"
    "addvl x20, SP, #2\n"
    ".inst 0xc1a7aa3c  // srshl { z28.s-z31.s }, { z28.s-z31.s }, z7.s\n"
    "trn1 z12.h, z12.h, z20.h\n"
    "add x8, x8, #0x1\n"
    "add z16.h, p0/M, z16.h, z5.h\n"
    "trn1 z13.h, z13.h, z19.h\n"
    "trn1 z14.h, z14.h, z18.h\n"
    "add x14, x14, %x[ld_in_col]\n"
    "trn1 z15.h, z15.h, z17.h\n"
    ".inst 0xa0402a8a  // ld1h { z10.h-z11.h }, pn10.b/Z, [x20]\n"
    ".inst 0xc1a4ab1c  // add { z28.s-z31.s }, { z28.s-z31.s }, z4.s\n"
    "mov z16.d, z16.d\n"
    ".inst 0xc17a1588  // sdot za.s[x8, 0], { z12.h-z15.h }, z10.h\n"
    ".inst 0xc1b7cf7c  // sclamp { z28.s-z31.s }, z27.s, z23.s\n"
    "st1b { z28.s }, p1, [x11]\n"
    "add x11, x11, x9\n"
    ".inst 0xc0040c02  // mova za.d[x8, #2], { z0.d-z3.d }\n"
    "st1b { z29.s }, p1, [x10]\n"
    "add x10, x10, x28\n"
    ".inst 0xc17b15a8  // sdot za.s[x8, 0], { z13.h-z16.h }, z11.h\n"
    ".inst 0xa0402bea  // ld1h { z10.h-z11.h }, pn10.b/Z, [SP]\n"
    "st1b { z30.s }, p1, [x27]\n"
    "add x27, x27, x25\n"
    "st1b { z31.s }, p1, [x26]\n"
    "add x26, x26, x24\n"
    "18:"  // Main loop skip tail
    "cbz x15, 19f\n"  // Skip remainder inputs
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z12.s }, p0/Z, [x14]\n"
    "add z12.h, p0/M, z12.h, z5.h\n"
    "add x20, x14, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1b { z20.s }, p0/Z, [x20]\n"
    "add z20.h, p0/M, z20.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1b { z13.s }, p0/Z, [x20]\n"
    "add z13.h, p0/M, z13.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z19.s }, p0/Z, [x20]\n"
    "add z19.h, p0/M, z19.h, z5.h\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row]\n"
    "trn1 z12.h, z12.h, z20.h\n"
    "trn1 z13.h, z13.h, z19.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z14.s }, p0/Z, [x20]\n"
    "add z14.h, p0/M, z14.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1b { z18.s }, p0/Z, [x20]\n"
    "add z18.h, p0/M, z18.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1b { z15.s }, p0/Z, [x20]\n"
    "add z15.h, p0/M, z15.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1b { z17.s }, p0/Z, [x20]\n"
    "mov x12, #0x8\n"
    "add z17.h, p0/M, z17.h, z5.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1b { z16.s }, p0/Z, [x20]\n"
    "add z16.h, p0/M, z16.h, z5.h\n"
    "trn1 z14.h, z14.h, z18.h\n"
    "trn1 z15.h, z15.h, z17.h\n"
    "mov z16.d, z16.d\n"
    "addvl x20, SP, #4\n"
    ".inst 0xc17a1588  // sdot za.s[x8, 0], { z12.h-z15.h }, z10.h\n"
    "sub x13, x13, #0x1\n"
    ".inst 0xc17b15a8  // sdot za.s[x8, 0], { z13.h-z16.h }, z11.h\n"
    ".inst 0xa0402a8a  // ld1h { z10.h-z11.h }, pn10.b/Z, [x20]\n"
    ".inst 0xc0060c1c  // mova { z28.d-z31.d }, za.d[x8, #0]\n"
    ".inst 0xc1a8ac1c  // sqdmulh { z28.s-z31.s }, { z28.s-z31.s }, z8.s\n"
    ".inst 0xc1a7aa3c  // srshl { z28.s-z31.s }, { z28.s-z31.s }, z7.s\n"
    ".inst 0xc17a1589  // sdot za.s[x8, 1], { z12.h-z15.h }, z10.h\n"
    ".inst 0xc1a4ab1c  // add { z28.s-z31.s }, { z28.s-z31.s }, z4.s\n"
    ".inst 0xc17b15a9  // sdot za.s[x8, 1], { z13.h-z16.h }, z11.h\n"
    "add x8, x8, #0x1\n"
    ".inst 0xc1b7cf7c  // sclamp { z28.s-z31.s }, z27.s, z23.s\n"
    "st1b { z28.s }, p1, [x11]\n"
    "add x11, x11, x9\n"
    ".inst 0xc0040c02  // mova za.d[x8, #2], { z0.d-z3.d }\n"
    "st1b { z29.s }, p1, [x10]\n"
    "add x10, x10, x28\n"
    "st1b { z30.s }, p1, [x27]\n"
    "add x27, x27, x25\n"
    "st1b { z31.s }, p1, [x26]\n"
    "add x26, x26, x24\n"
    "19:"  // Tail input: End
    "cbz x13, 21f\n"
    "20:"  // Right padding loop
    ".inst 0xc0060c1c  // mova { z28.d-z31.d }, za.d[x8, #0]\n"
    ".inst 0xc1a8ac1c  // sqdmulh { z28.s-z31.s }, { z28.s-z31.s }, z8.s\n"
    "add x8, x8, #0x1\n"
    ".inst 0xc1a7aa3c  // srshl { z28.s-z31.s }, { z28.s-z31.s }, z7.s\n"
    "subs x13, x13, #0x1\n"
    ".inst 0xc0040c02  // mova za.d[x8, #2], { z0.d-z3.d }\n"
    ".inst 0xc1a4ab1c  // add { z28.s-z31.s }, { z28.s-z31.s }, z4.s\n"
    ".inst 0xc1b7cf7c  // sclamp { z28.s-z31.s }, z27.s, z23.s\n"
    "st1b { z28.s }, p1, [x11]\n"
    "add x11, x11, x9\n"
    "st1b { z29.s }, p1, [x10]\n"
    "add x10, x10, x28\n"
    "st1b { z30.s }, p1, [x27]\n"
    "add x27, x27, x25\n"
    "st1b { z31.s }, p1, [x26]\n"
    "add x26, x26, x24\n"
    "bgt 20b\n"
    "21:"  // End
    "ldr x22, [%x[args], %[offsetof_Args_weights]]\n"
    "incw x22, ALL, MUL #9\n"
    "str x22, [%x[args], %[offsetof_Args_weights]]\n"
    "incw x16\n"
    "ldr x20, [%x[args], %[offsetof_Args_ld_in_vl]]\n"
    "whilelt p1.s, x16, x17\n"
    "ldr x14, [%x[args], %[offsetof_Args_inptr]]\n"
    "add x14, x14, x20\n"
    "str x14, [%x[args], %[offsetof_Args_inptr]]\n"
    "ldr x25, [%x[args], %[offsetof_Args_outptrs]]\n"
    "ldr x24, [%x[args], %[offsetof_Args_ld_out_vls]]\n"
    "ldp x23, x22, [x25, #0x0]\n"
    "ldp x21, x20, [x24, #0x0]\n"
    "add x23, x23, x21\n"
    "add x22, x22, x20\n"
    "stp x23, x22, [x25, #0x0]\n"
    "ldp x23, x22, [x25, #0x10]\n"
    "ldp x21, x20, [x24, #0x10]\n"
    "add x23, x23, x21\n"
    "add x22, x22, x20\n"
    "stp x23, x22, [x25, #0x10]\n"
    "b.any 1b\n"
    "addvl SP, SP, #6\n"
    ".inst 0xd503467f  // SMSTOP\n"
    :
    : [args] "r" (&args), [ld_in_col] "r" (ld_in_col), [ld_in_row] "r" (ld_in_row), [offsetof_Args_current_channel] "I" (offsetof(Args, current_channel)), [offsetof_Args_inptr] "I" (offsetof(Args, inptr)), [offsetof_Args_input_cols] "I" (offsetof(Args, input_cols)), [offsetof_Args_ld_in_vl] "I" (offsetof(Args, ld_in_vl)), [offsetof_Args_ld_out_cols] "I" (offsetof(Args, ld_out_cols)), [offsetof_Args_ld_out_vls] "I" (offsetof(Args, ld_out_vls)), [offsetof_Args_n_channels] "I" (offsetof(Args, n_channels)), [offsetof_Args_outptrs] "I" (offsetof(Args, outptrs)), [offsetof_Args_output_cols] "I" (offsetof(Args, output_cols)), [offsetof_Args_pad_bottom] "I" (offsetof(Args, pad_bottom)), [offsetof_Args_pad_left] "I" (offsetof(Args, pad_left)), [offsetof_Args_pad_top] "I" (offsetof(Args, pad_top)), [offsetof_Args_weights] "I" (offsetof(Args, weights)), [offsetof_Requantize32_a_offset] "I" (offsetof(arm_gemm::Requantize32, a_offset)), [offsetof_Requantize32_b_offset] "I" (offsetof(arm_gemm::Requantize32, b_offset)), [offsetof_Requantize32_bias] "I" (offsetof(arm_gemm::Requantize32, bias)), [offsetof_Requantize32_c_offset] "I" (offsetof(arm_gemm::Requantize32, c_offset)), [offsetof_Requantize32_maxval] "I" (offsetof(arm_gemm::Requantize32, maxval)), [offsetof_Requantize32_minval] "I" (offsetof(arm_gemm::Requantize32, minval)), [offsetof_Requantize32_per_channel_muls] "I" (offsetof(arm_gemm::Requantize32, per_channel_muls)), [offsetof_Requantize32_per_channel_right_shifts] "I" (offsetof(arm_gemm::Requantize32, per_channel_right_shifts)), [offsetof_Requantize32_per_layer_mul] "I" (offsetof(arm_gemm::Requantize32, per_layer_mul)), [offsetof_Requantize32_per_layer_right_shift] "I" (offsetof(arm_gemm::Requantize32, per_layer_right_shift)), [qp] "r" (&qp)
    : "cc", "memory", "p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
  );
}

}  // namespace depthwise
}  // namespace arm_conv

#endif  // defined(ARM_COMPUTE_ENABLE_SME2)
