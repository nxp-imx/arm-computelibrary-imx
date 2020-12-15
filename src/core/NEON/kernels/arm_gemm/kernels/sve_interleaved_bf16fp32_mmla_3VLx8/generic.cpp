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
#ifdef __ARM_FEATURE_SVE

#include "../../bfloat.hpp"
#include "../../asmlib.hpp"

namespace arm_gemm {

void sve_interleaved_bf16fp32_mmla_3VLx8(const bfloat16 *Apanel, const bfloat16 *Bpanel, float *Cpanel, int ablocks, int bblocks, int K) {
    const bfloat16 *a_ptr = Apanel;
    float *c_ptr = Cpanel;

    K /= 4;
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
                "mov z8.s, #0\n"
                "ptrue p0.h\n"
                "mov z9.s, #0\n"
                "mov z10.s, #0\n"
                "mov z11.s, #0\n"
                "ld1rqh z0.h, p0/z, [%[a_ptr]]\n"
                "mov z12.s, #0\n"
                "ld1h z4.h, p0/z, [%[b_ptr]]\n"
                "mov z13.s, #0\n"
                "ld1rqh z1.h, p0/z, [%[a_ptr], #0x10]\n"
                "mov z14.s, #0\n"
                "ld1h z5.h, p0/z, [%[b_ptr], #1, MUL VL]\n"
                "mov z15.s, #0\n"
                "ld1rqh z2.h, p0/z, [%[a_ptr], #0x20]\n"
                "mov z16.s, #0\n"
                "ld1h z6.h, p0/z, [%[b_ptr], #2, MUL VL]\n"
                "mov z17.s, #0\n"
                "add %[a_ptr], %[a_ptr], #0x40\n"
                "mov z18.s, #0\n"
                "addvl %[b_ptr], %[b_ptr], #4\n"
                "mov z19.s, #0\n"
                "mov z20.s, #0\n"
                "mov z21.s, #0\n"
                "mov z22.s, #0\n"
                "mov z23.s, #0\n"
                "mov z24.s, #0\n"
                "mov z25.s, #0\n"
                "mov z26.s, #0\n"
                "mov z27.s, #0\n"
                "mov z28.s, #0\n"
                "mov z29.s, #0\n"
                "mov z30.s, #0\n"
                "mov z31.s, #0\n"
                "cbz %[loops], 1f\n"
                "2:\n"
                ".inst 0x6464e408 // bfmmla z8.s, z0.h, z4.h\n"
                "ld1h z7.h, p0/z, [%[b_ptr], #-1, MUL VL]\n"
                ".inst 0x6464e42e // bfmmla z14.s, z1.h, z4.h\n"
                "ld1rqh z3.h, p0/z, [%[a_ptr], #-0x10]\n"
                ".inst 0x6464e454 // bfmmla z20.s, z2.h, z4.h\n"
                "subs %[loops], %[loops], #0x1\n"
                ".inst 0x6465e409 // bfmmla z9.s, z0.h, z5.h\n"
                ".inst 0x6464e47a // bfmmla z26.s, z3.h, z4.h\n"
                "ld1h z4.h, p0/z, [%[b_ptr]]\n"
                ".inst 0x6465e42f // bfmmla z15.s, z1.h, z5.h\n"
                ".inst 0x6465e455 // bfmmla z21.s, z2.h, z5.h\n"
                ".inst 0x6465e47b // bfmmla z27.s, z3.h, z5.h\n"
                "ld1h z5.h, p0/z, [%[b_ptr], #1, MUL VL]\n"
                ".inst 0x6466e40a // bfmmla z10.s, z0.h, z6.h\n"
                ".inst 0x6466e430 // bfmmla z16.s, z1.h, z6.h\n"
                ".inst 0x6466e456 // bfmmla z22.s, z2.h, z6.h\n"
                ".inst 0x6466e47c // bfmmla z28.s, z3.h, z6.h\n"
                "ld1h z6.h, p0/z, [%[b_ptr], #2, MUL VL]\n"
                ".inst 0x6467e40b // bfmmla z11.s, z0.h, z7.h\n"
                ".inst 0x6467e431 // bfmmla z17.s, z1.h, z7.h\n"
                ".inst 0x6467e457 // bfmmla z23.s, z2.h, z7.h\n"
                ".inst 0x6467e47d // bfmmla z29.s, z3.h, z7.h\n"
                "ld1h z7.h, p0/z, [%[b_ptr], #3, MUL VL]\n"
                ".inst 0x6464e40c // bfmmla z12.s, z0.h, z4.h\n"
                ".inst 0x6464e432 // bfmmla z18.s, z1.h, z4.h\n"
                ".inst 0x6464e458 // bfmmla z24.s, z2.h, z4.h\n"
                ".inst 0x6464e47e // bfmmla z30.s, z3.h, z4.h\n"
                "ld1h z4.h, p0/z, [%[b_ptr], #4, MUL VL]\n"
                ".inst 0x6465e40d // bfmmla z13.s, z0.h, z5.h\n"
                "ld1rqh z0.h, p0/z, [%[a_ptr]]\n"
                ".inst 0x6465e433 // bfmmla z19.s, z1.h, z5.h\n"
                "ld1rqh z1.h, p0/z, [%[a_ptr], #0x10]\n"
                ".inst 0x6465e459 // bfmmla z25.s, z2.h, z5.h\n"
                "ld1rqh z2.h, p0/z, [%[a_ptr], #0x20]\n"
                ".inst 0x6465e47f // bfmmla z31.s, z3.h, z5.h\n"
                "ld1h z5.h, p0/z, [%[b_ptr], #5, MUL VL]\n"
                ".inst 0x6466e408 // bfmmla z8.s, z0.h, z6.h\n"
                "ld1rqh z3.h, p0/z, [%[a_ptr], #0x30]\n"
                ".inst 0x6466e42e // bfmmla z14.s, z1.h, z6.h\n"
                "add %[a_ptr], %[a_ptr], #0x80\n"
                ".inst 0x6466e454 // bfmmla z20.s, z2.h, z6.h\n"
                "addvl %[b_ptr], %[b_ptr], #12\n"
                ".inst 0x6466e47a // bfmmla z26.s, z3.h, z6.h\n"
                ".inst 0x6467e409 // bfmmla z9.s, z0.h, z7.h\n"
                ".inst 0x6467e42f // bfmmla z15.s, z1.h, z7.h\n"
                "ld1h z6.h, p0/z, [%[b_ptr], #-6, MUL VL]\n"
                ".inst 0x6467e455 // bfmmla z21.s, z2.h, z7.h\n"
                ".inst 0x6467e47b // bfmmla z27.s, z3.h, z7.h\n"
                "ld1h z7.h, p0/z, [%[b_ptr], #-5, MUL VL]\n"
                ".inst 0x6464e40a // bfmmla z10.s, z0.h, z4.h\n"
                ".inst 0x6464e430 // bfmmla z16.s, z1.h, z4.h\n"
                ".inst 0x6464e456 // bfmmla z22.s, z2.h, z4.h\n"
                ".inst 0x6464e47c // bfmmla z28.s, z3.h, z4.h\n"
                "ld1h z4.h, p0/z, [%[b_ptr], #-4, MUL VL]\n"
                ".inst 0x6465e40b // bfmmla z11.s, z0.h, z5.h\n"
                ".inst 0x6465e431 // bfmmla z17.s, z1.h, z5.h\n"
                ".inst 0x6465e457 // bfmmla z23.s, z2.h, z5.h\n"
                ".inst 0x6465e47d // bfmmla z29.s, z3.h, z5.h\n"
                "ld1h z5.h, p0/z, [%[b_ptr], #-3, MUL VL]\n"
                ".inst 0x6466e40c // bfmmla z12.s, z0.h, z6.h\n"
                ".inst 0x6466e432 // bfmmla z18.s, z1.h, z6.h\n"
                ".inst 0x6466e458 // bfmmla z24.s, z2.h, z6.h\n"
                ".inst 0x6466e47e // bfmmla z30.s, z3.h, z6.h\n"
                "ld1h z6.h, p0/z, [%[b_ptr], #-2, MUL VL]\n"
                ".inst 0x6467e40d // bfmmla z13.s, z0.h, z7.h\n"
                "ld1rqh z0.h, p0/z, [%[a_ptr], #-0x40]\n"
                ".inst 0x6467e433 // bfmmla z19.s, z1.h, z7.h\n"
                "ld1rqh z1.h, p0/z, [%[a_ptr], #-0x30]\n"
                ".inst 0x6467e459 // bfmmla z25.s, z2.h, z7.h\n"
                "ld1rqh z2.h, p0/z, [%[a_ptr], #-0x20]\n"
                ".inst 0x6467e47f // bfmmla z31.s, z3.h, z7.h\n"
                "b.ne 2b\n"
                "1:\n"
                "cbz %[tails], 3f\n"
                ".inst 0x6464e408 // bfmmla z8.s, z0.h, z4.h\n"
                "ld1h z7.h, p0/z, [%[b_ptr], #-1, MUL VL]\n"
                ".inst 0x6464e42e // bfmmla z14.s, z1.h, z4.h\n"
                "ld1rqh z3.h, p0/z, [%[a_ptr], #-0x10]\n"
                ".inst 0x6464e454 // bfmmla z20.s, z2.h, z4.h\n"
                ".inst 0x6465e409 // bfmmla z9.s, z0.h, z5.h\n"
                ".inst 0x6465e42f // bfmmla z15.s, z1.h, z5.h\n"
                ".inst 0x6464e47a // bfmmla z26.s, z3.h, z4.h\n"
                "ld1h z4.h, p0/z, [%[b_ptr]]\n"
                ".inst 0x6465e455 // bfmmla z21.s, z2.h, z5.h\n"
                ".inst 0x6465e47b // bfmmla z27.s, z3.h, z5.h\n"
                "ld1h z5.h, p0/z, [%[b_ptr], #1, MUL VL]\n"
                ".inst 0x6466e40a // bfmmla z10.s, z0.h, z6.h\n"
                ".inst 0x6466e430 // bfmmla z16.s, z1.h, z6.h\n"
                ".inst 0x6466e456 // bfmmla z22.s, z2.h, z6.h\n"
                ".inst 0x6466e47c // bfmmla z28.s, z3.h, z6.h\n"
                "ld1h z6.h, p0/z, [%[b_ptr], #2, MUL VL]\n"
                ".inst 0x6467e40b // bfmmla z11.s, z0.h, z7.h\n"
                ".inst 0x6467e431 // bfmmla z17.s, z1.h, z7.h\n"
                ".inst 0x6467e457 // bfmmla z23.s, z2.h, z7.h\n"
                ".inst 0x6467e47d // bfmmla z29.s, z3.h, z7.h\n"
                "ld1h z7.h, p0/z, [%[b_ptr], #3, MUL VL]\n"
                ".inst 0x6464e40c // bfmmla z12.s, z0.h, z4.h\n"
                ".inst 0x6464e432 // bfmmla z18.s, z1.h, z4.h\n"
                ".inst 0x6464e458 // bfmmla z24.s, z2.h, z4.h\n"
                ".inst 0x6464e47e // bfmmla z30.s, z3.h, z4.h\n"
                "ld1h z4.h, p0/z, [%[b_ptr], #4, MUL VL]\n"
                ".inst 0x6465e40d // bfmmla z13.s, z0.h, z5.h\n"
                "ld1rqh z0.h, p0/z, [%[a_ptr]]\n"
                ".inst 0x6465e433 // bfmmla z19.s, z1.h, z5.h\n"
                "ld1rqh z1.h, p0/z, [%[a_ptr], #0x10]\n"
                ".inst 0x6465e459 // bfmmla z25.s, z2.h, z5.h\n"
                "ld1rqh z2.h, p0/z, [%[a_ptr], #0x20]\n"
                ".inst 0x6465e47f // bfmmla z31.s, z3.h, z5.h\n"
                "ld1h z5.h, p0/z, [%[b_ptr], #5, MUL VL]\n"
                ".inst 0x6466e408 // bfmmla z8.s, z0.h, z6.h\n"
                "ld1rqh z3.h, p0/z, [%[a_ptr], #0x30]\n"
                ".inst 0x6466e42e // bfmmla z14.s, z1.h, z6.h\n"
                "add %[a_ptr], %[a_ptr], #0x80\n"
                ".inst 0x6466e454 // bfmmla z20.s, z2.h, z6.h\n"
                "addvl %[b_ptr], %[b_ptr], #14\n"
                ".inst 0x6466e47a // bfmmla z26.s, z3.h, z6.h\n"
                ".inst 0x6467e409 // bfmmla z9.s, z0.h, z7.h\n"
                ".inst 0x6467e42f // bfmmla z15.s, z1.h, z7.h\n"
                "ld1h z6.h, p0/z, [%[b_ptr], #-8, MUL VL]\n"
                ".inst 0x6467e455 // bfmmla z21.s, z2.h, z7.h\n"
                ".inst 0x6467e47b // bfmmla z27.s, z3.h, z7.h\n"
                "ld1h z7.h, p0/z, [%[b_ptr], #-7, MUL VL]\n"
                ".inst 0x6464e40a // bfmmla z10.s, z0.h, z4.h\n"
                ".inst 0x6464e430 // bfmmla z16.s, z1.h, z4.h\n"
                ".inst 0x6464e456 // bfmmla z22.s, z2.h, z4.h\n"
                ".inst 0x6464e47c // bfmmla z28.s, z3.h, z4.h\n"
                "ld1h z4.h, p0/z, [%[b_ptr], #-6, MUL VL]\n"
                ".inst 0x6465e40b // bfmmla z11.s, z0.h, z5.h\n"
                ".inst 0x6465e431 // bfmmla z17.s, z1.h, z5.h\n"
                ".inst 0x6465e457 // bfmmla z23.s, z2.h, z5.h\n"
                ".inst 0x6465e47d // bfmmla z29.s, z3.h, z5.h\n"
                "ld1h z5.h, p0/z, [%[b_ptr], #-5, MUL VL]\n"
                ".inst 0x6466e40c // bfmmla z12.s, z0.h, z6.h\n"
                ".inst 0x6466e432 // bfmmla z18.s, z1.h, z6.h\n"
                ".inst 0x6466e458 // bfmmla z24.s, z2.h, z6.h\n"
                ".inst 0x6466e47e // bfmmla z30.s, z3.h, z6.h\n"
                "ld1h z6.h, p0/z, [%[b_ptr], #-4, MUL VL]\n"
                ".inst 0x6467e40d // bfmmla z13.s, z0.h, z7.h\n"
                "ld1rqh z0.h, p0/z, [%[a_ptr], #-0x40]\n"
                ".inst 0x6467e433 // bfmmla z19.s, z1.h, z7.h\n"
                "ld1rqh z1.h, p0/z, [%[a_ptr], #-0x30]\n"
                ".inst 0x6467e459 // bfmmla z25.s, z2.h, z7.h\n"
                "ld1rqh z2.h, p0/z, [%[a_ptr], #-0x20]\n"
                ".inst 0x6467e47f // bfmmla z31.s, z3.h, z7.h\n"
                "ld1h z7.h, p0/z, [%[b_ptr], #-3, MUL VL]\n"
                ".inst 0x6464e408 // bfmmla z8.s, z0.h, z4.h\n"
                "ld1rqh z3.h, p0/z, [%[a_ptr], #-0x10]\n"
                ".inst 0x6464e42e // bfmmla z14.s, z1.h, z4.h\n"
                ".inst 0x6464e454 // bfmmla z20.s, z2.h, z4.h\n"
                ".inst 0x6465e409 // bfmmla z9.s, z0.h, z5.h\n"
                ".inst 0x6464e47a // bfmmla z26.s, z3.h, z4.h\n"
                "ld1h z4.h, p0/z, [%[b_ptr], #-2, MUL VL]\n"
                ".inst 0x6465e42f // bfmmla z15.s, z1.h, z5.h\n"
                ".inst 0x6465e455 // bfmmla z21.s, z2.h, z5.h\n"
                ".inst 0x6465e47b // bfmmla z27.s, z3.h, z5.h\n"
                "ld1h z5.h, p0/z, [%[b_ptr], #-1, MUL VL]\n"
                ".inst 0x6466e40a // bfmmla z10.s, z0.h, z6.h\n"
                ".inst 0x6466e430 // bfmmla z16.s, z1.h, z6.h\n"
                ".inst 0x6466e456 // bfmmla z22.s, z2.h, z6.h\n"
                ".inst 0x6466e47c // bfmmla z28.s, z3.h, z6.h\n"
                "uzp1 z6.d, z14.d, z15.d\n"
                ".inst 0x6467e40b // bfmmla z11.s, z0.h, z7.h\n"
                ".inst 0x6467e431 // bfmmla z17.s, z1.h, z7.h\n"
                ".inst 0x6467e457 // bfmmla z23.s, z2.h, z7.h\n"
                ".inst 0x6467e47d // bfmmla z29.s, z3.h, z7.h\n"
                ".inst 0x6464e40c // bfmmla z12.s, z0.h, z4.h\n"
                "uzp1 z7.d, z16.d, z17.d\n"
                ".inst 0x6464e432 // bfmmla z18.s, z1.h, z4.h\n"
                ".inst 0x6464e458 // bfmmla z24.s, z2.h, z4.h\n"
                ".inst 0x6464e47e // bfmmla z30.s, z3.h, z4.h\n"
                "uzp2 z4.d, z10.d, z11.d\n"
                ".inst 0x6465e40d // bfmmla z13.s, z0.h, z5.h\n"
                "uzp1 z0.d, z8.d, z9.d\n"
                ".inst 0x6465e433 // bfmmla z19.s, z1.h, z5.h\n"
                "uzp1 z1.d, z10.d, z11.d\n"
                ".inst 0x6465e459 // bfmmla z25.s, z2.h, z5.h\n"
                "st1w z0.s, p0, [%[c_ptr]]\n"
                "uzp1 z2.d, z12.d, z13.d\n"
                "uzp1 z0.d, z18.d, z19.d\n"
                ".inst 0x6465e47f // bfmmla z31.s, z3.h, z5.h\n"
                "st1w z1.s, p0, [%[c_ptr], #1, MUL VL]\n"
                "uzp2 z3.d, z8.d, z9.d\n"
                "uzp2 z5.d, z12.d, z13.d\n"
                "uzp2 z1.d, z14.d, z15.d\n"
                "st1w z2.s, p0, [%[c_ptr], #2, MUL VL]\n"
                "b 4f\n"
                "3:\n"
                ".inst 0x6464e408 // bfmmla z8.s, z0.h, z4.h\n"
                "ld1h z7.h, p0/z, [%[b_ptr], #-1, MUL VL]\n"
                ".inst 0x6464e42e // bfmmla z14.s, z1.h, z4.h\n"
                "ld1rqh z3.h, p0/z, [%[a_ptr], #-0x10]\n"
                ".inst 0x6464e454 // bfmmla z20.s, z2.h, z4.h\n"
                "add %[a_ptr], %[a_ptr], #0x40\n"
                ".inst 0x6465e409 // bfmmla z9.s, z0.h, z5.h\n"
                "addvl %[b_ptr], %[b_ptr], #8\n"
                ".inst 0x6464e47a // bfmmla z26.s, z3.h, z4.h\n"
                ".inst 0x6465e42f // bfmmla z15.s, z1.h, z5.h\n"
                ".inst 0x6465e455 // bfmmla z21.s, z2.h, z5.h\n"
                "ld1h z4.h, p0/z, [%[b_ptr], #-8, MUL VL]\n"
                ".inst 0x6465e47b // bfmmla z27.s, z3.h, z5.h\n"
                "ld1h z5.h, p0/z, [%[b_ptr], #-7, MUL VL]\n"
                ".inst 0x6466e40a // bfmmla z10.s, z0.h, z6.h\n"
                ".inst 0x6466e430 // bfmmla z16.s, z1.h, z6.h\n"
                ".inst 0x6466e456 // bfmmla z22.s, z2.h, z6.h\n"
                ".inst 0x6466e47c // bfmmla z28.s, z3.h, z6.h\n"
                "ld1h z6.h, p0/z, [%[b_ptr], #-6, MUL VL]\n"
                ".inst 0x6467e40b // bfmmla z11.s, z0.h, z7.h\n"
                ".inst 0x6467e431 // bfmmla z17.s, z1.h, z7.h\n"
                ".inst 0x6467e457 // bfmmla z23.s, z2.h, z7.h\n"
                ".inst 0x6467e47d // bfmmla z29.s, z3.h, z7.h\n"
                "ld1h z7.h, p0/z, [%[b_ptr], #-5, MUL VL]\n"
                ".inst 0x6464e40c // bfmmla z12.s, z0.h, z4.h\n"
                ".inst 0x6464e432 // bfmmla z18.s, z1.h, z4.h\n"
                ".inst 0x6464e458 // bfmmla z24.s, z2.h, z4.h\n"
                ".inst 0x6464e47e // bfmmla z30.s, z3.h, z4.h\n"
                "ld1h z4.h, p0/z, [%[b_ptr], #-4, MUL VL]\n"
                ".inst 0x6465e40d // bfmmla z13.s, z0.h, z5.h\n"
                "ld1rqh z0.h, p0/z, [%[a_ptr], #-0x40]\n"
                ".inst 0x6465e433 // bfmmla z19.s, z1.h, z5.h\n"
                "ld1rqh z1.h, p0/z, [%[a_ptr], #-0x30]\n"
                ".inst 0x6465e459 // bfmmla z25.s, z2.h, z5.h\n"
                "ld1rqh z2.h, p0/z, [%[a_ptr], #-0x20]\n"
                ".inst 0x6465e47f // bfmmla z31.s, z3.h, z5.h\n"
                "ld1h z5.h, p0/z, [%[b_ptr], #-3, MUL VL]\n"
                ".inst 0x6466e408 // bfmmla z8.s, z0.h, z6.h\n"
                "ld1rqh z3.h, p0/z, [%[a_ptr], #-0x10]\n"
                ".inst 0x6466e42e // bfmmla z14.s, z1.h, z6.h\n"
                ".inst 0x6466e454 // bfmmla z20.s, z2.h, z6.h\n"
                ".inst 0x6467e409 // bfmmla z9.s, z0.h, z7.h\n"
                ".inst 0x6466e47a // bfmmla z26.s, z3.h, z6.h\n"
                "ld1h z6.h, p0/z, [%[b_ptr], #-2, MUL VL]\n"
                ".inst 0x6467e42f // bfmmla z15.s, z1.h, z7.h\n"
                ".inst 0x6467e455 // bfmmla z21.s, z2.h, z7.h\n"
                ".inst 0x6467e47b // bfmmla z27.s, z3.h, z7.h\n"
                "ld1h z7.h, p0/z, [%[b_ptr], #-1, MUL VL]\n"
                ".inst 0x6464e40a // bfmmla z10.s, z0.h, z4.h\n"
                ".inst 0x6464e430 // bfmmla z16.s, z1.h, z4.h\n"
                ".inst 0x6464e456 // bfmmla z22.s, z2.h, z4.h\n"
                ".inst 0x6464e47c // bfmmla z28.s, z3.h, z4.h\n"
                ".inst 0x6465e40b // bfmmla z11.s, z0.h, z5.h\n"
                ".inst 0x6465e431 // bfmmla z17.s, z1.h, z5.h\n"
                ".inst 0x6465e457 // bfmmla z23.s, z2.h, z5.h\n"
                ".inst 0x6465e47d // bfmmla z29.s, z3.h, z5.h\n"
                "uzp2 z4.d, z10.d, z11.d\n"
                ".inst 0x6466e40c // bfmmla z12.s, z0.h, z6.h\n"
                ".inst 0x6466e432 // bfmmla z18.s, z1.h, z6.h\n"
                ".inst 0x6466e458 // bfmmla z24.s, z2.h, z6.h\n"
                ".inst 0x6466e47e // bfmmla z30.s, z3.h, z6.h\n"
                "uzp1 z6.d, z14.d, z15.d\n"
                ".inst 0x6467e40d // bfmmla z13.s, z0.h, z7.h\n"
                "uzp1 z0.d, z8.d, z9.d\n"
                ".inst 0x6467e433 // bfmmla z19.s, z1.h, z7.h\n"
                "uzp1 z1.d, z10.d, z11.d\n"
                "uzp2 z5.d, z12.d, z13.d\n"
                "st1w z0.s, p0, [%[c_ptr]]\n"
                ".inst 0x6467e459 // bfmmla z25.s, z2.h, z7.h\n"
                "uzp1 z2.d, z12.d, z13.d\n"
                "uzp1 z0.d, z18.d, z19.d\n"
                "st1w z1.s, p0, [%[c_ptr], #1, MUL VL]\n"
                "uzp2 z1.d, z14.d, z15.d\n"
                ".inst 0x6467e47f // bfmmla z31.s, z3.h, z7.h\n"
                "uzp2 z3.d, z8.d, z9.d\n"
                "st1w z2.s, p0, [%[c_ptr], #2, MUL VL]\n"
                "uzp1 z7.d, z16.d, z17.d\n"
                "4:\n"
                "uzp2 z2.d, z16.d, z17.d\n"
                "st1w z3.s, p0, [%[c_ptr], #3, MUL VL]\n"
                "uzp2 z3.d, z18.d, z19.d\n"
                "st1w z4.s, p0, [%[c_ptr], #4, MUL VL]\n"
                "uzp1 z4.d, z20.d, z21.d\n"
                "st1w z5.s, p0, [%[c_ptr], #5, MUL VL]\n"
                "uzp1 z5.d, z22.d, z23.d\n"
                "st1w z6.s, p0, [%[c_ptr], #6, MUL VL]\n"
                "uzp1 z6.d, z24.d, z25.d\n"
                "st1w z7.s, p0, [%[c_ptr], #7, MUL VL]\n"
                "addvl %[c_ptr], %[c_ptr], #16\n"
                "uzp2 z7.d, z20.d, z21.d\n"
                "st1w z0.s, p0, [%[c_ptr], #-8, MUL VL]\n"
                "uzp2 z0.d, z22.d, z23.d\n"
                "st1w z1.s, p0, [%[c_ptr], #-7, MUL VL]\n"
                "uzp2 z1.d, z24.d, z25.d\n"
                "st1w z2.s, p0, [%[c_ptr], #-6, MUL VL]\n"
                "uzp1 z2.d, z26.d, z27.d\n"
                "st1w z3.s, p0, [%[c_ptr], #-5, MUL VL]\n"
                "uzp1 z3.d, z28.d, z29.d\n"
                "st1w z4.s, p0, [%[c_ptr], #-4, MUL VL]\n"
                "uzp1 z4.d, z30.d, z31.d\n"
                "st1w z5.s, p0, [%[c_ptr], #-3, MUL VL]\n"
                "uzp2 z5.d, z26.d, z27.d\n"
                "st1w z6.s, p0, [%[c_ptr], #-2, MUL VL]\n"
                "uzp2 z6.d, z28.d, z29.d\n"
                "st1w z7.s, p0, [%[c_ptr], #-1, MUL VL]\n"
                "uzp2 z7.d, z30.d, z31.d\n"
                "st1w z0.s, p0, [%[c_ptr]]\n"
                "st1w z1.s, p0, [%[c_ptr], #1, MUL VL]\n"
                "st1w z2.s, p0, [%[c_ptr], #2, MUL VL]\n"
                "st1w z3.s, p0, [%[c_ptr], #3, MUL VL]\n"
                "st1w z4.s, p0, [%[c_ptr], #4, MUL VL]\n"
                "st1w z5.s, p0, [%[c_ptr], #5, MUL VL]\n"
                "st1w z6.s, p0, [%[c_ptr], #6, MUL VL]\n"
                "st1w z7.s, p0, [%[c_ptr], #7, MUL VL]\n"
                "addvl %[c_ptr], %[c_ptr], #8\n"
            : [a_ptr] "+r" (a_ptr), [b_ptr] "+r" (b_ptr), [c_ptr] "+r" (c_ptr),
              [loops] "+r" (loops), [tails] "+r" (tails)
            :
            : "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31", "cc", "memory"
            );
        }
    }
}

} // namespace arm_gemm

#endif // __ARM_FEATURE_SVE