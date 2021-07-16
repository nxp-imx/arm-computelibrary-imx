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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifdef __aarch64__

#include <cstddef>

namespace arm_gemm {

void a64_interleaved_fp32_mla_8x12_a55(
    const float *Apanel, const float *Bpanel,
    float *Cpanel, int ablocks, int bblocks, int K) {

    struct KernelArgs {
        size_t bblocks = {};
        size_t K = {};
        const float *Bpanel = {};
    } ka;

    ka.bblocks = bblocks;
    ka.K = (K/1) - 1;
    ka.Bpanel = Bpanel;

    __asm__ __volatile__(

      "1:"  // Height loop
      "ldr x28, [%x[args_ptr], %[offsetof_bblocks]]\n"
      "mov x27, %x[Apanel]\n"
      "ldr x26, [%x[args_ptr], %[offsetof_Bpanel]]\n"
      "2:"  // Width loop
      "ldr x25, [%x[args_ptr], %[offsetof_K]]\n"
      "mov %x[Apanel], x27\n"
      "cmp x25, #0x4\n"
      "movi v8.16b, #0x0\n"
      "movi v9.16b, #0x0\n"
      "prfm pldl1keep, [%x[Apanel], #0x0]\n"
      "movi v10.16b, #0x0\n"
      "prfm pldl1keep, [x26, #0x0]\n"
      "movi v11.16b, #0x0\n"
      "prfm pldl1keep, [x26, #0x40]\n"
      "movi v12.16b, #0x0\n"
      "prfm pldl1keep, [%x[Apanel], #0x40]\n"
      "movi v13.16b, #0x0\n"
      "prfm pldl1keep, [x26, #0x80]\n"
      "movi v14.16b, #0x0\n"
      "prfm pldl1keep, [%x[Apanel], #0x80]\n"
      "movi v15.16b, #0x0\n"
      "prfm pldl1keep, [x26, #0xc0]\n"
      "movi v16.16b, #0x0\n"
      "prfm pldl1keep, [x26, #0x100]\n"
      "movi v17.16b, #0x0\n"
      "prfm pldl1keep, [%x[Apanel], #0xc0]\n"
      "movi v18.16b, #0x0\n"
      "prfm pldl1keep, [x26, #0x140]\n"
      "movi v19.16b, #0x0\n"
      "ldr q0, [%x[Apanel], #0x0]\n"
      "movi v20.16b, #0x0\n"
      "ldr q1, [%x[Apanel], #0x10]\n"
      "movi v21.16b, #0x0\n"
      "ldr q4, [x26, #0x0]\n"
      "movi v22.16b, #0x0\n"
      "ldr q5, [x26, #0x10]\n"
      "movi v23.16b, #0x0\n"
      "ldr q6, [x26, #0x20]\n"
      "movi v24.16b, #0x0\n"
      "movi v25.16b, #0x0\n"
      "movi v26.16b, #0x0\n"
      "movi v27.16b, #0x0\n"
      "movi v28.16b, #0x0\n"
      "movi v29.16b, #0x0\n"
      "movi v30.16b, #0x0\n"
      "movi v31.16b, #0x0\n"
      "blt 4f\n"
      "3:"  // main loop head
      "ldr d2, [%x[Apanel], #0x20]\n"
      "fmla v8.4s, v4.4s, v0.s[0]\n"
      "ldr x21, [%x[Apanel], #0x28]\n"
      "fmla v11.4s, v4.4s, v0.s[1]\n"
      "ldr d3, [%x[Apanel], #0x30]\n"
      "fmla v14.4s, v4.4s, v0.s[2]\n"
      "ldr x20, [%x[Apanel], #0x38]\n"
      "fmla v17.4s, v4.4s, v0.s[3]\n"
      "ldr d7, [x26, #0x30]\n"
      "fmla v20.4s, v4.4s, v1.s[0]\n"
      "ldr x24, [x26, #0x38]\n"
      "fmla v23.4s, v4.4s, v1.s[1]\n"
      "fmla v26.4s, v4.4s, v1.s[2]\n"
      "ldr x23, [x26, #0x48]\n"
      "fmla v29.4s, v4.4s, v1.s[3]\n"
      "ldr d4, [x26, #0x40]\n"
      "fmla v9.4s, v5.4s, v0.s[0]\n"
      "mov v2.d[1], x21\n"
      "fmla v12.4s, v5.4s, v0.s[1]\n"
      "mov v3.d[1], x20\n"
      "fmla v15.4s, v5.4s, v0.s[2]\n"
      "mov v7.d[1], x24\n"
      "fmla v18.4s, v5.4s, v0.s[3]\n"
      "mov v4.d[1], x23\n"
      "fmla v21.4s, v5.4s, v1.s[0]\n"
      "ldr x22, [x26, #0x58]\n"
      "fmla v24.4s, v5.4s, v1.s[1]\n"
      "ldr x21, [%x[Apanel], #0x48]\n"
      "fmla v27.4s, v5.4s, v1.s[2]\n"
      "ldr x20, [%x[Apanel], #0x58]\n"
      "fmla v30.4s, v5.4s, v1.s[3]\n"
      "ldr d5, [x26, #0x50]\n"
      "fmla v10.4s, v6.4s, v0.s[0]\n"
      "ldr x19, [x26, #0x68]\n"
      "fmla v13.4s, v6.4s, v0.s[1]\n"
      "ldr x24, [x26, #0x78]\n"
      "fmla v16.4s, v6.4s, v0.s[2]\n"
      "mov v5.d[1], x22\n"
      "fmla v19.4s, v6.4s, v0.s[3]\n"
      "ldr d0, [%x[Apanel], #0x40]\n"
      "fmla v22.4s, v6.4s, v1.s[0]\n"
      "mov v0.d[1], x21\n"
      "fmla v25.4s, v6.4s, v1.s[1]\n"
      "ldr x23, [x26, #0x88]\n"
      "fmla v28.4s, v6.4s, v1.s[2]\n"
      "ldr x21, [%x[Apanel], #0x68]\n"
      "fmla v31.4s, v6.4s, v1.s[3]\n"
      "ldr d1, [%x[Apanel], #0x50]\n"
      "ldr d6, [x26, #0x60]\n"
      "fmla v8.4s, v7.4s, v2.s[0]\n"
      "fmla v11.4s, v7.4s, v2.s[1]\n"
      "mov v1.d[1], x20\n"
      "fmla v14.4s, v7.4s, v2.s[2]\n"
      "mov v6.d[1], x19\n"
      "fmla v17.4s, v7.4s, v2.s[3]\n"
      "ldr x20, [%x[Apanel], #0x78]\n"
      "fmla v20.4s, v7.4s, v3.s[0]\n"
      "ldr x22, [x26, #0x98]\n"
      "fmla v23.4s, v7.4s, v3.s[1]\n"
      "ldr x19, [x26, #0xa8]\n"
      "fmla v26.4s, v7.4s, v3.s[2]\n"
      "prfm pldl1keep, [%x[Apanel], #0x100]\n"
      "fmla v29.4s, v7.4s, v3.s[3]\n"
      "ldr d7, [x26, #0x70]\n"
      "fmla v9.4s, v4.4s, v2.s[0]\n"
      "mov v7.d[1], x24\n"
      "fmla v12.4s, v4.4s, v2.s[1]\n"
      "ldr x24, [x26, #0xb8]\n"
      "fmla v15.4s, v4.4s, v2.s[2]\n"
      "prfm pldl1keep, [x26, #0x180]\n"
      "fmla v18.4s, v4.4s, v2.s[3]\n"
      "prfm pldl1keep, [x26, #0x1c0]\n"
      "fmla v21.4s, v4.4s, v3.s[0]\n"
      "prfm pldl1keep, [%x[Apanel], #0x140]\n"
      "fmla v24.4s, v4.4s, v3.s[1]\n"
      "prfm pldl1keep, [x26, #0x200]\n"
      "fmla v27.4s, v4.4s, v3.s[2]\n"
      "sub x25, x25, #0x4\n"
      "fmla v30.4s, v4.4s, v3.s[3]\n"
      "ldr d4, [x26, #0x80]\n"
      "fmla v10.4s, v5.4s, v2.s[0]\n"
      "mov v4.d[1], x23\n"
      "fmla v13.4s, v5.4s, v2.s[1]\n"
      "cmp x25, #0x4\n"
      "fmla v16.4s, v5.4s, v2.s[2]\n"
      "fmla v19.4s, v5.4s, v2.s[3]\n"
      "ldr d2, [%x[Apanel], #0x60]\n"
      "fmla v22.4s, v5.4s, v3.s[0]\n"
      "mov v2.d[1], x21\n"
      "fmla v25.4s, v5.4s, v3.s[1]\n"
      "fmla v28.4s, v5.4s, v3.s[2]\n"
      "fmla v31.4s, v5.4s, v3.s[3]\n"
      "ldr d3, [%x[Apanel], #0x70]\n"
      "fmla v8.4s, v6.4s, v0.s[0]\n"
      "ldr d5, [x26, #0x90]\n"
      "fmla v11.4s, v6.4s, v0.s[1]\n"
      "mov v3.d[1], x20\n"
      "fmla v14.4s, v6.4s, v0.s[2]\n"
      "mov v5.d[1], x22\n"
      "fmla v17.4s, v6.4s, v0.s[3]\n"
      "add %x[Apanel], %x[Apanel], #0x80\n"
      "fmla v20.4s, v6.4s, v1.s[0]\n"
      "ldr x21, [%x[Apanel], #0x8]\n"
      "fmla v23.4s, v6.4s, v1.s[1]\n"
      "ldr x20, [%x[Apanel], #0x18]\n"
      "fmla v26.4s, v6.4s, v1.s[2]\n"
      "fmla v29.4s, v6.4s, v1.s[3]\n"
      "ldr d6, [x26, #0xa0]\n"
      "fmla v9.4s, v7.4s, v0.s[0]\n"
      "mov v6.d[1], x19\n"
      "fmla v12.4s, v7.4s, v0.s[1]\n"
      "fmla v15.4s, v7.4s, v0.s[2]\n"
      "fmla v18.4s, v7.4s, v0.s[3]\n"
      "fmla v21.4s, v7.4s, v1.s[0]\n"
      "fmla v24.4s, v7.4s, v1.s[1]\n"
      "fmla v27.4s, v7.4s, v1.s[2]\n"
      "fmla v30.4s, v7.4s, v1.s[3]\n"
      "ldr d7, [x26, #0xb0]\n"
      "fmla v10.4s, v4.4s, v0.s[0]\n"
      "add x26, x26, #0xc0\n"
      "fmla v13.4s, v4.4s, v0.s[1]\n"
      "ldr x23, [x26, #0x8]\n"
      "fmla v16.4s, v4.4s, v0.s[2]\n"
      "ldr x22, [x26, #0x18]\n"
      "fmla v19.4s, v4.4s, v0.s[3]\n"
      "ldr d0, [%x[Apanel], #0x0]\n"
      "fmla v22.4s, v4.4s, v1.s[0]\n"
      "ldr x19, [x26, #0x28]\n"
      "fmla v25.4s, v4.4s, v1.s[1]\n"
      "mov v7.d[1], x24\n"
      "fmla v28.4s, v4.4s, v1.s[2]\n"
      "mov v0.d[1], x21\n"
      "fmla v31.4s, v4.4s, v1.s[3]\n"
      "ldr d1, [%x[Apanel], #0x10]\n"
      "fmla v8.4s, v5.4s, v2.s[0]\n"
      "ldr d4, [x26, #0x0]\n"
      "fmla v11.4s, v5.4s, v2.s[1]\n"
      "mov v1.d[1], x20\n"
      "fmla v14.4s, v5.4s, v2.s[2]\n"
      "mov v4.d[1], x23\n"
      "fmla v17.4s, v5.4s, v2.s[3]\n"
      "fmla v20.4s, v5.4s, v3.s[0]\n"
      "fmla v23.4s, v5.4s, v3.s[1]\n"
      "fmla v26.4s, v5.4s, v3.s[2]\n"
      "fmla v29.4s, v5.4s, v3.s[3]\n"
      "ldr d5, [x26, #0x10]\n"
      "fmla v9.4s, v6.4s, v2.s[0]\n"
      "mov v5.d[1], x22\n"
      "fmla v12.4s, v6.4s, v2.s[1]\n"
      "fmla v15.4s, v6.4s, v2.s[2]\n"
      "fmla v18.4s, v6.4s, v2.s[3]\n"
      "fmla v21.4s, v6.4s, v3.s[0]\n"
      "fmla v24.4s, v6.4s, v3.s[1]\n"
      "fmla v27.4s, v6.4s, v3.s[2]\n"
      "fmla v30.4s, v6.4s, v3.s[3]\n"
      "ldr d6, [x26, #0x20]\n"
      "mov v6.d[1], x19\n"
      "fmla v10.4s, v7.4s, v2.s[0]\n"
      "fmla v13.4s, v7.4s, v2.s[1]\n"
      "fmla v16.4s, v7.4s, v2.s[2]\n"
      "fmla v19.4s, v7.4s, v2.s[3]\n"
      "fmla v22.4s, v7.4s, v3.s[0]\n"
      "fmla v25.4s, v7.4s, v3.s[1]\n"
      "fmla v28.4s, v7.4s, v3.s[2]\n"
      "fmla v31.4s, v7.4s, v3.s[3]\n"
      "bge 3b\n"
      "4:"  // main loop skip
      "add %x[Apanel], %x[Apanel], #0x20\n"
      "fmla v8.4s, v4.4s, v0.s[0]\n"
      "add x26, x26, #0x30\n"
      "fmla v11.4s, v4.4s, v0.s[1]\n"
      "fmla v14.4s, v4.4s, v0.s[2]\n"
      "fmla v17.4s, v4.4s, v0.s[3]\n"
      "fmla v20.4s, v4.4s, v1.s[0]\n"
      "fmla v23.4s, v4.4s, v1.s[1]\n"
      "fmla v26.4s, v4.4s, v1.s[2]\n"
      "fmla v29.4s, v4.4s, v1.s[3]\n"
      "fmla v9.4s, v5.4s, v0.s[0]\n"
      "fmla v12.4s, v5.4s, v0.s[1]\n"
      "fmla v15.4s, v5.4s, v0.s[2]\n"
      "fmla v18.4s, v5.4s, v0.s[3]\n"
      "fmla v21.4s, v5.4s, v1.s[0]\n"
      "fmla v24.4s, v5.4s, v1.s[1]\n"
      "fmla v27.4s, v5.4s, v1.s[2]\n"
      "fmla v30.4s, v5.4s, v1.s[3]\n"
      "fmla v10.4s, v6.4s, v0.s[0]\n"
      "fmla v13.4s, v6.4s, v0.s[1]\n"
      "fmla v16.4s, v6.4s, v0.s[2]\n"
      "fmla v19.4s, v6.4s, v0.s[3]\n"
      "fmla v22.4s, v6.4s, v1.s[0]\n"
      "fmla v25.4s, v6.4s, v1.s[1]\n"
      "fmla v28.4s, v6.4s, v1.s[2]\n"
      "fmla v31.4s, v6.4s, v1.s[3]\n"
      "cbz x25, 6f\n"
      "5:"  // odd loop
      "ldr q0, [%x[Apanel], #0x0]\n"
      "subs x25, x25, #0x1\n"
      "ldr q1, [%x[Apanel], #0x10]\n"
      "add %x[Apanel], %x[Apanel], #0x20\n"
      "ldr q7, [x26, #0x0]\n"
      "fmla v8.4s, v7.4s, v0.s[0]\n"
      "ldr q4, [x26, #0x10]\n"
      "fmla v11.4s, v7.4s, v0.s[1]\n"
      "ldr q5, [x26, #0x20]\n"
      "fmla v14.4s, v7.4s, v0.s[2]\n"
      "fmla v17.4s, v7.4s, v0.s[3]\n"
      "add x26, x26, #0x30\n"
      "fmla v20.4s, v7.4s, v1.s[0]\n"
      "fmla v23.4s, v7.4s, v1.s[1]\n"
      "fmla v26.4s, v7.4s, v1.s[2]\n"
      "fmla v29.4s, v7.4s, v1.s[3]\n"
      "fmla v9.4s, v4.4s, v0.s[0]\n"
      "fmla v12.4s, v4.4s, v0.s[1]\n"
      "fmla v15.4s, v4.4s, v0.s[2]\n"
      "fmla v18.4s, v4.4s, v0.s[3]\n"
      "fmla v21.4s, v4.4s, v1.s[0]\n"
      "fmla v24.4s, v4.4s, v1.s[1]\n"
      "fmla v27.4s, v4.4s, v1.s[2]\n"
      "fmla v30.4s, v4.4s, v1.s[3]\n"
      "fmla v10.4s, v5.4s, v0.s[0]\n"
      "fmla v13.4s, v5.4s, v0.s[1]\n"
      "fmla v16.4s, v5.4s, v0.s[2]\n"
      "fmla v19.4s, v5.4s, v0.s[3]\n"
      "fmla v22.4s, v5.4s, v1.s[0]\n"
      "fmla v25.4s, v5.4s, v1.s[1]\n"
      "fmla v28.4s, v5.4s, v1.s[2]\n"
      "fmla v31.4s, v5.4s, v1.s[3]\n"
      "bne 5b\n"
      "6:"  // multiply loop done
      "subs x28, x28, #0x1\n"
      "str q8, [%x[Cpanel], #0x0]\n"
      "str q9, [%x[Cpanel], #0x10]\n"
      "str q10, [%x[Cpanel], #0x20]\n"
      "str q11, [%x[Cpanel], #0x30]\n"
      "str q12, [%x[Cpanel], #0x40]\n"
      "str q13, [%x[Cpanel], #0x50]\n"
      "str q14, [%x[Cpanel], #0x60]\n"
      "str q15, [%x[Cpanel], #0x70]\n"
      "str q16, [%x[Cpanel], #0x80]\n"
      "str q17, [%x[Cpanel], #0x90]\n"
      "str q18, [%x[Cpanel], #0xa0]\n"
      "str q19, [%x[Cpanel], #0xb0]\n"
      "str q20, [%x[Cpanel], #0xc0]\n"
      "str q21, [%x[Cpanel], #0xd0]\n"
      "str q22, [%x[Cpanel], #0xe0]\n"
      "str q23, [%x[Cpanel], #0xf0]\n"
      "str q24, [%x[Cpanel], #0x100]\n"
      "str q25, [%x[Cpanel], #0x110]\n"
      "str q26, [%x[Cpanel], #0x120]\n"
      "str q27, [%x[Cpanel], #0x130]\n"
      "str q28, [%x[Cpanel], #0x140]\n"
      "str q29, [%x[Cpanel], #0x150]\n"
      "str q30, [%x[Cpanel], #0x160]\n"
      "str q31, [%x[Cpanel], #0x170]\n"
      "add %x[Cpanel], %x[Cpanel], #0x180\n"
      "bgt 2b\n"
      "subs %x[ablocks], %x[ablocks], #0x1\n"
      "bne 1b\n"
      : [Apanel] "+&r" (Apanel), [Cpanel] "+&r" (Cpanel), [ablocks] "+&r" (ablocks)
      : [args_ptr] "r" (&ka), [offsetof_Bpanel] "I" (offsetof(KernelArgs, Bpanel)), [offsetof_K] "I" (offsetof(KernelArgs, K)), [offsetof_bblocks] "I" (offsetof(KernelArgs, bblocks))
      : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28"
    );
}

} // namespace arm_gemm
#endif // __aarch64__
