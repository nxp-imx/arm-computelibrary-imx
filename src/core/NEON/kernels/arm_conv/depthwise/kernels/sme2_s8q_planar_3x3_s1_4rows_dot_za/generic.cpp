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

void sme2_s8q_planar_3x3_s1_4rows_dot_za_impl(
  const int8_t *inptr,
  size_t ld_in_row,
  size_t ld_in_col,
  size_t ld_in_vl,
  unsigned int pad_top,
  unsigned int valid_input_rows,
  unsigned int pad_left,
  unsigned int valid_input_cols,
  const int8_t *weights,
  int8_t **outptrs,
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
    const int8_t *inptr;
    size_t ld_in_vl;
    long unsigned int pad_top, pad_bottom, pad_left;
    const int8_t *weights;
    long unsigned int input_cols, output_cols;
    int8_t **outptrs;
    const size_t *ld_out_cols;
    const size_t *ld_out_vls;
    long unsigned int current_channel, n_channels;
  };

  Args args = { inptr, ld_in_vl, pad_top, 6u - std::min(6u, pad_top + valid_input_rows), pad_left, weights, valid_input_cols, output_cols, outptrs, outlds, outvllds, start_channel, valid_channels };

  __asm__ __volatile__(
    ".inst 0xd503477f  // SMSTART ZA\n"
    "ldr x6, [%x[args], %[offsetof_Args_pad_bottom]]\n"
    "ptrue p2.b\n"
    "mov x20, #0x6\n"
    "ldr x7, [%x[args], %[offsetof_Args_pad_top]]\n"
    "ld1rh { z24.h }, p2/Z, [%x[qp], %[offsetof_Requantize32_a_offset]]\n"
    "sub x20, x20, x6\n"
    ".inst 0x25207812  // ptrue pn10.b\n"
    "ldr x17, [%x[args], %[offsetof_Args_n_channels]]\n"
    "whilelt p1.s, XZR, x17\n"
    "whilelt p9.s, XZR, x20\n"
    "ld1rw { z12.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_c_offset]]\n"
    "whilelt p8.s, XZR, x7\n"
    "addvl SP, SP, #-12\n"
    "ldr x16, [%x[args], %[offsetof_Args_current_channel]]\n"
    "neg z24.h, p2/M, z24.h\n"
    "eor p8.b, p2/Z, p8.b, p9.b\n"
    "ld1rw { z10.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_per_layer_mul]]\n"
    "ld1rw { z11.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_per_layer_right_shift]]\n"
    "ld1rw { z22.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_minval]]\n"
    "ld1rw { z26.s }, p2/Z, [%x[qp], %[offsetof_Requantize32_maxval]]\n"
    "1:"  // Channel loop
    "ldr x20, [%x[qp], %[offsetof_Requantize32_bias]]\n"
    "mov z8.s, #0x0\n"
    "cbz x20, 2f\n"
    "ld1w { z8.s }, p1/Z, [x20, x16, LSL #2]\n"
    "2:"  // Load bias: Done
    "ldr x22, [%x[args], %[offsetof_Args_weights]]\n"
    "mov x20, x22\n"
    "ld1sb { z27.s }, p2/Z, [x20]\n"
    "incw x20, ALL, MUL #3\n"
    "ld1rh { z21.h }, p2/Z, [%x[qp], %[offsetof_Requantize32_b_offset]]\n"
    "mov z20.h, #0x0\n"
    "sub z27.h, z27.h, z21.h\n"
    "incw x22\n"
    "ld1sb { z23.s }, p2/Z, [x20]\n"
    "incw x20, ALL, MUL #3\n"
    "sub z23.h, z23.h, z21.h\n"
    "trn1 z0.h, z20.h, z27.h\n"
    "ld1sb { z16.s }, p2/Z, [x20]\n"
    "sub z16.h, z16.h, z21.h\n"
    "mov x20, x22\n"
    "trn1 z1.h, z27.h, z23.h\n"
    "ld1sb { z27.s }, p2/Z, [x20]\n"
    "incw x20, ALL, MUL #3\n"
    "trn1 z2.h, z23.h, z16.h\n"
    "trn1 z3.h, z16.h, z20.h\n"
    "ld1sb { z23.s }, p2/Z, [x20]\n"
    "incw x20, ALL, MUL #3\n"
    "sub z27.h, z27.h, z21.h\n"
    "sub z23.h, z23.h, z21.h\n"
    "ld1sb { z16.s }, p2/Z, [x20]\n"
    "sub z16.h, z16.h, z21.h\n"
    "addvl x21, SP, #12\n"
    "incw x22\n"
    "addvl x21, x21, #-4\n"
    "mov x20, x22\n"
    "st1h { z0.h }, p2, [x21]\n"
    "trn1 z0.h, z20.h, z27.h\n"
    "st1h { z1.h }, p2, [x21, #1, MUL VL]\n"
    "trn1 z1.h, z27.h, z23.h\n"
    "ld1sb { z27.s }, p2/Z, [x20]\n"
    "incw x20, ALL, MUL #3\n"
    "st1h { z2.h }, p2, [x21, #2, MUL VL]\n"
    "trn1 z2.h, z23.h, z16.h\n"
    "ld1sb { z23.s }, p2/Z, [x20]\n"
    "incw x20, ALL, MUL #3\n"
    "st1h { z3.h }, p2, [x21, #3, MUL VL]\n"
    "trn1 z3.h, z16.h, z20.h\n"
    "ld1sb { z16.s }, p2/Z, [x20]\n"
    "ldr x20, [%x[qp], %[offsetof_Requantize32_per_channel_muls]]\n"
    "sub z27.h, z27.h, z21.h\n"
    "sub z23.h, z23.h, z21.h\n"
    "addvl x21, x21, #-4\n"
    "st1h { z0.h }, p2, [x21]\n"
    "sub z16.h, z16.h, z21.h\n"
    "st1h { z1.h }, p2, [x21, #1, MUL VL]\n"
    "mov z9.d, z8.d\n"
    "st1h { z2.h }, p2, [x21, #2, MUL VL]\n"
    "trn1 z0.h, z20.h, z27.h\n"
    "trn1 z1.h, z27.h, z23.h\n"
    "st1h { z3.h }, p2, [x21, #3, MUL VL]\n"
    "addvl x21, x21, #-4\n"
    "trn1 z2.h, z23.h, z16.h\n"
    "trn1 z3.h, z16.h, z20.h\n"
    "st1h { z0.h }, p2, [x21]\n"
    "st1h { z1.h }, p2, [x21, #1, MUL VL]\n"
    "st1h { z2.h }, p2, [x21, #2, MUL VL]\n"
    "st1h { z3.h }, p2, [x21, #3, MUL VL]\n"
    "cbz x20, 3f\n"
    "ld1w { z10.s }, p1/Z, [x20, x16, LSL #2]\n"
    "3:"  // Load mul: End
    "ldr x20, [%x[qp], %[offsetof_Requantize32_per_channel_right_shifts]]\n"
    "cbz x20, 4f\n"
    "ld1w { z11.s }, p1/Z, [x20, x16, LSL #2]\n"
    "4:"  // Load right_shift: End
    "ldr x15, [%x[args], %[offsetof_Args_input_cols]]\n"
    "sub x20, x15, #0x1\n"
    "orr x23, x20, %x[ld_in_col], LSL #16\n"
    "ldr x14, [%x[args], %[offsetof_Args_inptr]]\n"
    "orr x23, x17, x23, LSL #22\n"
    "mov x22, #0x6\n"
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
    ".inst 0xc0040900  // mova za.d[x8, #0], { z8.d-z9.d }\n"
    "ldr x20, [%x[args], %[offsetof_Args_ld_out_cols]]\n"
    ".inst 0xc0040901  // mova za.d[x8, #1], { z8.d-z9.d }\n"
    "mov x22, #0x2\n"
    "ldp x11, x10, [x25], #0x10\n"
    ".inst 0xc0040902  // mova za.d[x8, #2], { z8.d-z9.d }\n"
    "ldp x9, x28, [x20], #0x10\n"
    ".inst 0xc0040903  // mova za.d[x8, #3], { z8.d-z9.d }\n"
    "ldr x21, [%x[args], %[offsetof_Args_pad_left]]\n"
    ".inst 0xc0040904  // mova za.d[x8, #4], { z8.d-z9.d }\n"
    "ldp x27, x26, [x25], #0x10\n"
    ".inst 0xc0040905  // mova za.d[x8, #5], { z8.d-z9.d }\n"
    "ldp x25, x24, [x20], #0x10\n"
    "cbz x21, 7f\n"
    "cmp x21, x22\n"
    "csel x20, x21, x22, LT\n"
    "sub x21, x21, x20\n"
    "sub x22, x22, x20\n"
    "cbz x21, 7f\n"
    ".inst 0xc0060804  // mova { z4.d-z5.d }, za.d[x8, #0]\n"
    "sub x13, x13, x21\n"
    ".inst 0xc0060826  // mova { z6.d-z7.d }, za.d[x8, #1]\n"
    ".inst 0xc1aaac04  // sqdmulh { z4.s-z7.s }, { z4.s-z7.s }, z10.s\n"
    ".inst 0xc1abaa24  // srshl { z4.s-z7.s }, { z4.s-z7.s }, z11.s\n"
    ".inst 0xc1acab04  // add { z4.s-z7.s }, { z4.s-z7.s }, z12.s\n"
    ".inst 0xc1bacec4  // sclamp { z4.s-z7.s }, z22.s, z26.s\n"
    "6:"  // Left padding
    "subs x21, x21, #0x1\n"
    "st1b { z4.s }, p1, [x11]\n"
    "add x11, x11, x9\n"
    "st1b { z6.s }, p1, [x10]\n"
    "add x10, x10, x28\n"
    "st1b { z5.s }, p1, [x27]\n"
    "add x27, x27, x25\n"
    "st1b { z7.s }, p1, [x26]\n"
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
    "ld1sb { z17.s }, p1/Z, [x14]\n"
    "addvl x20, SP, #8\n"
    "ld1sb { z16.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z13.h, z17.h, z16.h\n"
    "add z13.h, z13.h, z24.h\n"
    "ld1sb { z17.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "add x14, x14, %x[ld_in_col]\n"
    "ld1sb { z16.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "trn1 z14.h, z17.h, z16.h\n"
    "add z14.h, z14.h, z24.h\n"
    "ld1sb { z17.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row]\n"
    "ld1sb { z16.s }, p1/Z, [x21]\n"
    "trn1 z15.h, z17.h, z16.h\n"
    "add z15.h, z15.h, z24.h\n"
    ".inst 0xa0402a80  // ld1h { z0.h-z1.h }, pn10.b/Z, [x20]\n"
    ".inst 0xc16115a8  // sdot za.s[x8, 0], { z13.h-z14.h }, z1.h\n"
    ".inst 0xc16015a9  // sdot za.s[x8, 1], { z13.h-z14.h }, z0.h\n"
    ".inst 0xa0412a82  // ld1h { z2.h-z3.h }, pn10.b/Z, [x20, #0x2, MUL VL]\n"
    ".inst 0xc16315c8  // sdot za.s[x8, 0], { z14.h-z15.h }, z3.h\n"
    ".inst 0xc16215c9  // sdot za.s[x8, 1], { z14.h-z15.h }, z2.h\n"
    "9:"  // Unpadded: 1 priming loads
    "add x22, x14, %x[ld_in_row]\n"
    "ld1sb { z17.s }, p1/Z, [x14]\n"
    "addvl x21, SP, #4\n"
    "ld1sb { z16.s }, p1/Z, [x22]\n"
    "add x22, x22, %x[ld_in_row]\n"
    "trn1 z13.h, z17.h, z16.h\n"
    "add z13.h, z13.h, z24.h\n"
    "ld1sb { z17.s }, p1/Z, [x22]\n"
    "add x22, x22, %x[ld_in_row]\n"
    "addvl x20, SP, #8\n"
    "ld1sb { z16.s }, p1/Z, [x22]\n"
    "add x22, x22, %x[ld_in_row]\n"
    "trn1 z14.h, z17.h, z16.h\n"
    "add z14.h, z14.h, z24.h\n"
    "ld1sb { z17.s }, p1/Z, [x22]\n"
    "add x22, x22, %x[ld_in_row]\n"
    "add x14, x14, %x[ld_in_col]\n"
    "ld1sb { z16.s }, p1/Z, [x22]\n"
    "trn1 z15.h, z17.h, z16.h\n"
    "add z15.h, z15.h, z24.h\n"
    ".inst 0xa0402aa0  // ld1h { z0.h-z1.h }, pn10.b/Z, [x21]\n"
    ".inst 0xc16115a8  // sdot za.s[x8, 0], { z13.h-z14.h }, z1.h\n"
    ".inst 0xc16015a9  // sdot za.s[x8, 1], { z13.h-z14.h }, z0.h\n"
    ".inst 0xa0402a80  // ld1h { z0.h-z1.h }, pn10.b/Z, [x20]\n"
    ".inst 0xa0412aa2  // ld1h { z2.h-z3.h }, pn10.b/Z, [x21, #0x2, MUL VL]\n"
    ".inst 0xc16115aa  // sdot za.s[x8, 2], { z13.h-z14.h }, z1.h\n"
    ".inst 0xc16015ab  // sdot za.s[x8, 3], { z13.h-z14.h }, z0.h\n"
    ".inst 0xc16315c8  // sdot za.s[x8, 0], { z14.h-z15.h }, z3.h\n"
    ".inst 0xc16215c9  // sdot za.s[x8, 1], { z14.h-z15.h }, z2.h\n"
    ".inst 0xa0412a82  // ld1h { z2.h-z3.h }, pn10.b/Z, [x20, #0x2, MUL VL]\n"
    ".inst 0xc16315ca  // sdot za.s[x8, 2], { z14.h-z15.h }, z3.h\n"
    ".inst 0xc16215cb  // sdot za.s[x8, 3], { z14.h-z15.h }, z2.h\n"
    "10:"  // Unpadded: 0 priming loads
    ".inst 0xa0402be0  // ld1h { z0.h-z1.h }, pn10.b/Z, [SP]\n"
    ".inst 0xa0412be2  // ld1h { z2.h-z3.h }, pn10.b/Z, [SP, #0x2, MUL VL]\n"
    "cbz x15, 18f\n"
    "add x20, x14, %x[ld_in_row]\n"
    "ld1sb { z17.s }, p1/Z, [x14]\n"
    "sub x15, x15, #0x1\n"
    "ld1sb { z16.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    "trn1 z13.h, z17.h, z16.h\n"
    "sub x13, x13, #0x1\n"
    "ld1sb { z17.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    "cmp x15, x13\n"
    "add z13.h, z13.h, z24.h\n"
    "ld1sb { z16.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    "trn1 z14.h, z17.h, z16.h\n"
    "csel x23, x15, x13, LT\n"
    "ld1sb { z17.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    "add z14.h, z14.h, z24.h\n"
    "add x14, x14, %x[ld_in_col]\n"
    "ld1sb { z16.s }, p1/Z, [x20]\n"
    "trn1 z15.h, z17.h, z16.h\n"
    "add z15.h, z15.h, z24.h\n"
    "sub x13, x13, x23\n"
    "cbz x23, 17f\n"
    "11:"  // Unpadded: Main loop
    ".inst 0xc16115a8  // sdot za.s[x8, 0], { z13.h-z14.h }, z1.h\n"
    "addvl x22, SP, #4\n"
    "addvl x21, SP, #8\n"
    "ld1sb { z21.s }, p1/Z, [x14]\n"
    ".inst 0xc16015a9  // sdot za.s[x8, 1], { z13.h-z14.h }, z0.h\n"
    ".inst 0xa0402ac0  // ld1h { z0.h-z1.h }, pn10.b/Z, [x22]\n"
    "add x20, x14, %x[ld_in_row]\n"
    "subs x23, x23, #0x1\n"
    ".inst 0xc16315c8  // sdot za.s[x8, 0], { z14.h-z15.h }, z3.h\n"
    "ld1sb { z20.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    "add x14, x14, %x[ld_in_col]\n"
    ".inst 0xc16215c9  // sdot za.s[x8, 1], { z14.h-z15.h }, z2.h\n"
    ".inst 0xa0412ac2  // ld1h { z2.h-z3.h }, pn10.b/Z, [x22, #0x2, MUL VL]\n"
    ".inst 0xc0060804  // mova { z4.d-z5.d }, za.d[x8, #0]\n"
    "ld1sb { z19.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0xc0060826  // mova { z6.d-z7.d }, za.d[x8, #1]\n"
    ".inst 0xc1aaac04  // sqdmulh { z4.s-z7.s }, { z4.s-z7.s }, z10.s\n"
    ".inst 0xc16115aa  // sdot za.s[x8, 2], { z13.h-z14.h }, z1.h\n"
    "ld1sb { z18.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0xc16015ab  // sdot za.s[x8, 3], { z13.h-z14.h }, z0.h\n"
    ".inst 0xa0402aa0  // ld1h { z0.h-z1.h }, pn10.b/Z, [x21]\n"
    ".inst 0xc1abaa24  // srshl { z4.s-z7.s }, { z4.s-z7.s }, z11.s\n"
    ".inst 0xc16115ac  // sdot za.s[x8, 4], { z13.h-z14.h }, z1.h\n"
    "ld1sb { z17.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0xc16015ad  // sdot za.s[x8, 5], { z13.h-z14.h }, z0.h\n"
    "ld1sb { z16.s }, p1/Z, [x20]\n"
    ".inst 0xc1acab04  // add { z4.s-z7.s }, { z4.s-z7.s }, z12.s\n"
    ".inst 0xc16315ca  // sdot za.s[x8, 2], { z14.h-z15.h }, z3.h\n"
    "trn1 z13.h, z21.h, z20.h\n"
    ".inst 0xa0402be0  // ld1h { z0.h-z1.h }, pn10.b/Z, [SP]\n"
    ".inst 0xc16215cb  // sdot za.s[x8, 3], { z14.h-z15.h }, z2.h\n"
    ".inst 0xa0412aa2  // ld1h { z2.h-z3.h }, pn10.b/Z, [x21, #0x2, MUL VL]\n"
    ".inst 0xc1bacec4  // sclamp { z4.s-z7.s }, z22.s, z26.s\n"
    ".inst 0xc16315cc  // sdot za.s[x8, 4], { z14.h-z15.h }, z3.h\n"
    "st1b { z4.s }, p1, [x11]\n"
    "add x11, x11, x9\n"
    "add z13.h, z13.h, z24.h\n"
    ".inst 0xc16215cd  // sdot za.s[x8, 5], { z14.h-z15.h }, z2.h\n"
    "trn1 z14.h, z19.h, z18.h\n"
    "trn1 z15.h, z17.h, z16.h\n"
    "add x8, x8, #0x2\n"
    ".inst 0xa0412be2  // ld1h { z2.h-z3.h }, pn10.b/Z, [SP, #0x2, MUL VL]\n"
    "st1b { z6.s }, p1, [x10]\n"
    "add x10, x10, x28\n"
    ".inst 0xc0040904  // mova za.d[x8, #4], { z8.d-z9.d }\n"
    "st1b { z5.s }, p1, [x27]\n"
    "add x27, x27, x25\n"
    ".inst 0xc0040905  // mova za.d[x8, #5], { z8.d-z9.d }\n"
    "add z14.h, z14.h, z24.h\n"
    "st1b { z7.s }, p1, [x26]\n"
    "add x26, x26, x24\n"
    "add z15.h, z15.h, z24.h\n"
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
    "ld1sb { z19.s }, p0/Z, [x14]\n"
    "add z19.h, p0/M, z19.h, z24.h\n"
    "add x20, x14, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1sb { z18.s }, p0/Z, [x20]\n"
    "add z18.h, p0/M, z18.h, z24.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1sb { z17.s }, p0/Z, [x20]\n"
    "add z17.h, p0/M, z17.h, z24.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1sb { z16.s }, p0/Z, [x20]\n"
    "add z16.h, p0/M, z16.h, z24.h\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row]\n"
    "trn1 z13.h, z19.h, z18.h\n"
    "trn1 z14.h, z17.h, z16.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1sb { z17.s }, p0/Z, [x20]\n"
    "add z17.h, p0/M, z17.h, z24.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1sb { z16.s }, p0/Z, [x20]\n"
    "addvl x20, SP, #8\n"
    "add z16.h, p0/M, z16.h, z24.h\n"
    ".inst 0xa0402a80  // ld1h { z0.h-z1.h }, pn10.b/Z, [x20]\n"
    "trn1 z15.h, z17.h, z16.h\n"
    ".inst 0xc16115a8  // sdot za.s[x8, 0], { z13.h-z14.h }, z1.h\n"
    "add x14, x14, %x[ld_in_col]\n"
    ".inst 0xc16015a9  // sdot za.s[x8, 1], { z13.h-z14.h }, z0.h\n"
    ".inst 0xa0412a82  // ld1h { z2.h-z3.h }, pn10.b/Z, [x20, #0x2, MUL VL]\n"
    ".inst 0xc16315c8  // sdot za.s[x8, 0], { z14.h-z15.h }, z3.h\n"
    ".inst 0xc16215c9  // sdot za.s[x8, 1], { z14.h-z15.h }, z2.h\n"
    "14:"  // Padded: 1 priming loads
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1sb { z19.s }, p0/Z, [x14]\n"
    "add z19.h, p0/M, z19.h, z24.h\n"
    "add x20, x14, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1sb { z18.s }, p0/Z, [x20]\n"
    "add z18.h, p0/M, z18.h, z24.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1sb { z17.s }, p0/Z, [x20]\n"
    "add z17.h, p0/M, z17.h, z24.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1sb { z16.s }, p0/Z, [x20]\n"
    "add z16.h, p0/M, z16.h, z24.h\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row]\n"
    "trn1 z13.h, z19.h, z18.h\n"
    "trn1 z14.h, z17.h, z16.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1sb { z17.s }, p0/Z, [x20]\n"
    "add z17.h, p0/M, z17.h, z24.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1sb { z16.s }, p0/Z, [x20]\n"
    "addvl x21, SP, #4\n"
    "add z16.h, p0/M, z16.h, z24.h\n"
    ".inst 0xa0402aa0  // ld1h { z0.h-z1.h }, pn10.b/Z, [x21]\n"
    "addvl x20, SP, #8\n"
    "trn1 z15.h, z17.h, z16.h\n"
    ".inst 0xc16115a8  // sdot za.s[x8, 0], { z13.h-z14.h }, z1.h\n"
    ".inst 0xc16015a9  // sdot za.s[x8, 1], { z13.h-z14.h }, z0.h\n"
    ".inst 0xa0402a80  // ld1h { z0.h-z1.h }, pn10.b/Z, [x20]\n"
    "add x14, x14, %x[ld_in_col]\n"
    ".inst 0xa0412aa2  // ld1h { z2.h-z3.h }, pn10.b/Z, [x21, #0x2, MUL VL]\n"
    ".inst 0xc16115aa  // sdot za.s[x8, 2], { z13.h-z14.h }, z1.h\n"
    ".inst 0xc16015ab  // sdot za.s[x8, 3], { z13.h-z14.h }, z0.h\n"
    ".inst 0xc16315c8  // sdot za.s[x8, 0], { z14.h-z15.h }, z3.h\n"
    ".inst 0xc16215c9  // sdot za.s[x8, 1], { z14.h-z15.h }, z2.h\n"
    ".inst 0xa0412a82  // ld1h { z2.h-z3.h }, pn10.b/Z, [x20, #0x2, MUL VL]\n"
    ".inst 0xc16315ca  // sdot za.s[x8, 2], { z14.h-z15.h }, z3.h\n"
    ".inst 0xc16215cb  // sdot za.s[x8, 3], { z14.h-z15.h }, z2.h\n"
    "15:"  // Padded: 0 priming loads
    ".inst 0xa0402be0  // ld1h { z0.h-z1.h }, pn10.b/Z, [SP]\n"
    ".inst 0xa0412be2  // ld1h { z2.h-z3.h }, pn10.b/Z, [SP, #0x2, MUL VL]\n"
    "cbz x15, 18f\n"
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1sb { z19.s }, p0/Z, [x14]\n"
    "add z19.h, p0/M, z19.h, z24.h\n"
    "add x20, x14, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1sb { z18.s }, p0/Z, [x20]\n"
    "add z18.h, p0/M, z18.h, z24.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1sb { z17.s }, p0/Z, [x20]\n"
    "add z17.h, p0/M, z17.h, z24.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1sb { z16.s }, p0/Z, [x20]\n"
    "add z16.h, p0/M, z16.h, z24.h\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row]\n"
    "trn1 z13.h, z19.h, z18.h\n"
    "trn1 z14.h, z17.h, z16.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1sb { z17.s }, p0/Z, [x20]\n"
    "add z17.h, p0/M, z17.h, z24.h\n"
    "add x20, x20, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1sb { z16.s }, p0/Z, [x20]\n"
    "add z16.h, p0/M, z16.h, z24.h\n"
    "sub x15, x15, #0x1\n"
    "sub x13, x13, #0x1\n"
    "cmp x15, x13\n"
    "trn1 z15.h, z17.h, z16.h\n"
    "csel x23, x15, x13, LT\n"
    "add x14, x14, %x[ld_in_col]\n"
    "sub x13, x13, x23\n"
    "cbz x23, 17f\n"
    "16:"  // Padded: Main loop
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1sb { z21.s }, p0/Z, [x14]\n"
    ".inst 0xc16115a8  // sdot za.s[x8, 0], { z13.h-z14.h }, z1.h\n"
    ".inst 0xc16015a9  // sdot za.s[x8, 1], { z13.h-z14.h }, z0.h\n"
    "add z21.h, p0/M, z21.h, z24.h\n"
    "add x22, x14, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1sb { z20.s }, p0/Z, [x22]\n"
    ".inst 0xc16315c8  // sdot za.s[x8, 0], { z14.h-z15.h }, z3.h\n"
    "add z20.h, p0/M, z20.h, z24.h\n"
    "add x22, x22, %x[ld_in_row]\n"
    ".inst 0xc16215c9  // sdot za.s[x8, 1], { z14.h-z15.h }, z2.h\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1sb { z19.s }, p0/Z, [x22]\n"
    "add z19.h, p0/M, z19.h, z24.h\n"
    ".inst 0xc0060804  // mova { z4.d-z5.d }, za.d[x8, #0]\n"
    "add x22, x22, %x[ld_in_row]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1sb { z18.s }, p0/Z, [x22]\n"
    ".inst 0xc0060826  // mova { z6.d-z7.d }, za.d[x8, #1]\n"
    "mov x12, #0x4\n"
    "addvl x21, SP, #4\n"
    "add z18.h, p0/M, z18.h, z24.h\n"
    ".inst 0xc1aaac04  // sqdmulh { z4.s-z7.s }, { z4.s-z7.s }, z10.s\n"
    "add x22, x22, %x[ld_in_row]\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    ".inst 0xa0402aa0  // ld1h { z0.h-z1.h }, pn10.b/Z, [x21]\n"
    "addvl x20, SP, #8\n"
    ".inst 0xc16115aa  // sdot za.s[x8, 2], { z13.h-z14.h }, z1.h\n"
    "subs x23, x23, #0x1\n"
    "ld1sb { z17.s }, p0/Z, [x22]\n"
    ".inst 0xc16015ab  // sdot za.s[x8, 3], { z13.h-z14.h }, z0.h\n"
    ".inst 0xc1abaa24  // srshl { z4.s-z7.s }, { z4.s-z7.s }, z11.s\n"
    ".inst 0xa0402a80  // ld1h { z0.h-z1.h }, pn10.b/Z, [x20]\n"
    "add z17.h, p0/M, z17.h, z24.h\n"
    "add x22, x22, %x[ld_in_row]\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    ".inst 0xa0412aa2  // ld1h { z2.h-z3.h }, pn10.b/Z, [x21, #0x2, MUL VL]\n"
    ".inst 0xc16115ac  // sdot za.s[x8, 4], { z13.h-z14.h }, z1.h\n"
    ".inst 0xc1acab04  // add { z4.s-z7.s }, { z4.s-z7.s }, z12.s\n"
    "ld1sb { z16.s }, p0/Z, [x22]\n"
    ".inst 0xc16015ad  // sdot za.s[x8, 5], { z13.h-z14.h }, z0.h\n"
    "add z16.h, p0/M, z16.h, z24.h\n"
    "add x14, x14, %x[ld_in_col]\n"
    ".inst 0xc16315ca  // sdot za.s[x8, 2], { z14.h-z15.h }, z3.h\n"
    ".inst 0xa0402be0  // ld1h { z0.h-z1.h }, pn10.b/Z, [SP]\n"
    ".inst 0xc1bacec4  // sclamp { z4.s-z7.s }, z22.s, z26.s\n"
    ".inst 0xc16215cb  // sdot za.s[x8, 3], { z14.h-z15.h }, z2.h\n"
    ".inst 0xa0412a82  // ld1h { z2.h-z3.h }, pn10.b/Z, [x20, #0x2, MUL VL]\n"
    "st1b { z4.s }, p1, [x11]\n"
    "add x11, x11, x9\n"
    ".inst 0xc16315cc  // sdot za.s[x8, 4], { z14.h-z15.h }, z3.h\n"
    "st1b { z6.s }, p1, [x10]\n"
    "add x10, x10, x28\n"
    "trn1 z13.h, z21.h, z20.h\n"
    ".inst 0xc16215cd  // sdot za.s[x8, 5], { z14.h-z15.h }, z2.h\n"
    "add x8, x8, #0x2\n"
    ".inst 0xa0412be2  // ld1h { z2.h-z3.h }, pn10.b/Z, [SP, #0x2, MUL VL]\n"
    "st1b { z5.s }, p1, [x27]\n"
    "add x27, x27, x25\n"
    "st1b { z7.s }, p1, [x26]\n"
    "add x26, x26, x24\n"
    ".inst 0xc0040904  // mova za.d[x8, #4], { z8.d-z9.d }\n"
    ".inst 0xc0040905  // mova za.d[x8, #5], { z8.d-z9.d }\n"
    "trn1 z14.h, z19.h, z18.h\n"
    "trn1 z15.h, z17.h, z16.h\n"
    "bgt 16b\n"
    "17:"  // Main loop tail
    ".inst 0xc16115a8  // sdot za.s[x8, 0], { z13.h-z14.h }, z1.h\n"
    "addvl x21, SP, #4\n"
    "addvl x20, SP, #8\n"
    ".inst 0xc16015a9  // sdot za.s[x8, 1], { z13.h-z14.h }, z0.h\n"
    ".inst 0xa0402aa0  // ld1h { z0.h-z1.h }, pn10.b/Z, [x21]\n"
    ".inst 0xc16315c8  // sdot za.s[x8, 0], { z14.h-z15.h }, z3.h\n"
    ".inst 0xc16215c9  // sdot za.s[x8, 1], { z14.h-z15.h }, z2.h\n"
    ".inst 0xa0412aa2  // ld1h { z2.h-z3.h }, pn10.b/Z, [x21, #0x2, MUL VL]\n"
    ".inst 0xc0060804  // mova { z4.d-z5.d }, za.d[x8, #0]\n"
    ".inst 0xc0060826  // mova { z6.d-z7.d }, za.d[x8, #1]\n"
    ".inst 0xc1aaac04  // sqdmulh { z4.s-z7.s }, { z4.s-z7.s }, z10.s\n"
    ".inst 0xc16115aa  // sdot za.s[x8, 2], { z13.h-z14.h }, z1.h\n"
    ".inst 0xc1abaa24  // srshl { z4.s-z7.s }, { z4.s-z7.s }, z11.s\n"
    ".inst 0xc16015ab  // sdot za.s[x8, 3], { z13.h-z14.h }, z0.h\n"
    ".inst 0xa0402a80  // ld1h { z0.h-z1.h }, pn10.b/Z, [x20]\n"
    ".inst 0xc1acab04  // add { z4.s-z7.s }, { z4.s-z7.s }, z12.s\n"
    ".inst 0xc16115ac  // sdot za.s[x8, 4], { z13.h-z14.h }, z1.h\n"
    ".inst 0xc1bacec4  // sclamp { z4.s-z7.s }, z22.s, z26.s\n"
    "st1b { z4.s }, p1, [x11]\n"
    "add x11, x11, x9\n"
    ".inst 0xc16015ad  // sdot za.s[x8, 5], { z13.h-z14.h }, z0.h\n"
    "st1b { z6.s }, p1, [x10]\n"
    "add x10, x10, x28\n"
    ".inst 0xc16315ca  // sdot za.s[x8, 2], { z14.h-z15.h }, z3.h\n"
    "st1b { z5.s }, p1, [x27]\n"
    "add x27, x27, x25\n"
    ".inst 0xc16215cb  // sdot za.s[x8, 3], { z14.h-z15.h }, z2.h\n"
    ".inst 0xa0412a82  // ld1h { z2.h-z3.h }, pn10.b/Z, [x20, #0x2, MUL VL]\n"
    "st1b { z7.s }, p1, [x26]\n"
    "add x26, x26, x24\n"
    ".inst 0xc16315cc  // sdot za.s[x8, 4], { z14.h-z15.h }, z3.h\n"
    ".inst 0xc16215cd  // sdot za.s[x8, 5], { z14.h-z15.h }, z2.h\n"
    "add x8, x8, #0x2\n"
    ".inst 0xc0040904  // mova za.d[x8, #4], { z8.d-z9.d }\n"
    ".inst 0xc0040905  // mova za.d[x8, #5], { z8.d-z9.d }\n"
    "18:"  // Main loop skip tail
    "cbz x13, 20f\n"
    "19:"  // Right padding loop
    ".inst 0xc0060804  // mova { z4.d-z5.d }, za.d[x8, #0]\n"
    "subs x13, x13, #0x1\n"
    ".inst 0xc0060826  // mova { z6.d-z7.d }, za.d[x8, #1]\n"
    ".inst 0xc1aaac04  // sqdmulh { z4.s-z7.s }, { z4.s-z7.s }, z10.s\n"
    "add x8, x8, #0x2\n"
    ".inst 0xc1abaa24  // srshl { z4.s-z7.s }, { z4.s-z7.s }, z11.s\n"
    ".inst 0xc0040904  // mova za.d[x8, #4], { z8.d-z9.d }\n"
    ".inst 0xc1acab04  // add { z4.s-z7.s }, { z4.s-z7.s }, z12.s\n"
    ".inst 0xc0040905  // mova za.d[x8, #5], { z8.d-z9.d }\n"
    ".inst 0xc1bacec4  // sclamp { z4.s-z7.s }, z22.s, z26.s\n"
    "st1b { z4.s }, p1, [x11]\n"
    "add x11, x11, x9\n"
    "st1b { z6.s }, p1, [x10]\n"
    "add x10, x10, x28\n"
    "st1b { z5.s }, p1, [x27]\n"
    "add x27, x27, x25\n"
    "st1b { z7.s }, p1, [x26]\n"
    "add x26, x26, x24\n"
    "bgt 19b\n"
    "20:"  // End
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
    "addvl SP, SP, #12\n"
    ".inst 0xd503467f  // SMSTOP\n"
    :
    : [args] "r" (&args), [ld_in_col] "r" (ld_in_col), [ld_in_row] "r" (ld_in_row), [offsetof_Args_current_channel] "I" (offsetof(Args, current_channel)), [offsetof_Args_inptr] "I" (offsetof(Args, inptr)), [offsetof_Args_input_cols] "I" (offsetof(Args, input_cols)), [offsetof_Args_ld_in_vl] "I" (offsetof(Args, ld_in_vl)), [offsetof_Args_ld_out_cols] "I" (offsetof(Args, ld_out_cols)), [offsetof_Args_ld_out_vls] "I" (offsetof(Args, ld_out_vls)), [offsetof_Args_n_channels] "I" (offsetof(Args, n_channels)), [offsetof_Args_outptrs] "I" (offsetof(Args, outptrs)), [offsetof_Args_output_cols] "I" (offsetof(Args, output_cols)), [offsetof_Args_pad_bottom] "I" (offsetof(Args, pad_bottom)), [offsetof_Args_pad_left] "I" (offsetof(Args, pad_left)), [offsetof_Args_pad_top] "I" (offsetof(Args, pad_top)), [offsetof_Args_weights] "I" (offsetof(Args, weights)), [offsetof_Requantize32_a_offset] "I" (offsetof(arm_gemm::Requantize32, a_offset)), [offsetof_Requantize32_b_offset] "I" (offsetof(arm_gemm::Requantize32, b_offset)), [offsetof_Requantize32_bias] "I" (offsetof(arm_gemm::Requantize32, bias)), [offsetof_Requantize32_c_offset] "I" (offsetof(arm_gemm::Requantize32, c_offset)), [offsetof_Requantize32_maxval] "I" (offsetof(arm_gemm::Requantize32, maxval)), [offsetof_Requantize32_minval] "I" (offsetof(arm_gemm::Requantize32, minval)), [offsetof_Requantize32_per_channel_muls] "I" (offsetof(arm_gemm::Requantize32, per_channel_muls)), [offsetof_Requantize32_per_channel_right_shifts] "I" (offsetof(arm_gemm::Requantize32, per_channel_right_shifts)), [offsetof_Requantize32_per_layer_mul] "I" (offsetof(arm_gemm::Requantize32, per_layer_mul)), [offsetof_Requantize32_per_layer_right_shift] "I" (offsetof(arm_gemm::Requantize32, per_layer_right_shift)), [qp] "r" (&qp)
    : "cc", "memory", "p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
  );
}

}  // namespace depthwise
}  // namespace arm_conv

#endif  // defined(ARM_COMPUTE_ENABLE_SME2)
