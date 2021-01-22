/*
 * Copyright (c) 2021 Arm Limited.
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

#include <cstddef>
#include <cstdint>

#if defined(__ARM_FEATURE_SVE) && defined(__ARM_FP16_ARGS)

namespace arm_conv {
namespace depthwise {

void sve_fp16_nhwc_3x3_s1_output2x2_mla_depthfirst_direct_impl(
  const unsigned int n_tile_rows,
  const unsigned int n_tile_cols,
  const __fp16 *inptr,
  int64_t ld_input_row,
  int64_t ld_input_col,
  __fp16 *outptr,
  int64_t ld_output_row,
  int64_t ld_output_col,
  const void *params,
  unsigned int n_channels,
  const __fp16 activation_min,
  const __fp16 activation_max
)
{
  struct Args
  {
    const uint64_t n_tile_rows, n_tile_cols;
    const __fp16 *inptr;
    const uint64_t ld_input_row;
    const uint64_t ld_input_col;
    __fp16 *outptr;
    const uint64_t ld_output_row;
    const uint64_t ld_output_col;
    const void *params;
    const __fp16 min, max;

    uint64_t tile_i = 0, tile_j = 0;

    Args(
      const unsigned int n_tile_rows,
      const unsigned int n_tile_cols,
      const __fp16 *inptr,
      int64_t ld_input_row,
      int64_t ld_input_col,
      __fp16 *outptr,
      int64_t ld_output_row,
      int64_t ld_output_col,
      const void *params,
      const float activation_min,
      const float activation_max
    ) : n_tile_rows(n_tile_rows), n_tile_cols(n_tile_cols), inptr(inptr),
        ld_input_row(ld_input_row), ld_input_col(ld_input_col), outptr(outptr),
        ld_output_row(ld_output_row), ld_output_col(ld_output_col),
        params(params), min(activation_min), max(activation_max)
    {
    }
  };

  Args params_struct(
    n_tile_rows, n_tile_cols,
    inptr, ld_input_row, ld_input_col,
    outptr, ld_output_row, ld_output_col,
    params, activation_min, activation_max
  );

  __asm__ __volatile__(
    "ptrue p3.b\n"
    "mov x17, #0x0\n"
    "mov x16, #0x0\n"
    "1:"  // Tile loop
    "str x17, [%x[params_struct], %[offsetof_args_tile_i]]\n"
    "mov x23, #0x2\n"
    "str x16, [%x[params_struct], %[offsetof_args_tile_j]]\n"
    "mov x15, #0x2\n"
    "ldr x14, [%x[params_struct], %[offsetof_args_params]]\n"
    "mov x13, #0x0\n"
    "ldr x22, [%x[params_struct], %[offsetof_args_ld_input_row]]\n"
    "cnth x12\n"
    "ldr x11, [%x[params_struct], %[offsetof_args_ld_input_col]]\n"
    "sub x21, XZR, x12\n"
    "ldr x10, [%x[params_struct], %[offsetof_args_inptr]]\n"
    "mul x19, x17, x22\n" // offset = tile_i * ld_input_row
    "ldr x20, [%x[params_struct], %[offsetof_args_ld_output_row]]\n"
    "madd x19, x16, x11, x19\n" // offset += tile_j * ld_input_col
    "ldr x9, [%x[params_struct], %[offsetof_args_ld_output_col]]\n"
    "mul x19, x19, x23\n" // offset *= kernel_stride * output_size
    "ldr x28, [%x[params_struct], %[offsetof_args_outptr]]\n"
    "add x10, x10, x19, LSL #1\n" // inptr[0] += offset * sizeof(__fp16)
    "ld1rh { z18.h }, p3/Z, [%x[params_struct], %[offsetof_args_min]]\n"
    "add x27, x10, x22, LSL #1\n"
    "ld1rh { z17.h }, p3/Z, [%x[params_struct], %[offsetof_args_max]]\n"
    "add x26, x27, x22, LSL #1\n"
    "ld1h { z16.h }, p3/Z, [x14]\n" // Load from weights and bias
    "mov z31.d, z16.d\n"
    "ld1h { z0.h }, p3/Z, [x14, #1, MUL VL]\n" // Load from weights and bias
    "add x25, x26, x22, LSL #1\n"
    "mov z30.d, z16.d\n"
    "ld1h { z1.h }, p3/Z, [x14, #2, MUL VL]\n" // Load from weights and bias
    "add x24, x11, x11\n"
    "mov z29.d, z16.d\n"
    "ld1h { z2.h }, p3/Z, [x14, #3, MUL VL]\n" // Load from weights and bias
    "add x23, x24, x11\n"
    "mov z28.d, z16.d\n"
    "ld1h { z3.h }, p3/Z, [x14, #4, MUL VL]\n" // Load from weights and bias
    "mul x19, x17, x20\n" // offset = tile_i * ld_output_row
    "ld1h { z4.h }, p3/Z, [x14, #5, MUL VL]\n" // Load from weights and bias
    "madd x19, x16, x9, x19\n" // offset += tile_j * ld_output_col
    "ld1h { z5.h }, p3/Z, [x14, #6, MUL VL]\n" // Load from weights and bias
    "mul x19, x19, x15\n" // offset *= output_tile_size
    "ld1h { z6.h }, p3/Z, [x14, #7, MUL VL]\n" // Load from weights and bias
    "add x28, x28, x19, LSL #1\n" // outptrs[0] += offset * sizeof(__fp16)
    "whilelt p2.h, XZR, %x[n_channels]\n"
    "ld1h { z9.h }, p2/Z, [x27, x11, LSL #1]\n" // Load input point (1, 1)
    "ld1h { z10.h }, p2/Z, [x10]\n" // Load input point (0, 0)
    "add x22, x28, x20, LSL #1\n"
    "ld1h { z11.h }, p2/Z, [x10, x23, LSL #1]\n" // Load input point (0, 3)
    "addvl x14, x14, #16\n"
    "ld1h { z12.h }, p2/Z, [x27, x24, LSL #1]\n" // Load input point (1, 2)
    "cmp x12, %x[n_channels]\n"
    "ld1h { z7.h }, p3/Z, [x14, #-8, MUL VL]\n" // Load from weights and bias
    "ld1h { z8.h }, p3/Z, [x14, #-7, MUL VL]\n" // Load from weights and bias
    "addvl x14, x14, #-6\n"
    "ld1h { z13.h }, p2/Z, [x26, x11, LSL #1]\n" // Load input point (2, 1)
    "bge 3f\n"
    "2:"  // Tile loop: Channel loop
    "fmla z31.h, p3/M, z4.h, z9.h\n"
    "ld1h { z16.h }, p3/Z, [x14]\n" // Load from weights and bias
    "whilelt p1.h, x12, %x[n_channels]\n"
    "fmla z30.h, p3/M, z3.h, z9.h\n"
    "inch x21\n"
    "fmla z29.h, p3/M, z1.h, z9.h\n"
    "mov p0.b, p2.b\n"
    "fmla z28.h, p3/M, z0.h, z9.h\n"
    "ld1h { z9.h }, p2/Z, [x25]\n" // Load input point (3, 0)
    "inch x13\n"
    "fmla z31.h, p3/M, z0.h, z10.h\n"
    "ld1h { z10.h }, p2/Z, [x26, x24, LSL #1]\n" // Load input point (2, 2)
    "inch x12\n"
    "fmla z30.h, p3/M, z2.h, z11.h\n"
    "ld1h { z11.h }, p2/Z, [x25, x23, LSL #1]\n" // Load input point (3, 3)
    "fmla z29.h, p3/M, z2.h, z12.h\n"
    "fmla z28.h, p3/M, z1.h, z12.h\n"
    "fmla z31.h, p3/M, z5.h, z12.h\n"
    "fmla z30.h, p3/M, z4.h, z12.h\n"
    "ld1h { z12.h }, p2/Z, [x10, x11, LSL #1]\n" // Load input point (0, 1)
    "fmla z29.h, p3/M, z6.h, z9.h\n"
    "ld1h { z9.h }, p2/Z, [x10, x24, LSL #1]\n" // Load input point (0, 2)
    "addvl x10, x10, #1\n"
    "fmla z28.h, p3/M, z3.h, z13.h\n"
    "fmla z31.h, p3/M, z7.h, z13.h\n"
    "fmla z30.h, p3/M, z6.h, z13.h\n"
    "fmla z29.h, p3/M, z4.h, z13.h\n"
    "fmla z28.h, p3/M, z8.h, z11.h\n"
    "ld1h { z11.h }, p2/Z, [x27]\n" // Load input point (1, 0)
    "fmla z31.h, p3/M, z1.h, z12.h\n"
    "fmla z30.h, p3/M, z0.h, z12.h\n"
    "ld1h { z12.h }, p2/Z, [x27, x23, LSL #1]\n" // Load input point (1, 3)
    "addvl x27, x27, #1\n"
    "fmla z29.h, p3/M, z5.h, z10.h\n"
    "fmla z28.h, p3/M, z4.h, z10.h\n"
    "ld1h { z4.h }, p3/Z, [x14, #5, MUL VL]\n" // Load from weights and bias
    "fmla z31.h, p3/M, z2.h, z9.h\n"
    "fmla z30.h, p3/M, z1.h, z9.h\n"
    "ld1h { z9.h }, p2/Z, [x26]\n" // Load input point (2, 0)
    "ld1h { z1.h }, p3/Z, [x14, #2, MUL VL]\n" // Load from weights and bias
    "fmla z29.h, p3/M, z0.h, z11.h\n"
    "ld1h { z0.h }, p3/Z, [x14, #1, MUL VL]\n" // Load from weights and bias
    "fmla z28.h, p3/M, z2.h, z12.h\n"
    "ld1h { z2.h }, p3/Z, [x14, #3, MUL VL]\n" // Load from weights and bias
    "fmla z31.h, p3/M, z8.h, z10.h\n"
    "fmla z30.h, p3/M, z7.h, z10.h\n"
    "ld1h { z10.h }, p2/Z, [x26, x23, LSL #1]\n" // Load input point (2, 3)
    "addvl x26, x26, #1\n"
    "fmla z29.h, p3/M, z3.h, z9.h\n"
    "ld1h { z13.h }, p1/Z, [x26, x11, LSL #1]\n" // Load input point (2, 1)
    "fmla z31.h, p3/M, z3.h, z11.h\n"
    "ld1h { z11.h }, p2/Z, [x25, x11, LSL #1]\n" // Load input point (3, 1)
    "fmla z28.h, p3/M, z5.h, z10.h\n"
    "ld1h { z3.h }, p3/Z, [x14, #4, MUL VL]\n" // Load from weights and bias
    "fmla z30.h, p3/M, z5.h, z12.h\n"
    "ld1h { z12.h }, p2/Z, [x25, x24, LSL #1]\n" // Load input point (3, 2)
    "whilelt p2.h, x13, %x[n_channels]\n"
    "fmla z29.h, p3/M, z7.h, z11.h\n"
    "ld1h { z5.h }, p3/Z, [x14, #6, MUL VL]\n" // Load from weights and bias
    "addvl x25, x25, #1\n"
    "fmla z31.h, p3/M, z6.h, z9.h\n"
    "ld1h { z9.h }, p1/Z, [x27, x11, LSL #1]\n" // Load input point (1, 1)
    "cmp x12, %x[n_channels]\n"
    "fmla z30.h, p3/M, z8.h, z10.h\n"
    "ld1h { z10.h }, p1/Z, [x10]\n" // Load input point (0, 0)
    "fmla z28.h, p3/M, z6.h, z11.h\n"
    "ld1h { z11.h }, p1/Z, [x10, x23, LSL #1]\n" // Load input point (0, 3)
    "ld1h { z6.h }, p3/Z, [x14, #7, MUL VL]\n" // Load from weights and bias
    "fmla z29.h, p3/M, z8.h, z12.h\n"
    "addvl x14, x14, #16\n"
    "fmax z31.h, p3/M, z31.h, z18.h\n"
    "ld1h { z8.h }, p3/Z, [x14, #-7, MUL VL]\n" // Load from weights and bias
    "fmla z28.h, p3/M, z7.h, z12.h\n"
    "ld1h { z12.h }, p1/Z, [x27, x24, LSL #1]\n" // Load input point (1, 2)
    "fmax z30.h, p3/M, z30.h, z18.h\n"
    "ld1h { z7.h }, p3/Z, [x14, #-8, MUL VL]\n" // Load from weights and bias
    "addvl x14, x14, #-6\n"
    "fmax z29.h, p3/M, z29.h, z18.h\n"
    "fmin z31.h, p3/M, z31.h, z17.h\n"
    "st1h { z31.h }, p0, [x28]\n" // Store output point (0, 0)
    "mov z31.d, z16.d\n"
    "fmin z30.h, p3/M, z30.h, z17.h\n"
    "st1h { z30.h }, p0, [x28, x9, LSL #1]\n" // Store output point (0, 1)
    "mov z30.d, z16.d\n"
    "addvl x28, x28, #1\n"
    "fmin z29.h, p3/M, z29.h, z17.h\n"
    "st1h { z29.h }, p0, [x22]\n" // Store output point (1, 0)
    "mov z29.d, z16.d\n"
    "fmax z28.h, p3/M, z28.h, z18.h\n"
    "fmin z28.h, p3/M, z28.h, z17.h\n"
    "st1h { z28.h }, p0, [x22, x9, LSL #1]\n" // Store output point (1, 1)
    "mov z28.d, z16.d\n"
    "addvl x22, x22, #1\n"
    "blt 2b\n"
    "3:"  // Tile loop: Channel tail
    "fmla z31.h, p3/M, z4.h, z9.h\n"
    "ldr x17, [%x[params_struct], %[offsetof_args_tile_i]]\n"
    "mov p0.b, p2.b\n"
    "fmla z30.h, p3/M, z3.h, z9.h\n"
    "ldr x16, [%x[params_struct], %[offsetof_args_tile_j]]\n"
    "add x21, x17, #0x1\n"
    "fmla z29.h, p3/M, z1.h, z9.h\n"
    "ldr x20, [%x[params_struct], %[offsetof_args_n_tile_rows]]\n"
    "fmla z28.h, p3/M, z0.h, z9.h\n"
    "ld1h { z9.h }, p2/Z, [x25]\n" // Load input point (3, 0)
    "add x16, x16, #0x1\n"
    "fmla z31.h, p3/M, z0.h, z10.h\n"
    "ld1h { z10.h }, p2/Z, [x26, x24, LSL #1]\n" // Load input point (2, 2)
    "fmla z30.h, p3/M, z2.h, z11.h\n"
    "ld1h { z11.h }, p2/Z, [x25, x23, LSL #1]\n" // Load input point (3, 3)
    "ldr x19, [%x[params_struct], %[offsetof_args_n_tile_cols]]\n"
    "fmla z29.h, p3/M, z2.h, z12.h\n"
    "cmp x16, x19\n"
    "fmla z31.h, p3/M, z5.h, z12.h\n"
    "fmla z30.h, p3/M, z4.h, z12.h\n"
    "csel x16, x16, XZR, LT\n"
    "fmla z28.h, p3/M, z1.h, z12.h\n"
    "ld1h { z12.h }, p2/Z, [x10, x11, LSL #1]\n" // Load input point (0, 1)
    "csel x17, x17, x21, LT\n"
    "fmla z29.h, p3/M, z6.h, z9.h\n"
    "ld1h { z9.h }, p2/Z, [x10, x24, LSL #1]\n" // Load input point (0, 2)
    "cmp x17, x20\n"
    "fmla z31.h, p3/M, z7.h, z13.h\n"
    "fmla z30.h, p3/M, z6.h, z13.h\n"
    "fmla z28.h, p3/M, z3.h, z13.h\n"
    "fmla z29.h, p3/M, z4.h, z13.h\n"
    "fmla z31.h, p3/M, z1.h, z12.h\n"
    "fmla z30.h, p3/M, z0.h, z12.h\n"
    "ld1h { z12.h }, p2/Z, [x27, x23, LSL #1]\n" // Load input point (1, 3)
    "fmla z28.h, p3/M, z8.h, z11.h\n"
    "ld1h { z11.h }, p2/Z, [x27]\n" // Load input point (1, 0)
    "fmla z29.h, p3/M, z5.h, z10.h\n"
    "fmla z31.h, p3/M, z2.h, z9.h\n"
    "fmla z30.h, p3/M, z1.h, z9.h\n"
    "ld1h { z9.h }, p2/Z, [x26]\n" // Load input point (2, 0)
    "fmla z28.h, p3/M, z4.h, z10.h\n"
    "fmla z29.h, p3/M, z0.h, z11.h\n"
    "fmla z31.h, p3/M, z8.h, z10.h\n"
    "fmla z30.h, p3/M, z7.h, z10.h\n"
    "ld1h { z10.h }, p2/Z, [x26, x23, LSL #1]\n" // Load input point (2, 3)
    "fmla z28.h, p3/M, z2.h, z12.h\n"
    "fmla z29.h, p3/M, z3.h, z9.h\n"
    "fmla z31.h, p3/M, z3.h, z11.h\n"
    "ld1h { z11.h }, p2/Z, [x25, x11, LSL #1]\n" // Load input point (3, 1)
    "fmla z30.h, p3/M, z5.h, z12.h\n"
    "ld1h { z12.h }, p2/Z, [x25, x24, LSL #1]\n" // Load input point (3, 2)
    "fmla z28.h, p3/M, z5.h, z10.h\n"
    "fmla z29.h, p3/M, z7.h, z11.h\n"
    "fmla z31.h, p3/M, z6.h, z9.h\n"
    "fmla z30.h, p3/M, z8.h, z10.h\n"
    "fmla z28.h, p3/M, z6.h, z11.h\n"
    "fmla z29.h, p3/M, z8.h, z12.h\n"
    "fmax z31.h, p3/M, z31.h, z18.h\n"
    "fmax z30.h, p3/M, z30.h, z18.h\n"
    "fmla z28.h, p3/M, z7.h, z12.h\n"
    "fmax z29.h, p3/M, z29.h, z18.h\n"
    "fmin z31.h, p3/M, z31.h, z17.h\n"
    "st1h { z31.h }, p0, [x28]\n" // Store output point (0, 0)
    "fmin z30.h, p3/M, z30.h, z17.h\n"
    "fmin z29.h, p3/M, z29.h, z17.h\n"
    "st1h { z30.h }, p0, [x28, x9, LSL #1]\n" // Store output point (0, 1)
    "fmax z28.h, p3/M, z28.h, z18.h\n"
    "st1h { z29.h }, p0, [x22]\n" // Store output point (1, 0)
    "fmin z28.h, p3/M, z28.h, z17.h\n"
    "st1h { z28.h }, p0, [x22, x9, LSL #1]\n" // Store output point (1, 1)
    "blt 1b\n"
    :
    : [n_channels] "r" ((unsigned long) n_channels), [offsetof_args_inptr] "I" (offsetof(Args, inptr)), [offsetof_args_ld_input_col] "I" (offsetof(Args, ld_input_col)), [offsetof_args_ld_input_row] "I" (offsetof(Args, ld_input_row)), [offsetof_args_ld_output_col] "I" (offsetof(Args, ld_output_col)), [offsetof_args_ld_output_row] "I" (offsetof(Args, ld_output_row)), [offsetof_args_max] "I" (offsetof(Args, max)), [offsetof_args_min] "I" (offsetof(Args, min)), [offsetof_args_n_tile_cols] "I" (offsetof(Args, n_tile_cols)), [offsetof_args_n_tile_rows] "I" (offsetof(Args, n_tile_rows)), [offsetof_args_outptr] "I" (offsetof(Args, outptr)), [offsetof_args_params] "I" (offsetof(Args, params)), [offsetof_args_tile_i] "I" (offsetof(Args, tile_i)), [offsetof_args_tile_j] "I" (offsetof(Args, tile_j)), [params_struct] "r" (&params_struct)
    : "cc", "memory", "p0", "p1", "p2", "p3", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z16", "z17", "z18", "z28", "z29", "z30", "z31"
  );
}

}  // namespace depthwise
}  // namespace arm_conv

#endif  // defined(__ARM_FEATURE_SVE) && defined(__ARM_FP16_ARGS)
