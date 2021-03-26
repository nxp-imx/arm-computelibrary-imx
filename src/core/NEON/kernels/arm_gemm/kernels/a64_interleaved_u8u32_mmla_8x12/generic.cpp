/*
 * Copyright (c) 2019-2020 Arm Limited.
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

#include <cstdint>
#include "../../asmlib.hpp"

namespace arm_gemm {

void a64_interleaved_u8u32_mmla_8x12(const uint8_t *Apanel, const uint8_t *Bpanel, uint32_t *Cpanel, int ablocks, int bblocks, int K) {
    const uint8_t *a_ptr = Apanel;
    uint32_t *c_ptr = Cpanel;

    K /= 8;
    const long loops_count = (K / 2) - 1;
    const long tails_count = K % 2;

    for (int yb=0; yb<ablocks; yb++) {
        const uint8_t *a_ptr0 = a_ptr;
        const uint8_t *b_ptr = Bpanel;

        for (int xb=0; xb<bblocks; xb++) {
            a_ptr = a_ptr0;
            long loops = loops_count;
            long tails = tails_count;

            __asm __volatile (
                "movi v8.4s, #0\n"
                "ldr q0, [%[a_ptr]]\n"
                "movi v9.4s, #0\n"
                "ldr q4, [%[b_ptr]]\n"
                "movi v10.4s, #0\n"
                "ldr q1, [%[a_ptr], #0x10]\n"
                "movi v11.4s, #0\n"
                "ldr q5, [%[b_ptr], #0x10]\n"
                "movi v12.4s, #0\n"
                "ldr q2, [%[a_ptr], #0x20]\n"
                "movi v13.4s, #0\n"
                "movi v14.4s, #0\n"
                "movi v15.4s, #0\n"
                "movi v16.4s, #0\n"
                "add %[a_ptr], %[a_ptr], #0x40\n"
                "movi v17.4s, #0\n"
                "add %[b_ptr], %[b_ptr], #0x40\n"
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
                "ldr q3, [%[a_ptr], #-0x10]\n"
                ".inst 0x6e84a408 // ummla v8.4s, v0.16b, v4.16b\n"
                ".inst 0x6e85a409 // ummla v9.4s, v0.16b, v5.16b\n"

                "ldp q6, q7, [%[b_ptr], #-0x20]\n"
                ".inst 0x6e84a42e // ummla v14.4s, v1.16b, v4.16b\n"

                ".inst 0x6e85a42f // ummla v15.4s, v1.16b, v5.16b\n"
                ".inst 0x6e84a454 // ummla v20.4s, v2.16b, v4.16b\n"

                ".inst 0x6e85a455 // ummla v21.4s, v2.16b, v5.16b\n"
                "subs %[loops], %[loops], #0x1\n"

                ".inst 0x6e84a47a // ummla v26.4s, v3.16b, v4.16b\n"
                ".inst 0x6e85a47b // ummla v27.4s, v3.16b, v5.16b\n"

                "ldp q4, q5, [%[b_ptr]]\n"
                ".inst 0x6e86a40a // ummla v10.4s, v0.16b, v6.16b\n"

                ".inst 0x6e86a430 // ummla v16.4s, v1.16b, v6.16b\n"
                ".inst 0x6e86a456 // ummla v22.4s, v2.16b, v6.16b\n"

                ".inst 0x6e86a47c // ummla v28.4s, v3.16b, v6.16b\n"
                ".inst 0x6e87a40b // ummla v11.4s, v0.16b, v7.16b\n"

                ".inst 0x6e87a431 // ummla v17.4s, v1.16b, v7.16b\n"
                ".inst 0x6e87a457 // ummla v23.4s, v2.16b, v7.16b\n"

                ".inst 0x6e87a47d // ummla v29.4s, v3.16b, v7.16b\n"
                "ldp q6, q7, [%[b_ptr], #0x20]\n"

                ".inst 0x6e84a40c // ummla v12.4s, v0.16b, v4.16b\n"
                ".inst 0x6e85a40d // ummla v13.4s, v0.16b, v5.16b\n"
                "ldr q0, [%[a_ptr]]\n"

                ".inst 0x6e84a432 // ummla v18.4s, v1.16b, v4.16b\n"
                ".inst 0x6e85a433 // ummla v19.4s, v1.16b, v5.16b\n"
                "ldr q1, [%[a_ptr], #0x10]\n"

                ".inst 0x6e84a458 // ummla v24.4s, v2.16b, v4.16b\n"
                ".inst 0x6e85a459 // ummla v25.4s, v2.16b, v5.16b\n"
                "ldr q2, [%[a_ptr], #0x20]\n"

                ".inst 0x6e84a47e // ummla v30.4s, v3.16b, v4.16b\n"
                ".inst 0x6e85a47f // ummla v31.4s, v3.16b, v5.16b\n"
                "ldr q3, [%[a_ptr], #0x30]\n"

                ".inst 0x6e86a408 // ummla v8.4s, v0.16b, v6.16b\n"
                ".inst 0x6e87a409 // ummla v9.4s, v0.16b, v7.16b\n"

                "ldp q4, q5, [%[b_ptr], #0x40]\n"
                ".inst 0x6e86a42e // ummla v14.4s, v1.16b, v6.16b\n"

                ".inst 0x6e87a42f // ummla v15.4s, v1.16b, v7.16b\n"
                ".inst 0x6e86a454 // ummla v20.4s, v2.16b, v6.16b\n"

                ".inst 0x6e87a455 // ummla v21.4s, v2.16b, v7.16b\n"
                "add %[a_ptr], %[a_ptr], #0x80\n"
                ".inst 0x6e86a47a // ummla v26.4s, v3.16b, v6.16b\n"

                ".inst 0x6e87a47b // ummla v27.4s, v3.16b, v7.16b\n"
                "add %[b_ptr], %[b_ptr], #0xc0\n"
                "ldp q6, q7, [%[b_ptr], #-0x60]\n"

                ".inst 0x6e84a40a // ummla v10.4s, v0.16b, v4.16b\n"
                ".inst 0x6e84a430 // ummla v16.4s, v1.16b, v4.16b\n"

                ".inst 0x6e84a456 // ummla v22.4s, v2.16b, v4.16b\n"
                ".inst 0x6e84a47c // ummla v28.4s, v3.16b, v4.16b\n"

                ".inst 0x6e85a40b // ummla v11.4s, v0.16b, v5.16b\n"
                ".inst 0x6e85a431 // ummla v17.4s, v1.16b, v5.16b\n"

                ".inst 0x6e85a457 // ummla v23.4s, v2.16b, v5.16b\n"
                ".inst 0x6e85a47d // ummla v29.4s, v3.16b, v5.16b\n"

                "ldp q4, q5, [%[b_ptr], #-0x40]\n"
                ".inst 0x6e86a40c // ummla v12.4s, v0.16b, v6.16b\n"

                ".inst 0x6e87a40d // ummla v13.4s, v0.16b, v7.16b\n"
                "ldr q0, [%[a_ptr], #-0x40]\n"
                ".inst 0x6e86a432 // ummla v18.4s, v1.16b, v6.16b\n"

                ".inst 0x6e87a433 // ummla v19.4s, v1.16b, v7.16b\n"
                "ldr q1, [%[a_ptr], #-0x30]\n"
                ".inst 0x6e86a458 // ummla v24.4s, v2.16b, v6.16b\n"

                ".inst 0x6e87a459 // ummla v25.4s, v2.16b, v7.16b\n"
                "ldr q2, [%[a_ptr], #-0x20]\n"
                ".inst 0x6e86a47e // ummla v30.4s, v3.16b, v6.16b\n"

                ".inst 0x6e87a47f // ummla v31.4s, v3.16b, v7.16b\n"
                "b.ne 2b\n"

                "1:\n"
                "ldr q3, [%[a_ptr], #-0x10]\n"
                "cbz %[tails], 3f\n"
                ".inst 0x6e84a408 // ummla v8.4s, v0.16b, v4.16b\n"
                ".inst 0x6e84a42e // ummla v14.4s, v1.16b, v4.16b\n"
                "ldr q6, [%[b_ptr], #-0x20]\n"
                ".inst 0x6e84a454 // ummla v20.4s, v2.16b, v4.16b\n"
                ".inst 0x6e84a47a // ummla v26.4s, v3.16b, v4.16b\n"
                "ldr q7, [%[b_ptr], #-0x10]\n"
                ".inst 0x6e85a409 // ummla v9.4s, v0.16b, v5.16b\n"
                ".inst 0x6e85a42f // ummla v15.4s, v1.16b, v5.16b\n"
                "ldr q4, [%[b_ptr]]\n"
                ".inst 0x6e85a455 // ummla v21.4s, v2.16b, v5.16b\n"
                ".inst 0x6e85a47b // ummla v27.4s, v3.16b, v5.16b\n"
                "ldr q5, [%[b_ptr], #0x10]\n"
                ".inst 0x6e86a40a // ummla v10.4s, v0.16b, v6.16b\n"
                ".inst 0x6e86a430 // ummla v16.4s, v1.16b, v6.16b\n"
                ".inst 0x6e86a456 // ummla v22.4s, v2.16b, v6.16b\n"
                ".inst 0x6e86a47c // ummla v28.4s, v3.16b, v6.16b\n"
                "ldr q6, [%[b_ptr], #0x20]\n"
                ".inst 0x6e87a40b // ummla v11.4s, v0.16b, v7.16b\n"
                ".inst 0x6e87a431 // ummla v17.4s, v1.16b, v7.16b\n"
                ".inst 0x6e87a457 // ummla v23.4s, v2.16b, v7.16b\n"
                ".inst 0x6e87a47d // ummla v29.4s, v3.16b, v7.16b\n"
                "ldr q7, [%[b_ptr], #0x30]\n"
                ".inst 0x6e84a40c // ummla v12.4s, v0.16b, v4.16b\n"
                ".inst 0x6e84a432 // ummla v18.4s, v1.16b, v4.16b\n"
                ".inst 0x6e84a458 // ummla v24.4s, v2.16b, v4.16b\n"
                ".inst 0x6e84a47e // ummla v30.4s, v3.16b, v4.16b\n"
                "ldr q4, [%[b_ptr], #0x40]\n"
                ".inst 0x6e85a40d // ummla v13.4s, v0.16b, v5.16b\n"
                "ldr q0, [%[a_ptr]]\n"
                ".inst 0x6e85a433 // ummla v19.4s, v1.16b, v5.16b\n"
                "ldr q1, [%[a_ptr], #0x10]\n"
                ".inst 0x6e85a459 // ummla v25.4s, v2.16b, v5.16b\n"
                "ldr q2, [%[a_ptr], #0x20]\n"
                ".inst 0x6e85a47f // ummla v31.4s, v3.16b, v5.16b\n"
                "ldr q5, [%[b_ptr], #0x50]\n"
                ".inst 0x6e86a408 // ummla v8.4s, v0.16b, v6.16b\n"
                "ldr q3, [%[a_ptr], #0x30]\n"
                ".inst 0x6e86a42e // ummla v14.4s, v1.16b, v6.16b\n"
                "add %[a_ptr], %[a_ptr], #0x80\n"
                ".inst 0x6e86a454 // ummla v20.4s, v2.16b, v6.16b\n"
                "add %[b_ptr], %[b_ptr], #0xe0\n"
                ".inst 0x6e86a47a // ummla v26.4s, v3.16b, v6.16b\n"
                "ldr q6, [%[b_ptr], #-0x80]\n"
                ".inst 0x6e87a409 // ummla v9.4s, v0.16b, v7.16b\n"
                ".inst 0x6e87a42f // ummla v15.4s, v1.16b, v7.16b\n"
                ".inst 0x6e87a455 // ummla v21.4s, v2.16b, v7.16b\n"
                ".inst 0x6e87a47b // ummla v27.4s, v3.16b, v7.16b\n"
                "ldr q7, [%[b_ptr], #-0x70]\n"
                ".inst 0x6e84a40a // ummla v10.4s, v0.16b, v4.16b\n"
                ".inst 0x6e84a430 // ummla v16.4s, v1.16b, v4.16b\n"
                ".inst 0x6e84a456 // ummla v22.4s, v2.16b, v4.16b\n"
                ".inst 0x6e84a47c // ummla v28.4s, v3.16b, v4.16b\n"
                "ldr q4, [%[b_ptr], #-0x60]\n"
                ".inst 0x6e85a40b // ummla v11.4s, v0.16b, v5.16b\n"
                ".inst 0x6e85a431 // ummla v17.4s, v1.16b, v5.16b\n"
                ".inst 0x6e85a457 // ummla v23.4s, v2.16b, v5.16b\n"
                ".inst 0x6e85a47d // ummla v29.4s, v3.16b, v5.16b\n"
                "ldr q5, [%[b_ptr], #-0x50]\n"
                ".inst 0x6e86a40c // ummla v12.4s, v0.16b, v6.16b\n"
                ".inst 0x6e86a432 // ummla v18.4s, v1.16b, v6.16b\n"
                ".inst 0x6e86a458 // ummla v24.4s, v2.16b, v6.16b\n"
                ".inst 0x6e86a47e // ummla v30.4s, v3.16b, v6.16b\n"
                "ldr q6, [%[b_ptr], #-0x40]\n"
                ".inst 0x6e87a40d // ummla v13.4s, v0.16b, v7.16b\n"
                "ldr q0, [%[a_ptr], #-0x40]\n"
                ".inst 0x6e87a433 // ummla v19.4s, v1.16b, v7.16b\n"
                "ldr q1, [%[a_ptr], #-0x30]\n"
                ".inst 0x6e87a459 // ummla v25.4s, v2.16b, v7.16b\n"
                "ldr q2, [%[a_ptr], #-0x20]\n"
                ".inst 0x6e87a47f // ummla v31.4s, v3.16b, v7.16b\n"
                "ldr q7, [%[b_ptr], #-0x30]\n"
                ".inst 0x6e84a408 // ummla v8.4s, v0.16b, v4.16b\n"
                "ldr q3, [%[a_ptr], #-0x10]\n"
                ".inst 0x6e84a42e // ummla v14.4s, v1.16b, v4.16b\n"
                ".inst 0x6e84a454 // ummla v20.4s, v2.16b, v4.16b\n"
                ".inst 0x6e85a409 // ummla v9.4s, v0.16b, v5.16b\n"
                ".inst 0x6e84a47a // ummla v26.4s, v3.16b, v4.16b\n"
                "ldr q4, [%[b_ptr], #-0x20]\n"
                ".inst 0x6e85a42f // ummla v15.4s, v1.16b, v5.16b\n"
                ".inst 0x6e85a455 // ummla v21.4s, v2.16b, v5.16b\n"
                ".inst 0x6e85a47b // ummla v27.4s, v3.16b, v5.16b\n"
                "ldr q5, [%[b_ptr], #-0x10]\n"
                ".inst 0x6e86a40a // ummla v10.4s, v0.16b, v6.16b\n"
                ".inst 0x6e86a430 // ummla v16.4s, v1.16b, v6.16b\n"
                ".inst 0x6e86a456 // ummla v22.4s, v2.16b, v6.16b\n"
                ".inst 0x6e86a47c // ummla v28.4s, v3.16b, v6.16b\n"
                "uzp1 v6.2d, v14.2d, v15.2d\n"
                ".inst 0x6e87a40b // ummla v11.4s, v0.16b, v7.16b\n"
                ".inst 0x6e87a431 // ummla v17.4s, v1.16b, v7.16b\n"
                ".inst 0x6e87a457 // ummla v23.4s, v2.16b, v7.16b\n"
                ".inst 0x6e87a47d // ummla v29.4s, v3.16b, v7.16b\n"
                ".inst 0x6e84a40c // ummla v12.4s, v0.16b, v4.16b\n"
                "uzp1 v7.2d, v16.2d, v17.2d\n"
                ".inst 0x6e84a432 // ummla v18.4s, v1.16b, v4.16b\n"
                ".inst 0x6e84a458 // ummla v24.4s, v2.16b, v4.16b\n"
                ".inst 0x6e84a47e // ummla v30.4s, v3.16b, v4.16b\n"
                "uzp2 v4.2d, v10.2d, v11.2d\n"
                ".inst 0x6e85a40d // ummla v13.4s, v0.16b, v5.16b\n"
                "uzp1 v0.2d, v8.2d, v9.2d\n"
                ".inst 0x6e85a433 // ummla v19.4s, v1.16b, v5.16b\n"
                "uzp1 v1.2d, v10.2d, v11.2d\n"
                ".inst 0x6e85a459 // ummla v25.4s, v2.16b, v5.16b\n"
                "str q0, [%[c_ptr]]\n"
                "uzp1 v2.2d, v12.2d, v13.2d\n"
                "uzp1 v0.2d, v18.2d, v19.2d\n"
                ".inst 0x6e85a47f // ummla v31.4s, v3.16b, v5.16b\n"
                "str q1, [%[c_ptr], #0x10]\n"
                "uzp2 v3.2d, v8.2d, v9.2d\n"
                "uzp2 v5.2d, v12.2d, v13.2d\n"
                "uzp2 v1.2d, v14.2d, v15.2d\n"
                "str q2, [%[c_ptr], #0x20]\n"
                "b 4f\n"
                "3:\n"
                "ldr q6, [%[b_ptr], #-0x20]\n"
                "ldr q7, [%[b_ptr], #-0x10]\n"
                ".inst 0x6e84a408 // ummla v8.4s, v0.16b, v4.16b\n"
                "add %[a_ptr], %[a_ptr], #0x40\n"
                ".inst 0x6e84a42e // ummla v14.4s, v1.16b, v4.16b\n"
                "add %[b_ptr], %[b_ptr], #0x80\n"
                ".inst 0x6e84a454 // ummla v20.4s, v2.16b, v4.16b\n"
                ".inst 0x6e84a47a // ummla v26.4s, v3.16b, v4.16b\n"
                "ldr q4, [%[b_ptr], #-0x80]\n"
                ".inst 0x6e85a409 // ummla v9.4s, v0.16b, v5.16b\n"
                ".inst 0x6e85a42f // ummla v15.4s, v1.16b, v5.16b\n"
                ".inst 0x6e85a455 // ummla v21.4s, v2.16b, v5.16b\n"
                ".inst 0x6e85a47b // ummla v27.4s, v3.16b, v5.16b\n"
                "ldr q5, [%[b_ptr], #-0x70]\n"
                ".inst 0x6e86a40a // ummla v10.4s, v0.16b, v6.16b\n"
                ".inst 0x6e86a430 // ummla v16.4s, v1.16b, v6.16b\n"
                ".inst 0x6e86a456 // ummla v22.4s, v2.16b, v6.16b\n"
                ".inst 0x6e86a47c // ummla v28.4s, v3.16b, v6.16b\n"
                "ldr q6, [%[b_ptr], #-0x60]\n"
                ".inst 0x6e87a40b // ummla v11.4s, v0.16b, v7.16b\n"
                ".inst 0x6e87a431 // ummla v17.4s, v1.16b, v7.16b\n"
                ".inst 0x6e87a457 // ummla v23.4s, v2.16b, v7.16b\n"
                ".inst 0x6e87a47d // ummla v29.4s, v3.16b, v7.16b\n"
                "ldr q7, [%[b_ptr], #-0x50]\n"
                ".inst 0x6e84a40c // ummla v12.4s, v0.16b, v4.16b\n"
                ".inst 0x6e84a432 // ummla v18.4s, v1.16b, v4.16b\n"
                ".inst 0x6e84a458 // ummla v24.4s, v2.16b, v4.16b\n"
                ".inst 0x6e84a47e // ummla v30.4s, v3.16b, v4.16b\n"
                "ldr q4, [%[b_ptr], #-0x40]\n"
                ".inst 0x6e85a40d // ummla v13.4s, v0.16b, v5.16b\n"
                "ldr q0, [%[a_ptr], #-0x40]\n"
                ".inst 0x6e85a433 // ummla v19.4s, v1.16b, v5.16b\n"
                "ldr q1, [%[a_ptr], #-0x30]\n"
                ".inst 0x6e85a459 // ummla v25.4s, v2.16b, v5.16b\n"
                "ldr q2, [%[a_ptr], #-0x20]\n"
                ".inst 0x6e85a47f // ummla v31.4s, v3.16b, v5.16b\n"
                "ldr q5, [%[b_ptr], #-0x30]\n"
                ".inst 0x6e86a408 // ummla v8.4s, v0.16b, v6.16b\n"
                "ldr q3, [%[a_ptr], #-0x10]\n"
                ".inst 0x6e86a42e // ummla v14.4s, v1.16b, v6.16b\n"
                ".inst 0x6e86a454 // ummla v20.4s, v2.16b, v6.16b\n"
                ".inst 0x6e87a409 // ummla v9.4s, v0.16b, v7.16b\n"
                ".inst 0x6e86a47a // ummla v26.4s, v3.16b, v6.16b\n"
                "ldr q6, [%[b_ptr], #-0x20]\n"
                ".inst 0x6e87a42f // ummla v15.4s, v1.16b, v7.16b\n"
                ".inst 0x6e87a455 // ummla v21.4s, v2.16b, v7.16b\n"
                ".inst 0x6e87a47b // ummla v27.4s, v3.16b, v7.16b\n"
                "ldr q7, [%[b_ptr], #-0x10]\n"
                ".inst 0x6e84a40a // ummla v10.4s, v0.16b, v4.16b\n"
                ".inst 0x6e84a430 // ummla v16.4s, v1.16b, v4.16b\n"
                ".inst 0x6e84a456 // ummla v22.4s, v2.16b, v4.16b\n"
                ".inst 0x6e84a47c // ummla v28.4s, v3.16b, v4.16b\n"
                ".inst 0x6e85a40b // ummla v11.4s, v0.16b, v5.16b\n"
                ".inst 0x6e85a431 // ummla v17.4s, v1.16b, v5.16b\n"
                ".inst 0x6e85a457 // ummla v23.4s, v2.16b, v5.16b\n"
                ".inst 0x6e85a47d // ummla v29.4s, v3.16b, v5.16b\n"
                "uzp2 v4.2d, v10.2d, v11.2d\n"
                ".inst 0x6e86a40c // ummla v12.4s, v0.16b, v6.16b\n"
                ".inst 0x6e86a432 // ummla v18.4s, v1.16b, v6.16b\n"
                ".inst 0x6e86a458 // ummla v24.4s, v2.16b, v6.16b\n"
                ".inst 0x6e86a47e // ummla v30.4s, v3.16b, v6.16b\n"
                "uzp1 v6.2d, v14.2d, v15.2d\n"
                ".inst 0x6e87a40d // ummla v13.4s, v0.16b, v7.16b\n"
                "uzp1 v0.2d, v8.2d, v9.2d\n"
                ".inst 0x6e87a433 // ummla v19.4s, v1.16b, v7.16b\n"
                "uzp1 v1.2d, v10.2d, v11.2d\n"
                "uzp2 v5.2d, v12.2d, v13.2d\n"
                "str q0, [%[c_ptr]]\n"
                ".inst 0x6e87a459 // ummla v25.4s, v2.16b, v7.16b\n"
                "uzp1 v2.2d, v12.2d, v13.2d\n"
                "uzp1 v0.2d, v18.2d, v19.2d\n"
                "str q1, [%[c_ptr], #0x10]\n"
                "uzp2 v1.2d, v14.2d, v15.2d\n"
                ".inst 0x6e87a47f // ummla v31.4s, v3.16b, v7.16b\n"
                "uzp2 v3.2d, v8.2d, v9.2d\n"
                "str q2, [%[c_ptr], #0x20]\n"
                "uzp1 v7.2d, v16.2d, v17.2d\n"
                "4:\n"
                "uzp2 v2.2d, v16.2d, v17.2d\n"
                "str q3, [%[c_ptr], #0x30]\n"
                "uzp2 v3.2d, v18.2d, v19.2d\n"
                "str q4, [%[c_ptr], #0x40]\n"
                "uzp1 v4.2d, v20.2d, v21.2d\n"
                "str q5, [%[c_ptr], #0x50]\n"
                "uzp1 v5.2d, v22.2d, v23.2d\n"
                "str q6, [%[c_ptr], #0x60]\n"
                "uzp1 v6.2d, v24.2d, v25.2d\n"
                "str q7, [%[c_ptr], #0x70]\n"
                "uzp2 v7.2d, v20.2d, v21.2d\n"
                "str q0, [%[c_ptr], #0x80]\n"
                "uzp2 v0.2d, v22.2d, v23.2d\n"
                "str q1, [%[c_ptr], #0x90]\n"
                "uzp2 v1.2d, v24.2d, v25.2d\n"
                "str q2, [%[c_ptr], #0xa0]\n"
                "uzp1 v2.2d, v26.2d, v27.2d\n"
                "str q3, [%[c_ptr], #0xb0]\n"
                "uzp1 v3.2d, v28.2d, v29.2d\n"
                "str q4, [%[c_ptr], #0xc0]\n"
                "uzp1 v4.2d, v30.2d, v31.2d\n"
                "str q5, [%[c_ptr], #0xd0]\n"
                "uzp2 v5.2d, v26.2d, v27.2d\n"
                "str q6, [%[c_ptr], #0xe0]\n"
                "uzp2 v6.2d, v28.2d, v29.2d\n"
                "str q7, [%[c_ptr], #0xf0]\n"
                "uzp2 v7.2d, v30.2d, v31.2d\n"
                "str q0, [%[c_ptr], #0x100]\n"
                "str q1, [%[c_ptr], #0x110]\n"
                "str q2, [%[c_ptr], #0x120]\n"
                "str q3, [%[c_ptr], #0x130]\n"
                "str q4, [%[c_ptr], #0x140]\n"
                "str q5, [%[c_ptr], #0x150]\n"
                "str q6, [%[c_ptr], #0x160]\n"
                "str q7, [%[c_ptr], #0x170]\n"
                "add %[c_ptr], %[c_ptr], #0x180\n"
            : [a_ptr] "+r" (a_ptr), [b_ptr] "+r" (b_ptr), [c_ptr] "+r" (c_ptr),
              [loops] "+r" (loops), [tails] "+r" (tails)
            :
            : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31", "cc", "memory"
            );
        }
    }
}

} // namespace arm_gemm

#endif // __aarch64__