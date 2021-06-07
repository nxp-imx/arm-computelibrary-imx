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
#ifdef ARM_COMPUTE_ENABLE_SVE

#include <cstdint>
#include "../../asmlib.hpp"

namespace arm_gemm {

void sve_interleaved_s8s32_mmla_8x3VL(const int8_t *Apanel, const int8_t *Bpanel, int32_t *Cpanel, int ablocks, int bblocks, int K) {
    const int8_t *a_ptr = Apanel;
    int32_t *c_ptr = Cpanel;

    K /= 8;
    const long loops_count = (K / 2) - 1;
    const long tails_count = K % 2;

    for (int yb=0; yb<ablocks; yb++) {
        const int8_t *a_ptr0 = a_ptr;
        const int8_t *b_ptr = Bpanel;

        for (int xb=0; xb<bblocks; xb++) {
            a_ptr = a_ptr0;
            long loops = loops_count;
            long tails = tails_count;

            __asm __volatile (
                "mov z8.s, #0\n"
                "ptrue p0.b\n"
                "mov z9.s, #0\n"
                "mov z10.s, #0\n"
                "mov z11.s, #0\n"
                "ld1rqb z0.b, p0/z, [%[a_ptr]]\n"
                "mov z12.s, #0\n"
                "ld1b z4.b, p0/z, [%[b_ptr]]\n"
                "mov z13.s, #0\n"
                "ld1rqb z1.b, p0/z, [%[a_ptr], #0x10]\n"
                "mov z14.s, #0\n"
                "ld1b z5.b, p0/z, [%[b_ptr], #1, MUL VL]\n"
                "mov z15.s, #0\n"
                "ld1rqb z2.b, p0/z, [%[a_ptr], #0x20]\n"
                "mov z16.s, #0\n"
                "ld1b z6.b, p0/z, [%[b_ptr], #2, MUL VL]\n"
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
                ".inst 0x45049808 // smmla z8.s, z0.b, z4.b\n"
                "ld1b z7.b, p0/z, [%[b_ptr], #-1, MUL VL]\n"
                ".inst 0x4504982e // smmla z14.s, z1.b, z4.b\n"
                "ld1rqb z3.b, p0/z, [%[a_ptr], #-0x10]\n"
                ".inst 0x45049854 // smmla z20.s, z2.b, z4.b\n"
                "subs %[loops], %[loops], #0x1\n"
                ".inst 0x45059809 // smmla z9.s, z0.b, z5.b\n"
                ".inst 0x4504987a // smmla z26.s, z3.b, z4.b\n"
                "ld1b z4.b, p0/z, [%[b_ptr]]\n"
                ".inst 0x4505982f // smmla z15.s, z1.b, z5.b\n"
                ".inst 0x45059855 // smmla z21.s, z2.b, z5.b\n"
                ".inst 0x4505987b // smmla z27.s, z3.b, z5.b\n"
                "ld1b z5.b, p0/z, [%[b_ptr], #1, MUL VL]\n"
                ".inst 0x4506980a // smmla z10.s, z0.b, z6.b\n"
                ".inst 0x45069830 // smmla z16.s, z1.b, z6.b\n"
                ".inst 0x45069856 // smmla z22.s, z2.b, z6.b\n"
                ".inst 0x4506987c // smmla z28.s, z3.b, z6.b\n"
                "ld1b z6.b, p0/z, [%[b_ptr], #2, MUL VL]\n"
                ".inst 0x4507980b // smmla z11.s, z0.b, z7.b\n"
                ".inst 0x45079831 // smmla z17.s, z1.b, z7.b\n"
                ".inst 0x45079857 // smmla z23.s, z2.b, z7.b\n"
                ".inst 0x4507987d // smmla z29.s, z3.b, z7.b\n"
                "ld1b z7.b, p0/z, [%[b_ptr], #3, MUL VL]\n"
                ".inst 0x4504980c // smmla z12.s, z0.b, z4.b\n"
                ".inst 0x45049832 // smmla z18.s, z1.b, z4.b\n"
                ".inst 0x45049858 // smmla z24.s, z2.b, z4.b\n"
                ".inst 0x4504987e // smmla z30.s, z3.b, z4.b\n"
                "ld1b z4.b, p0/z, [%[b_ptr], #4, MUL VL]\n"
                ".inst 0x4505980d // smmla z13.s, z0.b, z5.b\n"
                "ld1rqb z0.b, p0/z, [%[a_ptr]]\n"
                ".inst 0x45059833 // smmla z19.s, z1.b, z5.b\n"
                "ld1rqb z1.b, p0/z, [%[a_ptr], #0x10]\n"
                ".inst 0x45059859 // smmla z25.s, z2.b, z5.b\n"
                "ld1rqb z2.b, p0/z, [%[a_ptr], #0x20]\n"
                ".inst 0x4505987f // smmla z31.s, z3.b, z5.b\n"
                "ld1b z5.b, p0/z, [%[b_ptr], #5, MUL VL]\n"
                ".inst 0x45069808 // smmla z8.s, z0.b, z6.b\n"
                "ld1rqb z3.b, p0/z, [%[a_ptr], #0x30]\n"
                ".inst 0x4506982e // smmla z14.s, z1.b, z6.b\n"
                "add %[a_ptr], %[a_ptr], #0x80\n"
                ".inst 0x45069854 // smmla z20.s, z2.b, z6.b\n"
                "addvl %[b_ptr], %[b_ptr], #12\n"
                ".inst 0x4506987a // smmla z26.s, z3.b, z6.b\n"
                ".inst 0x45079809 // smmla z9.s, z0.b, z7.b\n"
                ".inst 0x4507982f // smmla z15.s, z1.b, z7.b\n"
                "ld1b z6.b, p0/z, [%[b_ptr], #-6, MUL VL]\n"
                ".inst 0x45079855 // smmla z21.s, z2.b, z7.b\n"
                ".inst 0x4507987b // smmla z27.s, z3.b, z7.b\n"
                "ld1b z7.b, p0/z, [%[b_ptr], #-5, MUL VL]\n"
                ".inst 0x4504980a // smmla z10.s, z0.b, z4.b\n"
                ".inst 0x45049830 // smmla z16.s, z1.b, z4.b\n"
                ".inst 0x45049856 // smmla z22.s, z2.b, z4.b\n"
                ".inst 0x4504987c // smmla z28.s, z3.b, z4.b\n"
                "ld1b z4.b, p0/z, [%[b_ptr], #-4, MUL VL]\n"
                ".inst 0x4505980b // smmla z11.s, z0.b, z5.b\n"
                ".inst 0x45059831 // smmla z17.s, z1.b, z5.b\n"
                ".inst 0x45059857 // smmla z23.s, z2.b, z5.b\n"
                ".inst 0x4505987d // smmla z29.s, z3.b, z5.b\n"
                "ld1b z5.b, p0/z, [%[b_ptr], #-3, MUL VL]\n"
                ".inst 0x4506980c // smmla z12.s, z0.b, z6.b\n"
                ".inst 0x45069832 // smmla z18.s, z1.b, z6.b\n"
                ".inst 0x45069858 // smmla z24.s, z2.b, z6.b\n"
                ".inst 0x4506987e // smmla z30.s, z3.b, z6.b\n"
                "ld1b z6.b, p0/z, [%[b_ptr], #-2, MUL VL]\n"
                ".inst 0x4507980d // smmla z13.s, z0.b, z7.b\n"
                "ld1rqb z0.b, p0/z, [%[a_ptr], #-0x40]\n"
                ".inst 0x45079833 // smmla z19.s, z1.b, z7.b\n"
                "ld1rqb z1.b, p0/z, [%[a_ptr], #-0x30]\n"
                ".inst 0x45079859 // smmla z25.s, z2.b, z7.b\n"
                "ld1rqb z2.b, p0/z, [%[a_ptr], #-0x20]\n"
                ".inst 0x4507987f // smmla z31.s, z3.b, z7.b\n"
                "b.ne 2b\n"
                "1:\n"
                "cbz %[tails], 3f\n"
                ".inst 0x45049808 // smmla z8.s, z0.b, z4.b\n"
                "ld1b z7.b, p0/z, [%[b_ptr], #-1, MUL VL]\n"
                ".inst 0x4504982e // smmla z14.s, z1.b, z4.b\n"
                "ld1rqb z3.b, p0/z, [%[a_ptr], #-0x10]\n"
                ".inst 0x45049854 // smmla z20.s, z2.b, z4.b\n"
                ".inst 0x45059809 // smmla z9.s, z0.b, z5.b\n"
                ".inst 0x4505982f // smmla z15.s, z1.b, z5.b\n"
                ".inst 0x4504987a // smmla z26.s, z3.b, z4.b\n"
                "ld1b z4.b, p0/z, [%[b_ptr]]\n"
                ".inst 0x45059855 // smmla z21.s, z2.b, z5.b\n"
                ".inst 0x4505987b // smmla z27.s, z3.b, z5.b\n"
                "ld1b z5.b, p0/z, [%[b_ptr], #1, MUL VL]\n"
                ".inst 0x4506980a // smmla z10.s, z0.b, z6.b\n"
                ".inst 0x45069830 // smmla z16.s, z1.b, z6.b\n"
                ".inst 0x45069856 // smmla z22.s, z2.b, z6.b\n"
                ".inst 0x4506987c // smmla z28.s, z3.b, z6.b\n"
                "ld1b z6.b, p0/z, [%[b_ptr], #2, MUL VL]\n"
                ".inst 0x4507980b // smmla z11.s, z0.b, z7.b\n"
                ".inst 0x45079831 // smmla z17.s, z1.b, z7.b\n"
                ".inst 0x45079857 // smmla z23.s, z2.b, z7.b\n"
                ".inst 0x4507987d // smmla z29.s, z3.b, z7.b\n"
                "ld1b z7.b, p0/z, [%[b_ptr], #3, MUL VL]\n"
                ".inst 0x4504980c // smmla z12.s, z0.b, z4.b\n"
                ".inst 0x45049832 // smmla z18.s, z1.b, z4.b\n"
                ".inst 0x45049858 // smmla z24.s, z2.b, z4.b\n"
                ".inst 0x4504987e // smmla z30.s, z3.b, z4.b\n"
                "ld1b z4.b, p0/z, [%[b_ptr], #4, MUL VL]\n"
                ".inst 0x4505980d // smmla z13.s, z0.b, z5.b\n"
                "ld1rqb z0.b, p0/z, [%[a_ptr]]\n"
                ".inst 0x45059833 // smmla z19.s, z1.b, z5.b\n"
                "ld1rqb z1.b, p0/z, [%[a_ptr], #0x10]\n"
                ".inst 0x45059859 // smmla z25.s, z2.b, z5.b\n"
                "ld1rqb z2.b, p0/z, [%[a_ptr], #0x20]\n"
                ".inst 0x4505987f // smmla z31.s, z3.b, z5.b\n"
                "ld1b z5.b, p0/z, [%[b_ptr], #5, MUL VL]\n"
                ".inst 0x45069808 // smmla z8.s, z0.b, z6.b\n"
                "ld1rqb z3.b, p0/z, [%[a_ptr], #0x30]\n"
                ".inst 0x4506982e // smmla z14.s, z1.b, z6.b\n"
                "add %[a_ptr], %[a_ptr], #0x80\n"
                ".inst 0x45069854 // smmla z20.s, z2.b, z6.b\n"
                "addvl %[b_ptr], %[b_ptr], #14\n"
                ".inst 0x4506987a // smmla z26.s, z3.b, z6.b\n"
                ".inst 0x45079809 // smmla z9.s, z0.b, z7.b\n"
                ".inst 0x4507982f // smmla z15.s, z1.b, z7.b\n"
                "ld1b z6.b, p0/z, [%[b_ptr], #-8, MUL VL]\n"
                ".inst 0x45079855 // smmla z21.s, z2.b, z7.b\n"
                ".inst 0x4507987b // smmla z27.s, z3.b, z7.b\n"
                "ld1b z7.b, p0/z, [%[b_ptr], #-7, MUL VL]\n"
                ".inst 0x4504980a // smmla z10.s, z0.b, z4.b\n"
                ".inst 0x45049830 // smmla z16.s, z1.b, z4.b\n"
                ".inst 0x45049856 // smmla z22.s, z2.b, z4.b\n"
                ".inst 0x4504987c // smmla z28.s, z3.b, z4.b\n"
                "ld1b z4.b, p0/z, [%[b_ptr], #-6, MUL VL]\n"
                ".inst 0x4505980b // smmla z11.s, z0.b, z5.b\n"
                ".inst 0x45059831 // smmla z17.s, z1.b, z5.b\n"
                ".inst 0x45059857 // smmla z23.s, z2.b, z5.b\n"
                ".inst 0x4505987d // smmla z29.s, z3.b, z5.b\n"
                "ld1b z5.b, p0/z, [%[b_ptr], #-5, MUL VL]\n"
                ".inst 0x4506980c // smmla z12.s, z0.b, z6.b\n"
                ".inst 0x45069832 // smmla z18.s, z1.b, z6.b\n"
                ".inst 0x45069858 // smmla z24.s, z2.b, z6.b\n"
                ".inst 0x4506987e // smmla z30.s, z3.b, z6.b\n"
                "ld1b z6.b, p0/z, [%[b_ptr], #-4, MUL VL]\n"
                ".inst 0x4507980d // smmla z13.s, z0.b, z7.b\n"
                "ld1rqb z0.b, p0/z, [%[a_ptr], #-0x40]\n"
                ".inst 0x45079833 // smmla z19.s, z1.b, z7.b\n"
                "ld1rqb z1.b, p0/z, [%[a_ptr], #-0x30]\n"
                ".inst 0x45079859 // smmla z25.s, z2.b, z7.b\n"
                "ld1rqb z2.b, p0/z, [%[a_ptr], #-0x20]\n"
                ".inst 0x4507987f // smmla z31.s, z3.b, z7.b\n"
                "ld1b z7.b, p0/z, [%[b_ptr], #-3, MUL VL]\n"
                ".inst 0x45049808 // smmla z8.s, z0.b, z4.b\n"
                "ld1rqb z3.b, p0/z, [%[a_ptr], #-0x10]\n"
                ".inst 0x4504982e // smmla z14.s, z1.b, z4.b\n"
                ".inst 0x45049854 // smmla z20.s, z2.b, z4.b\n"
                ".inst 0x45059809 // smmla z9.s, z0.b, z5.b\n"
                ".inst 0x4504987a // smmla z26.s, z3.b, z4.b\n"
                "ld1b z4.b, p0/z, [%[b_ptr], #-2, MUL VL]\n"
                ".inst 0x4505982f // smmla z15.s, z1.b, z5.b\n"
                ".inst 0x45059855 // smmla z21.s, z2.b, z5.b\n"
                ".inst 0x4505987b // smmla z27.s, z3.b, z5.b\n"
                "ld1b z5.b, p0/z, [%[b_ptr], #-1, MUL VL]\n"
                ".inst 0x4506980a // smmla z10.s, z0.b, z6.b\n"
                ".inst 0x45069830 // smmla z16.s, z1.b, z6.b\n"
                ".inst 0x45069856 // smmla z22.s, z2.b, z6.b\n"
                ".inst 0x4506987c // smmla z28.s, z3.b, z6.b\n"
                "uzp1 z6.d, z14.d, z15.d\n"
                ".inst 0x4507980b // smmla z11.s, z0.b, z7.b\n"
                ".inst 0x45079831 // smmla z17.s, z1.b, z7.b\n"
                ".inst 0x45079857 // smmla z23.s, z2.b, z7.b\n"
                ".inst 0x4507987d // smmla z29.s, z3.b, z7.b\n"
                ".inst 0x4504980c // smmla z12.s, z0.b, z4.b\n"
                "uzp1 z7.d, z16.d, z17.d\n"
                ".inst 0x45049832 // smmla z18.s, z1.b, z4.b\n"
                ".inst 0x45049858 // smmla z24.s, z2.b, z4.b\n"
                ".inst 0x4504987e // smmla z30.s, z3.b, z4.b\n"
                "uzp2 z4.d, z10.d, z11.d\n"
                ".inst 0x4505980d // smmla z13.s, z0.b, z5.b\n"
                "uzp1 z0.d, z8.d, z9.d\n"
                ".inst 0x45059833 // smmla z19.s, z1.b, z5.b\n"
                "uzp1 z1.d, z10.d, z11.d\n"
                ".inst 0x45059859 // smmla z25.s, z2.b, z5.b\n"
                "st1w z0.s, p0, [%[c_ptr]]\n"
                "uzp1 z2.d, z12.d, z13.d\n"
                "uzp1 z0.d, z18.d, z19.d\n"
                ".inst 0x4505987f // smmla z31.s, z3.b, z5.b\n"
                "st1w z1.s, p0, [%[c_ptr], #1, MUL VL]\n"
                "uzp2 z3.d, z8.d, z9.d\n"
                "uzp2 z5.d, z12.d, z13.d\n"
                "uzp2 z1.d, z14.d, z15.d\n"
                "st1w z2.s, p0, [%[c_ptr], #2, MUL VL]\n"
                "b 4f\n"
                "3:\n"
                ".inst 0x45049808 // smmla z8.s, z0.b, z4.b\n"
                "ld1b z7.b, p0/z, [%[b_ptr], #-1, MUL VL]\n"
                ".inst 0x4504982e // smmla z14.s, z1.b, z4.b\n"
                "ld1rqb z3.b, p0/z, [%[a_ptr], #-0x10]\n"
                ".inst 0x45049854 // smmla z20.s, z2.b, z4.b\n"
                "add %[a_ptr], %[a_ptr], #0x40\n"
                ".inst 0x45059809 // smmla z9.s, z0.b, z5.b\n"
                "addvl %[b_ptr], %[b_ptr], #8\n"
                ".inst 0x4504987a // smmla z26.s, z3.b, z4.b\n"
                ".inst 0x4505982f // smmla z15.s, z1.b, z5.b\n"
                ".inst 0x45059855 // smmla z21.s, z2.b, z5.b\n"
                "ld1b z4.b, p0/z, [%[b_ptr], #-8, MUL VL]\n"
                ".inst 0x4505987b // smmla z27.s, z3.b, z5.b\n"
                "ld1b z5.b, p0/z, [%[b_ptr], #-7, MUL VL]\n"
                ".inst 0x4506980a // smmla z10.s, z0.b, z6.b\n"
                ".inst 0x45069830 // smmla z16.s, z1.b, z6.b\n"
                ".inst 0x45069856 // smmla z22.s, z2.b, z6.b\n"
                ".inst 0x4506987c // smmla z28.s, z3.b, z6.b\n"
                "ld1b z6.b, p0/z, [%[b_ptr], #-6, MUL VL]\n"
                ".inst 0x4507980b // smmla z11.s, z0.b, z7.b\n"
                ".inst 0x45079831 // smmla z17.s, z1.b, z7.b\n"
                ".inst 0x45079857 // smmla z23.s, z2.b, z7.b\n"
                ".inst 0x4507987d // smmla z29.s, z3.b, z7.b\n"
                "ld1b z7.b, p0/z, [%[b_ptr], #-5, MUL VL]\n"
                ".inst 0x4504980c // smmla z12.s, z0.b, z4.b\n"
                ".inst 0x45049832 // smmla z18.s, z1.b, z4.b\n"
                ".inst 0x45049858 // smmla z24.s, z2.b, z4.b\n"
                ".inst 0x4504987e // smmla z30.s, z3.b, z4.b\n"
                "ld1b z4.b, p0/z, [%[b_ptr], #-4, MUL VL]\n"
                ".inst 0x4505980d // smmla z13.s, z0.b, z5.b\n"
                "ld1rqb z0.b, p0/z, [%[a_ptr], #-0x40]\n"
                ".inst 0x45059833 // smmla z19.s, z1.b, z5.b\n"
                "ld1rqb z1.b, p0/z, [%[a_ptr], #-0x30]\n"
                ".inst 0x45059859 // smmla z25.s, z2.b, z5.b\n"
                "ld1rqb z2.b, p0/z, [%[a_ptr], #-0x20]\n"
                ".inst 0x4505987f // smmla z31.s, z3.b, z5.b\n"
                "ld1b z5.b, p0/z, [%[b_ptr], #-3, MUL VL]\n"
                ".inst 0x45069808 // smmla z8.s, z0.b, z6.b\n"
                "ld1rqb z3.b, p0/z, [%[a_ptr], #-0x10]\n"
                ".inst 0x4506982e // smmla z14.s, z1.b, z6.b\n"
                ".inst 0x45069854 // smmla z20.s, z2.b, z6.b\n"
                ".inst 0x45079809 // smmla z9.s, z0.b, z7.b\n"
                ".inst 0x4506987a // smmla z26.s, z3.b, z6.b\n"
                "ld1b z6.b, p0/z, [%[b_ptr], #-2, MUL VL]\n"
                ".inst 0x4507982f // smmla z15.s, z1.b, z7.b\n"
                ".inst 0x45079855 // smmla z21.s, z2.b, z7.b\n"
                ".inst 0x4507987b // smmla z27.s, z3.b, z7.b\n"
                "ld1b z7.b, p0/z, [%[b_ptr], #-1, MUL VL]\n"
                ".inst 0x4504980a // smmla z10.s, z0.b, z4.b\n"
                ".inst 0x45049830 // smmla z16.s, z1.b, z4.b\n"
                ".inst 0x45049856 // smmla z22.s, z2.b, z4.b\n"
                ".inst 0x4504987c // smmla z28.s, z3.b, z4.b\n"
                ".inst 0x4505980b // smmla z11.s, z0.b, z5.b\n"
                ".inst 0x45059831 // smmla z17.s, z1.b, z5.b\n"
                ".inst 0x45059857 // smmla z23.s, z2.b, z5.b\n"
                ".inst 0x4505987d // smmla z29.s, z3.b, z5.b\n"
                "uzp2 z4.d, z10.d, z11.d\n"
                ".inst 0x4506980c // smmla z12.s, z0.b, z6.b\n"
                ".inst 0x45069832 // smmla z18.s, z1.b, z6.b\n"
                ".inst 0x45069858 // smmla z24.s, z2.b, z6.b\n"
                ".inst 0x4506987e // smmla z30.s, z3.b, z6.b\n"
                "uzp1 z6.d, z14.d, z15.d\n"
                ".inst 0x4507980d // smmla z13.s, z0.b, z7.b\n"
                "uzp1 z0.d, z8.d, z9.d\n"
                ".inst 0x45079833 // smmla z19.s, z1.b, z7.b\n"
                "uzp1 z1.d, z10.d, z11.d\n"
                "uzp2 z5.d, z12.d, z13.d\n"
                "st1w z0.s, p0, [%[c_ptr]]\n"
                ".inst 0x45079859 // smmla z25.s, z2.b, z7.b\n"
                "uzp1 z2.d, z12.d, z13.d\n"
                "uzp1 z0.d, z18.d, z19.d\n"
                "st1w z1.s, p0, [%[c_ptr], #1, MUL VL]\n"
                "uzp2 z1.d, z14.d, z15.d\n"
                ".inst 0x4507987f // smmla z31.s, z3.b, z7.b\n"
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

#endif // ARM_COMPUTE_ENABLE_SVE
