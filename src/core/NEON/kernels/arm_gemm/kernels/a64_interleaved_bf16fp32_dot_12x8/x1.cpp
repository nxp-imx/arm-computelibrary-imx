/*
 * Copyright (c) 2019 Arm Limited.
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
#ifdef __aarch64__

#include "../../bfloat.hpp"
#include "../../asmlib.hpp"

namespace arm_gemm {

void a64_interleaved_bf16fp32_dot_12x8_x1(const bfloat16 *Apanel, const bfloat16 *Bpanel, float *Cpanel, int ablocks, int bblocks, int K) {
    const bfloat16 *a_ptr = Apanel;
    float *c_ptr = Cpanel;

    K /= 2;
    const long loops_count = (K / 2) - 1;
    const long tails_count = K % 2;

    for (int yb=0; yb<ablocks; yb++) {
        const bfloat16 *a_ptr0 = a_ptr;
        const bfloat16 *b_ptr = Bpanel;

        for (int xb=0; xb<bblocks; xb++) {
            a_ptr = a_ptr0;
            long loops = loops_count;
            long tails = tails_count;

            __asm __volatile (
                "movi v8.4s, #0\n"
                "ldr q0, [%[a_ptr]]\n"
                "movi v9.4s, #0\n"
                "ldr q2, [%[b_ptr]]\n"
                "movi v10.4s, #0\n"
                "ldr q1, [%[a_ptr], #0x10]\n"
                "movi v11.4s, #0\n"
                "ldr q3, [%[b_ptr], #0x10]\n"
                "movi v12.4s, #0\n"
                "ldr q4, [%[b_ptr], #0x20]\n"
                "movi v13.4s, #0\n"
                "add %[a_ptr], %[a_ptr], #0x20\n"
                "movi v14.4s, #0\n"
                "add %[b_ptr], %[b_ptr], #0x30\n"
                "movi v15.4s, #0\n"
                "movi v16.4s, #0\n"
                "movi v17.4s, #0\n"
                "movi v18.4s, #0\n"
                "movi v19.4s, #0\n"
                "movi v20.4s, #0\n"
                "movi v21.4s, #0\n"
                "movi v22.4s, #0\n"
                "movi v23.4s, #0\n"
                "movi v24.4s, #0\n"
                "movi v25.4s, #0\n"
                "movi v26.4s, #0\n"
                "movi v27.4s, #0\n"
                "movi v28.4s, #0\n"
                "movi v29.4s, #0\n"
                "movi v30.4s, #0\n"
                "movi v31.4s, #0\n"
                "cbz %[loops], 1f\n"
                "2:\n"
                ".inst 0x4f40f048 // bfdot v8.4s, v2.8h, v0.h[0]\n"
                "subs %[loops], %[loops], #0x1\n"
                ".inst 0x4f60f049 // bfdot v9.4s, v2.8h, v0.h[1]\n"
                ".inst 0x4f40f84a // bfdot v10.4s, v2.8h, v0.h[2]\n"
                ".inst 0x4f60f84b // bfdot v11.4s, v2.8h, v0.h[3]\n"
                ".inst 0x4f41f054 // bfdot v20.4s, v2.8h, v1.h[0]\n"
                ".inst 0x4f61f055 // bfdot v21.4s, v2.8h, v1.h[1]\n"
                ".inst 0x4f41f856 // bfdot v22.4s, v2.8h, v1.h[2]\n"
                ".inst 0x4f61f857 // bfdot v23.4s, v2.8h, v1.h[3]\n"
                "ldr q2, [%[b_ptr]]\n"
                ".inst 0x4f40f06c // bfdot v12.4s, v3.8h, v0.h[0]\n"
                ".inst 0x4f60f06d // bfdot v13.4s, v3.8h, v0.h[1]\n"
                ".inst 0x4f40f86e // bfdot v14.4s, v3.8h, v0.h[2]\n"
                ".inst 0x4f60f86f // bfdot v15.4s, v3.8h, v0.h[3]\n"
                ".inst 0x4f41f078 // bfdot v24.4s, v3.8h, v1.h[0]\n"
                ".inst 0x4f61f079 // bfdot v25.4s, v3.8h, v1.h[1]\n"
                ".inst 0x4f41f87a // bfdot v26.4s, v3.8h, v1.h[2]\n"
                ".inst 0x4f61f87b // bfdot v27.4s, v3.8h, v1.h[3]\n"
                "ldr q3, [%[b_ptr], #0x10]\n"
                ".inst 0x4f40f090 // bfdot v16.4s, v4.8h, v0.h[0]\n"
                ".inst 0x4f60f091 // bfdot v17.4s, v4.8h, v0.h[1]\n"
                ".inst 0x4f40f892 // bfdot v18.4s, v4.8h, v0.h[2]\n"
                ".inst 0x4f60f893 // bfdot v19.4s, v4.8h, v0.h[3]\n"
                "ldr q0, [%[a_ptr]]\n"
                ".inst 0x4f41f09c // bfdot v28.4s, v4.8h, v1.h[0]\n"
                ".inst 0x4f61f09d // bfdot v29.4s, v4.8h, v1.h[1]\n"
                ".inst 0x4f41f89e // bfdot v30.4s, v4.8h, v1.h[2]\n"
                ".inst 0x4f61f89f // bfdot v31.4s, v4.8h, v1.h[3]\n"
                "ldr q4, [%[b_ptr], #0x20]\n"
                ".inst 0x4f40f048 // bfdot v8.4s, v2.8h, v0.h[0]\n"
                "ldr q1, [%[a_ptr], #0x10]\n"
                ".inst 0x4f60f049 // bfdot v9.4s, v2.8h, v0.h[1]\n"
                "add %[a_ptr], %[a_ptr], #0x40\n"
                ".inst 0x4f40f84a // bfdot v10.4s, v2.8h, v0.h[2]\n"
                "add %[b_ptr], %[b_ptr], #0x60\n"
                ".inst 0x4f60f84b // bfdot v11.4s, v2.8h, v0.h[3]\n"
                ".inst 0x4f41f054 // bfdot v20.4s, v2.8h, v1.h[0]\n"
                ".inst 0x4f61f055 // bfdot v21.4s, v2.8h, v1.h[1]\n"
                ".inst 0x4f41f856 // bfdot v22.4s, v2.8h, v1.h[2]\n"
                ".inst 0x4f61f857 // bfdot v23.4s, v2.8h, v1.h[3]\n"
                "ldr q2, [%[b_ptr], #-0x30]\n"
                ".inst 0x4f40f06c // bfdot v12.4s, v3.8h, v0.h[0]\n"
                ".inst 0x4f60f06d // bfdot v13.4s, v3.8h, v0.h[1]\n"
                ".inst 0x4f40f86e // bfdot v14.4s, v3.8h, v0.h[2]\n"
                ".inst 0x4f60f86f // bfdot v15.4s, v3.8h, v0.h[3]\n"
                ".inst 0x4f41f078 // bfdot v24.4s, v3.8h, v1.h[0]\n"
                ".inst 0x4f61f079 // bfdot v25.4s, v3.8h, v1.h[1]\n"
                ".inst 0x4f41f87a // bfdot v26.4s, v3.8h, v1.h[2]\n"
                ".inst 0x4f61f87b // bfdot v27.4s, v3.8h, v1.h[3]\n"
                "ldr q3, [%[b_ptr], #-0x20]\n"
                ".inst 0x4f40f090 // bfdot v16.4s, v4.8h, v0.h[0]\n"
                ".inst 0x4f60f091 // bfdot v17.4s, v4.8h, v0.h[1]\n"
                ".inst 0x4f40f892 // bfdot v18.4s, v4.8h, v0.h[2]\n"
                ".inst 0x4f60f893 // bfdot v19.4s, v4.8h, v0.h[3]\n"
                "ldr q0, [%[a_ptr], #-0x20]\n"
                ".inst 0x4f41f09c // bfdot v28.4s, v4.8h, v1.h[0]\n"
                ".inst 0x4f61f09d // bfdot v29.4s, v4.8h, v1.h[1]\n"
                ".inst 0x4f41f89e // bfdot v30.4s, v4.8h, v1.h[2]\n"
                ".inst 0x4f61f89f // bfdot v31.4s, v4.8h, v1.h[3]\n"
                "ldr q4, [%[b_ptr], #-0x10]\n"
                "ldr q1, [%[a_ptr], #-0x10]\n"
                "b.ne 2b\n"
                "1:\n"
                "cbz %[tails], 3f\n"
                ".inst 0x4f40f048 // bfdot v8.4s, v2.8h, v0.h[0]\n"
                ".inst 0x4f60f049 // bfdot v9.4s, v2.8h, v0.h[1]\n"
                ".inst 0x4f40f84a // bfdot v10.4s, v2.8h, v0.h[2]\n"
                ".inst 0x4f60f84b // bfdot v11.4s, v2.8h, v0.h[3]\n"
                ".inst 0x4f41f054 // bfdot v20.4s, v2.8h, v1.h[0]\n"
                ".inst 0x4f61f055 // bfdot v21.4s, v2.8h, v1.h[1]\n"
                ".inst 0x4f41f856 // bfdot v22.4s, v2.8h, v1.h[2]\n"
                ".inst 0x4f61f857 // bfdot v23.4s, v2.8h, v1.h[3]\n"
                "ldr q2, [%[b_ptr]]\n"
                ".inst 0x4f40f06c // bfdot v12.4s, v3.8h, v0.h[0]\n"
                ".inst 0x4f60f06d // bfdot v13.4s, v3.8h, v0.h[1]\n"
                ".inst 0x4f40f86e // bfdot v14.4s, v3.8h, v0.h[2]\n"
                ".inst 0x4f60f86f // bfdot v15.4s, v3.8h, v0.h[3]\n"
                ".inst 0x4f41f078 // bfdot v24.4s, v3.8h, v1.h[0]\n"
                ".inst 0x4f61f079 // bfdot v25.4s, v3.8h, v1.h[1]\n"
                ".inst 0x4f41f87a // bfdot v26.4s, v3.8h, v1.h[2]\n"
                ".inst 0x4f61f87b // bfdot v27.4s, v3.8h, v1.h[3]\n"
                "ldr q3, [%[b_ptr], #0x10]\n"
                ".inst 0x4f40f090 // bfdot v16.4s, v4.8h, v0.h[0]\n"
                ".inst 0x4f60f091 // bfdot v17.4s, v4.8h, v0.h[1]\n"
                ".inst 0x4f40f892 // bfdot v18.4s, v4.8h, v0.h[2]\n"
                ".inst 0x4f60f893 // bfdot v19.4s, v4.8h, v0.h[3]\n"
                "ldr q0, [%[a_ptr]]\n"
                ".inst 0x4f41f09c // bfdot v28.4s, v4.8h, v1.h[0]\n"
                ".inst 0x4f61f09d // bfdot v29.4s, v4.8h, v1.h[1]\n"
                ".inst 0x4f41f89e // bfdot v30.4s, v4.8h, v1.h[2]\n"
                ".inst 0x4f61f89f // bfdot v31.4s, v4.8h, v1.h[3]\n"
                "ldr q4, [%[b_ptr], #0x20]\n"
                ".inst 0x4f40f048 // bfdot v8.4s, v2.8h, v0.h[0]\n"
                "ldr q1, [%[a_ptr], #0x10]\n"
                ".inst 0x4f60f049 // bfdot v9.4s, v2.8h, v0.h[1]\n"
                "add %[a_ptr], %[a_ptr], #0x40\n"
                ".inst 0x4f40f84a // bfdot v10.4s, v2.8h, v0.h[2]\n"
                "add %[b_ptr], %[b_ptr], #0x60\n"
                ".inst 0x4f60f84b // bfdot v11.4s, v2.8h, v0.h[3]\n"
                ".inst 0x4f41f054 // bfdot v20.4s, v2.8h, v1.h[0]\n"
                ".inst 0x4f61f055 // bfdot v21.4s, v2.8h, v1.h[1]\n"
                ".inst 0x4f41f856 // bfdot v22.4s, v2.8h, v1.h[2]\n"
                ".inst 0x4f61f857 // bfdot v23.4s, v2.8h, v1.h[3]\n"
                "ldr q2, [%[b_ptr], #-0x30]\n"
                ".inst 0x4f40f06c // bfdot v12.4s, v3.8h, v0.h[0]\n"
                ".inst 0x4f60f06d // bfdot v13.4s, v3.8h, v0.h[1]\n"
                ".inst 0x4f40f86e // bfdot v14.4s, v3.8h, v0.h[2]\n"
                ".inst 0x4f60f86f // bfdot v15.4s, v3.8h, v0.h[3]\n"
                ".inst 0x4f41f078 // bfdot v24.4s, v3.8h, v1.h[0]\n"
                ".inst 0x4f61f079 // bfdot v25.4s, v3.8h, v1.h[1]\n"
                ".inst 0x4f41f87a // bfdot v26.4s, v3.8h, v1.h[2]\n"
                ".inst 0x4f61f87b // bfdot v27.4s, v3.8h, v1.h[3]\n"
                "ldr q3, [%[b_ptr], #-0x20]\n"
                ".inst 0x4f40f090 // bfdot v16.4s, v4.8h, v0.h[0]\n"
                ".inst 0x4f60f091 // bfdot v17.4s, v4.8h, v0.h[1]\n"
                ".inst 0x4f40f892 // bfdot v18.4s, v4.8h, v0.h[2]\n"
                ".inst 0x4f60f893 // bfdot v19.4s, v4.8h, v0.h[3]\n"
                "ldr q0, [%[a_ptr], #-0x20]\n"
                ".inst 0x4f41f09c // bfdot v28.4s, v4.8h, v1.h[0]\n"
                ".inst 0x4f61f09d // bfdot v29.4s, v4.8h, v1.h[1]\n"
                ".inst 0x4f41f89e // bfdot v30.4s, v4.8h, v1.h[2]\n"
                ".inst 0x4f61f89f // bfdot v31.4s, v4.8h, v1.h[3]\n"
                "ldr q4, [%[b_ptr], #-0x10]\n"
                ".inst 0x4f40f048 // bfdot v8.4s, v2.8h, v0.h[0]\n"
                "ldr q1, [%[a_ptr], #-0x10]\n"
                ".inst 0x4f60f049 // bfdot v9.4s, v2.8h, v0.h[1]\n"
                ".inst 0x4f40f84a // bfdot v10.4s, v2.8h, v0.h[2]\n"
                ".inst 0x4f60f84b // bfdot v11.4s, v2.8h, v0.h[3]\n"
                "str q8, [%[c_ptr]]\n"
                ".inst 0x4f41f054 // bfdot v20.4s, v2.8h, v1.h[0]\n"
                ".inst 0x4f61f055 // bfdot v21.4s, v2.8h, v1.h[1]\n"
                ".inst 0x4f41f856 // bfdot v22.4s, v2.8h, v1.h[2]\n"
                ".inst 0x4f61f857 // bfdot v23.4s, v2.8h, v1.h[3]\n"
                ".inst 0x4f40f06c // bfdot v12.4s, v3.8h, v0.h[0]\n"
                ".inst 0x4f60f06d // bfdot v13.4s, v3.8h, v0.h[1]\n"
                ".inst 0x4f40f86e // bfdot v14.4s, v3.8h, v0.h[2]\n"
                ".inst 0x4f60f86f // bfdot v15.4s, v3.8h, v0.h[3]\n"
                "str q12, [%[c_ptr], #0x10]\n"
                ".inst 0x4f41f078 // bfdot v24.4s, v3.8h, v1.h[0]\n"
                ".inst 0x4f61f079 // bfdot v25.4s, v3.8h, v1.h[1]\n"
                ".inst 0x4f41f87a // bfdot v26.4s, v3.8h, v1.h[2]\n"
                ".inst 0x4f61f87b // bfdot v27.4s, v3.8h, v1.h[3]\n"
                ".inst 0x4f40f090 // bfdot v16.4s, v4.8h, v0.h[0]\n"
                ".inst 0x4f60f091 // bfdot v17.4s, v4.8h, v0.h[1]\n"
                ".inst 0x4f40f892 // bfdot v18.4s, v4.8h, v0.h[2]\n"
                ".inst 0x4f60f893 // bfdot v19.4s, v4.8h, v0.h[3]\n"
                "str q16, [%[c_ptr], #0x20]\n"
                ".inst 0x4f41f09c // bfdot v28.4s, v4.8h, v1.h[0]\n"
                ".inst 0x4f61f09d // bfdot v29.4s, v4.8h, v1.h[1]\n"
                ".inst 0x4f41f89e // bfdot v30.4s, v4.8h, v1.h[2]\n"
                "str q9, [%[c_ptr], #0x30]\n"
                ".inst 0x4f61f89f // bfdot v31.4s, v4.8h, v1.h[3]\n"
                "b 4f\n"
                "3:\n"
                ".inst 0x4f40f048 // bfdot v8.4s, v2.8h, v0.h[0]\n"
                "add %[a_ptr], %[a_ptr], #0x20\n"
                ".inst 0x4f60f049 // bfdot v9.4s, v2.8h, v0.h[1]\n"
                "add %[b_ptr], %[b_ptr], #0x30\n"
                ".inst 0x4f40f84a // bfdot v10.4s, v2.8h, v0.h[2]\n"
                ".inst 0x4f60f84b // bfdot v11.4s, v2.8h, v0.h[3]\n"
                ".inst 0x4f41f054 // bfdot v20.4s, v2.8h, v1.h[0]\n"
                ".inst 0x4f61f055 // bfdot v21.4s, v2.8h, v1.h[1]\n"
                ".inst 0x4f41f856 // bfdot v22.4s, v2.8h, v1.h[2]\n"
                ".inst 0x4f61f857 // bfdot v23.4s, v2.8h, v1.h[3]\n"
                "ldr q2, [%[b_ptr], #-0x30]\n"
                ".inst 0x4f40f06c // bfdot v12.4s, v3.8h, v0.h[0]\n"
                ".inst 0x4f60f06d // bfdot v13.4s, v3.8h, v0.h[1]\n"
                ".inst 0x4f40f86e // bfdot v14.4s, v3.8h, v0.h[2]\n"
                ".inst 0x4f60f86f // bfdot v15.4s, v3.8h, v0.h[3]\n"
                ".inst 0x4f41f078 // bfdot v24.4s, v3.8h, v1.h[0]\n"
                ".inst 0x4f61f079 // bfdot v25.4s, v3.8h, v1.h[1]\n"
                ".inst 0x4f41f87a // bfdot v26.4s, v3.8h, v1.h[2]\n"
                ".inst 0x4f61f87b // bfdot v27.4s, v3.8h, v1.h[3]\n"
                "ldr q3, [%[b_ptr], #-0x20]\n"
                ".inst 0x4f40f090 // bfdot v16.4s, v4.8h, v0.h[0]\n"
                ".inst 0x4f60f091 // bfdot v17.4s, v4.8h, v0.h[1]\n"
                ".inst 0x4f40f892 // bfdot v18.4s, v4.8h, v0.h[2]\n"
                ".inst 0x4f60f893 // bfdot v19.4s, v4.8h, v0.h[3]\n"
                "ldr q0, [%[a_ptr], #-0x20]\n"
                ".inst 0x4f41f09c // bfdot v28.4s, v4.8h, v1.h[0]\n"
                ".inst 0x4f61f09d // bfdot v29.4s, v4.8h, v1.h[1]\n"
                ".inst 0x4f41f89e // bfdot v30.4s, v4.8h, v1.h[2]\n"
                ".inst 0x4f61f89f // bfdot v31.4s, v4.8h, v1.h[3]\n"
                "ldr q4, [%[b_ptr], #-0x10]\n"
                ".inst 0x4f40f048 // bfdot v8.4s, v2.8h, v0.h[0]\n"
                "ldr q1, [%[a_ptr], #-0x10]\n"
                ".inst 0x4f60f049 // bfdot v9.4s, v2.8h, v0.h[1]\n"
                ".inst 0x4f40f84a // bfdot v10.4s, v2.8h, v0.h[2]\n"
                ".inst 0x4f60f84b // bfdot v11.4s, v2.8h, v0.h[3]\n"
                "str q8, [%[c_ptr]]\n"
                ".inst 0x4f41f054 // bfdot v20.4s, v2.8h, v1.h[0]\n"
                ".inst 0x4f61f055 // bfdot v21.4s, v2.8h, v1.h[1]\n"
                ".inst 0x4f41f856 // bfdot v22.4s, v2.8h, v1.h[2]\n"
                ".inst 0x4f61f857 // bfdot v23.4s, v2.8h, v1.h[3]\n"
                ".inst 0x4f40f06c // bfdot v12.4s, v3.8h, v0.h[0]\n"
                ".inst 0x4f60f06d // bfdot v13.4s, v3.8h, v0.h[1]\n"
                ".inst 0x4f40f86e // bfdot v14.4s, v3.8h, v0.h[2]\n"
                ".inst 0x4f60f86f // bfdot v15.4s, v3.8h, v0.h[3]\n"
                "str q12, [%[c_ptr], #0x10]\n"
                ".inst 0x4f41f078 // bfdot v24.4s, v3.8h, v1.h[0]\n"
                ".inst 0x4f61f079 // bfdot v25.4s, v3.8h, v1.h[1]\n"
                ".inst 0x4f41f87a // bfdot v26.4s, v3.8h, v1.h[2]\n"
                ".inst 0x4f61f87b // bfdot v27.4s, v3.8h, v1.h[3]\n"
                ".inst 0x4f40f090 // bfdot v16.4s, v4.8h, v0.h[0]\n"
                ".inst 0x4f60f091 // bfdot v17.4s, v4.8h, v0.h[1]\n"
                ".inst 0x4f40f892 // bfdot v18.4s, v4.8h, v0.h[2]\n"
                ".inst 0x4f60f893 // bfdot v19.4s, v4.8h, v0.h[3]\n"
                "str q16, [%[c_ptr], #0x20]\n"
                ".inst 0x4f41f09c // bfdot v28.4s, v4.8h, v1.h[0]\n"
                ".inst 0x4f61f09d // bfdot v29.4s, v4.8h, v1.h[1]\n"
                ".inst 0x4f41f89e // bfdot v30.4s, v4.8h, v1.h[2]\n"
                "str q9, [%[c_ptr], #0x30]\n"
                ".inst 0x4f61f89f // bfdot v31.4s, v4.8h, v1.h[3]\n"
                "4:\n"
                "str q13, [%[c_ptr], #0x40]\n"
                "str q17, [%[c_ptr], #0x50]\n"
                "str q10, [%[c_ptr], #0x60]\n"
                "str q14, [%[c_ptr], #0x70]\n"
                "str q18, [%[c_ptr], #0x80]\n"
                "str q11, [%[c_ptr], #0x90]\n"
                "str q15, [%[c_ptr], #0xa0]\n"
                "str q19, [%[c_ptr], #0xb0]\n"
                "str q20, [%[c_ptr], #0xc0]\n"
                "str q24, [%[c_ptr], #0xd0]\n"
                "str q28, [%[c_ptr], #0xe0]\n"
                "str q21, [%[c_ptr], #0xf0]\n"
                "str q25, [%[c_ptr], #0x100]\n"
                "str q29, [%[c_ptr], #0x110]\n"
                "str q22, [%[c_ptr], #0x120]\n"
                "str q26, [%[c_ptr], #0x130]\n"
                "str q30, [%[c_ptr], #0x140]\n"
                "str q23, [%[c_ptr], #0x150]\n"
                "str q27, [%[c_ptr], #0x160]\n"
                "str q31, [%[c_ptr], #0x170]\n"
                "add %[c_ptr], %[c_ptr], #0x180\n"
            : [a_ptr] "+r" (a_ptr), [b_ptr] "+r" (b_ptr), [c_ptr] "+r" (c_ptr),
              [loops] "+r" (loops), [tails] "+r" (tails)
            :
            : "v0", "v1", "v2", "v3", "v4", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31", "cc", "memory"
            );
        }
    }
}

} // namespace arm_gemm

#endif // __aarch64__
