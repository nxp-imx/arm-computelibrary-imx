/*
 * Copyright (c) 2019 ARM Limited.
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
#pragma once

#ifdef __ARM_FEATURE_SVE

template<>
inline void MergeResults<3, 8, true>(uint32_t *out, const uint32_t *in, const int ldout, const int y0, const int ymax, const int x0, const int xmax, const uint32_t alpha, const uint32_t beta)
{
    const uint32_t *inptr = in;

    for (int y=y0; y<ymax; y+=8) {
        uint32_t *outptr0 = out + (y * ldout) + x0;
        uint32_t *outptr1 = outptr0 + ldout;
        uint32_t *outptr2 = outptr1 + ldout;
        uint32_t *outptr3 = outptr2 + ldout;
        uint32_t *outptr4 = outptr3 + ldout;
        uint32_t *outptr5 = outptr4 + ldout;
        uint32_t *outptr6 = outptr5 + ldout;
        uint32_t *outptr7 = outptr6 + ldout;

        const int height = ymax - y;

        for (int i=x0; i<xmax; i+=(3 * get_vector_length<uint32_t>())) {
            if (beta==0u)
            {
                switch(height) {
                case 1:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "ld1w z9.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PSTL1KEEP, [%[outptr0], #0x60]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                case 2:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "ld1w z11.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PSTL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PSTL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "st1w z9.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                case 3:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #6, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "st1w z10.s, p0, [%[outptr2]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z11.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #7, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr2], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PSTL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PSTL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x200]\n"
                            "ld1w z11.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr2], #0x60]\n"
                            "st1w z10.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "ld1w z8.s, p0/z, [x8, #-8, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "st1w z8.s, p0, [%[outptr2], #2, MUL VL]\n"
                            "addvl %[outptr2], %[outptr2], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                case 4:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #6, MUL VL]\n"
                            "ld1w z11.s, p0/z, [x8, #-7, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "st1w z10.s, p0, [%[outptr2]]\n"
                            "st1w z11.s, p0, [%[outptr3]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #7, MUL VL]\n"
                            "ld1w z11.s, p0/z, [x8, #-6, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr2], #1, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr3], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PSTL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PSTL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x200]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr2], #0x60]\n"
                            "st1w z8.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "ld1w z10.s, p0/z, [x8, #-8, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr3], #0x60]\n"
                            "st1w z9.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "ld1w z11.s, p0/z, [x8, #-5, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr2], #2, MUL VL]\n"
                            "addvl %[outptr2], %[outptr2], #3\n"
                            "st1w z11.s, p0, [%[outptr3], #2, MUL VL]\n"
                            "addvl %[outptr3], %[outptr3], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                case 5:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x240]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #6, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "ld1w z11.s, p0/z, [x8, #-7, MUL VL]\n"
                            "ld1w z8.s, p0/z, [x8, #-4, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "st1w z10.s, p0, [%[outptr2]]\n"
                            "st1w z11.s, p0, [%[outptr3]]\n"
                            "st1w z8.s, p0, [%[outptr4]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "ld1w z11.s, p0/z, [%[inptr], #7, MUL VL]\n"
                            "ld1w z8.s, p0/z, [x8, #-6, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "ld1w z9.s, p0/z, [x8, #-3, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr2], #1, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr3], #1, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr4], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PSTL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PSTL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x200]\n"
                            "ld1w z11.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr2], #0x60]\n"
                            "st1w z10.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "ld1w z8.s, p0/z, [x8, #-8, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr3], #0x60]\n"
                            "st1w z11.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "ld1w z9.s, p0/z, [x8, #-5, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr4], #0x60]\n"
                            "st1w z8.s, p0, [%[outptr2], #2, MUL VL]\n"
                            "addvl %[outptr2], %[outptr2], #3\n"
                            "ld1w z10.s, p0/z, [x8, #-2, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr3], #2, MUL VL]\n"
                            "addvl %[outptr3], %[outptr3], #3\n"
                            "st1w z10.s, p0, [%[outptr4], #2, MUL VL]\n"
                            "addvl %[outptr4], %[outptr4], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                case 6:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x240]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #6, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "ld1w z11.s, p0/z, [x8, #-7, MUL VL]\n"
                            "ld1w z8.s, p0/z, [x8, #-4, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "ld1w z9.s, p0/z, [x8, #-1, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr2]]\n"
                            "st1w z11.s, p0, [%[outptr3]]\n"
                            "st1w z8.s, p0, [%[outptr4]]\n"
                            "st1w z9.s, p0, [%[outptr5]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x280]\n"
                            "ld1w z11.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #7, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "ld1w z9.s, p0/z, [x8, #-6, MUL VL]\n"
                            "ld1w z10.s, p0/z, [x8, #-3, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "ld1w z11.s, p0/z, [x8]\n"
                            "st1w z8.s, p0, [%[outptr2], #1, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr3], #1, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr4], #1, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr5], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PSTL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PSTL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x200]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr2], #0x60]\n"
                            "st1w z8.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "ld1w z10.s, p0/z, [x8, #-8, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr3], #0x60]\n"
                            "st1w z9.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "ld1w z11.s, p0/z, [x8, #-5, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr4], #0x60]\n"
                            "st1w z10.s, p0, [%[outptr2], #2, MUL VL]\n"
                            "addvl %[outptr2], %[outptr2], #3\n"
                            "ld1w z8.s, p0/z, [x8, #-2, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr5], #0x60]\n"
                            "st1w z11.s, p0, [%[outptr3], #2, MUL VL]\n"
                            "addvl %[outptr3], %[outptr3], #3\n"
                            "ld1w z9.s, p0/z, [x8, #1, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr4], #2, MUL VL]\n"
                            "addvl %[outptr4], %[outptr4], #3\n"
                            "st1w z9.s, p0, [%[outptr5], #2, MUL VL]\n"
                            "addvl %[outptr5], %[outptr5], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                case 7:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x240]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #6, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "ld1w z11.s, p0/z, [x8, #-7, MUL VL]\n"
                            "ld1w z8.s, p0/z, [x8, #-4, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "ld1w z9.s, p0/z, [x8, #-1, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr2]]\n"
                            "ld1w z10.s, p0/z, [x8, #2, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr3]]\n"
                            "st1w z8.s, p0, [%[outptr4]]\n"
                            "st1w z9.s, p0, [%[outptr5]]\n"
                            "st1w z10.s, p0, [%[outptr6]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z11.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x280]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #7, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "ld1w z10.s, p0/z, [x8, #-6, MUL VL]\n"
                            "ld1w z11.s, p0/z, [x8, #-3, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "ld1w z8.s, p0/z, [x8]\n"
                            "st1w z9.s, p0, [%[outptr2], #1, MUL VL]\n"
                            "ld1w z9.s, p0/z, [x8, #3, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr3], #1, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr4], #1, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr5], #1, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr6], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PSTL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PSTL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x200]\n"
                            "ld1w z11.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr2], #0x60]\n"
                            "st1w z10.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "ld1w z8.s, p0/z, [x8, #-8, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr3], #0x60]\n"
                            "st1w z11.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "ld1w z9.s, p0/z, [x8, #-5, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr4], #0x60]\n"
                            "st1w z8.s, p0, [%[outptr2], #2, MUL VL]\n"
                            "addvl %[outptr2], %[outptr2], #3\n"
                            "ld1w z10.s, p0/z, [x8, #-2, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr5], #0x60]\n"
                            "st1w z9.s, p0, [%[outptr3], #2, MUL VL]\n"
                            "addvl %[outptr3], %[outptr3], #3\n"
                            "ld1w z11.s, p0/z, [x8, #1, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x2c0]\n"
                            "st1w z10.s, p0, [%[outptr4], #2, MUL VL]\n"
                            "addvl %[outptr4], %[outptr4], #3\n"
                            "ld1w z8.s, p0/z, [x8, #4, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr6], #0x60]\n"
                            "st1w z11.s, p0, [%[outptr5], #2, MUL VL]\n"
                            "addvl %[outptr5], %[outptr5], #3\n"
                            "st1w z8.s, p0, [%[outptr6], #2, MUL VL]\n"
                            "addvl %[outptr6], %[outptr6], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                default:
                case 8:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x240]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #6, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "ld1w z11.s, p0/z, [x8, #-7, MUL VL]\n"
                            "ld1w z8.s, p0/z, [x8, #-4, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "ld1w z9.s, p0/z, [x8, #-1, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr2]]\n"
                            "ld1w z10.s, p0/z, [x8, #2, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr3]]\n"
                            "ld1w z11.s, p0/z, [x8, #5, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr4]]\n"
                            "st1w z9.s, p0, [%[outptr5]]\n"
                            "st1w z10.s, p0, [%[outptr6]]\n"
                            "st1w z11.s, p0, [%[outptr7]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x280]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #7, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "ld1w z11.s, p0/z, [x8, #-6, MUL VL]\n"
                            "ld1w z8.s, p0/z, [x8, #-3, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "ld1w z9.s, p0/z, [x8]\n"
                            "st1w z10.s, p0, [%[outptr2], #1, MUL VL]\n"
                            "ld1w z10.s, p0/z, [x8, #3, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr3], #1, MUL VL]\n"
                            "ld1w z11.s, p0/z, [x8, #6, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr4], #1, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr5], #1, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr6], #1, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr7], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PSTL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PSTL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x200]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr2], #0x60]\n"
                            "st1w z8.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "ld1w z10.s, p0/z, [x8, #-8, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr3], #0x60]\n"
                            "st1w z9.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "ld1w z11.s, p0/z, [x8, #-5, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr4], #0x60]\n"
                            "st1w z10.s, p0, [%[outptr2], #2, MUL VL]\n"
                            "addvl %[outptr2], %[outptr2], #3\n"
                            "ld1w z8.s, p0/z, [x8, #-2, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr5], #0x60]\n"
                            "st1w z11.s, p0, [%[outptr3], #2, MUL VL]\n"
                            "addvl %[outptr3], %[outptr3], #3\n"
                            "ld1w z9.s, p0/z, [x8, #1, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x2c0]\n"
                            "st1w z8.s, p0, [%[outptr4], #2, MUL VL]\n"
                            "addvl %[outptr4], %[outptr4], #3\n"
                            "ld1w z10.s, p0/z, [x8, #4, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr6], #0x60]\n"
                            "st1w z9.s, p0, [%[outptr5], #2, MUL VL]\n"
                            "addvl %[outptr5], %[outptr5], #3\n"
                            "ld1w z11.s, p0/z, [x8, #7, MUL VL]\n"
                            "prfm PSTL1KEEP, [%[outptr7], #0x60]\n"
                            "st1w z10.s, p0, [%[outptr6], #2, MUL VL]\n"
                            "addvl %[outptr6], %[outptr6], #3\n"
                            "st1w z11.s, p0, [%[outptr7], #2, MUL VL]\n"
                            "addvl %[outptr7], %[outptr7], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;


                }
            }
            else
            {
                switch(height) {
                case 1:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z4.s, p0/z, [%[outptr0]]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "ld1w z5.s, p0/z, [%[outptr0], #1, MUL VL]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z9.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PLDL1KEEP, [%[outptr0], #0x60]\n"
                            "ld1w z6.s, p0/z, [%[outptr0], #2, MUL VL]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z10.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                case 2:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z4.s, p0/z, [%[outptr0]]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "ld1w z5.s, p0/z, [%[outptr1]]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z6.s, p0/z, [%[outptr0], #1, MUL VL]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr1], #1, MUL VL]\n"
                            "ld1w z11.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z10.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PLDL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PLDL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z4.s, p0/z, [%[outptr0], #2, MUL VL]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "ld1w z5.s, p0/z, [%[outptr1], #2, MUL VL]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z8.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "st1w z9.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                case 3:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z4.s, p0/z, [%[outptr0]]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "ld1w z5.s, p0/z, [%[outptr1]]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr2]]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z10.s, p0/z, [%[inptr], #6, MUL VL]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "st1w z10.s, p0, [%[outptr2]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z7.s, p0/z, [%[outptr0], #1, MUL VL]\n"
                            "ld1w z11.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "ld1w z4.s, p0/z, [%[outptr1], #1, MUL VL]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "ld1w z5.s, p0/z, [%[outptr2], #1, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "ld1w z9.s, p0/z, [%[inptr], #7, MUL VL]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z11.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr2], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PLDL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PLDL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z6.s, p0/z, [%[outptr0], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x200]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr2], #0x60]\n"
                            "ld1w z7.s, p0/z, [%[outptr1], #2, MUL VL]\n"
                            "ld1w z11.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "ld1w z4.s, p0/z, [%[outptr2], #2, MUL VL]\n"
                            "ld1w z8.s, p0/z, [x8, #-8, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z10.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "add z8.s, z8.s, z4.s\n"
                            "st1w z11.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "st1w z8.s, p0, [%[outptr2], #2, MUL VL]\n"
                            "addvl %[outptr2], %[outptr2], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                case 4:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z4.s, p0/z, [%[outptr0]]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "ld1w z5.s, p0/z, [%[outptr1]]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr2]]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z10.s, p0/z, [%[inptr], #6, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr3]]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "ld1w z11.s, p0/z, [x8, #-7, MUL VL]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "st1w z10.s, p0, [%[outptr2]]\n"
                            "st1w z11.s, p0, [%[outptr3]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z4.s, p0/z, [%[outptr0], #1, MUL VL]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "ld1w z5.s, p0/z, [%[outptr1], #1, MUL VL]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr2], #1, MUL VL]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z10.s, p0/z, [%[inptr], #7, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr3], #1, MUL VL]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "ld1w z11.s, p0/z, [x8, #-6, MUL VL]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z8.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z9.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr2], #1, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr3], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PLDL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PLDL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z4.s, p0/z, [%[outptr0], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x200]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr2], #0x60]\n"
                            "ld1w z5.s, p0/z, [%[outptr1], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr3], #0x60]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z9.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr2], #2, MUL VL]\n"
                            "ld1w z10.s, p0/z, [x8, #-8, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr3], #2, MUL VL]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z8.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "ld1w z11.s, p0/z, [x8, #-5, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z9.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z10.s, p0, [%[outptr2], #2, MUL VL]\n"
                            "addvl %[outptr2], %[outptr2], #3\n"
                            "st1w z11.s, p0, [%[outptr3], #2, MUL VL]\n"
                            "addvl %[outptr3], %[outptr3], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                case 5:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z4.s, p0/z, [%[outptr0]]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x240]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "ld1w z5.s, p0/z, [%[outptr1]]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr2]]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z10.s, p0/z, [%[inptr], #6, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr3]]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "ld1w z11.s, p0/z, [x8, #-7, MUL VL]\n"
                            "ld1w z4.s, p0/z, [%[outptr4]]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "ld1w z8.s, p0/z, [x8, #-4, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "st1w z10.s, p0, [%[outptr2]]\n"
                            "st1w z11.s, p0, [%[outptr3]]\n"
                            "st1w z8.s, p0, [%[outptr4]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z5.s, p0/z, [%[outptr0], #1, MUL VL]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr1], #1, MUL VL]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr2], #1, MUL VL]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "ld1w z11.s, p0/z, [%[inptr], #7, MUL VL]\n"
                            "ld1w z4.s, p0/z, [%[outptr3], #1, MUL VL]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "ld1w z8.s, p0/z, [x8, #-6, MUL VL]\n"
                            "ld1w z5.s, p0/z, [%[outptr4], #1, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z9.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "ld1w z9.s, p0/z, [x8, #-3, MUL VL]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "st1w z10.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z11.s, p0, [%[outptr2], #1, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr3], #1, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr4], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PLDL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PLDL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z6.s, p0/z, [%[outptr0], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x200]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr2], #0x60]\n"
                            "ld1w z7.s, p0/z, [%[outptr1], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr3], #0x60]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "ld1w z11.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "ld1w z4.s, p0/z, [%[outptr2], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr4], #0x60]\n"
                            "ld1w z8.s, p0/z, [x8, #-8, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z10.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "ld1w z5.s, p0/z, [%[outptr3], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "add z8.s, z8.s, z4.s\n"
                            "st1w z11.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "ld1w z9.s, p0/z, [x8, #-5, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "ld1w z6.s, p0/z, [%[outptr4], #2, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr2], #2, MUL VL]\n"
                            "addvl %[outptr2], %[outptr2], #3\n"
                            "add z9.s, z9.s, z5.s\n"
                            "ld1w z10.s, p0/z, [x8, #-2, MUL VL]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z9.s, p0, [%[outptr3], #2, MUL VL]\n"
                            "addvl %[outptr3], %[outptr3], #3\n"
                            "st1w z10.s, p0, [%[outptr4], #2, MUL VL]\n"
                            "addvl %[outptr4], %[outptr4], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                case 6:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z4.s, p0/z, [%[outptr0]]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x240]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "ld1w z5.s, p0/z, [%[outptr1]]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr2]]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z10.s, p0/z, [%[inptr], #6, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr3]]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "ld1w z11.s, p0/z, [x8, #-7, MUL VL]\n"
                            "ld1w z4.s, p0/z, [%[outptr4]]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "ld1w z8.s, p0/z, [x8, #-4, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "ld1w z5.s, p0/z, [%[outptr5]]\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z9.s, p0/z, [x8, #-1, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr2]]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z11.s, p0, [%[outptr3]]\n"
                            "st1w z8.s, p0, [%[outptr4]]\n"
                            "st1w z9.s, p0, [%[outptr5]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z6.s, p0/z, [%[outptr0], #1, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x280]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr1], #1, MUL VL]\n"
                            "ld1w z11.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "ld1w z4.s, p0/z, [%[outptr2], #1, MUL VL]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "ld1w z8.s, p0/z, [%[inptr], #7, MUL VL]\n"
                            "ld1w z5.s, p0/z, [%[outptr3], #1, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "ld1w z9.s, p0/z, [x8, #-6, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr4], #1, MUL VL]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "st1w z10.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "ld1w z10.s, p0/z, [x8, #-3, MUL VL]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "ld1w z7.s, p0/z, [%[outptr5], #1, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "ld1w z11.s, p0/z, [x8]\n"
                            "st1w z8.s, p0, [%[outptr2], #1, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z9.s, p0, [%[outptr3], #1, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr4], #1, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr5], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PLDL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PLDL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z4.s, p0/z, [%[outptr0], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x200]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr2], #0x60]\n"
                            "ld1w z5.s, p0/z, [%[outptr1], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr3], #0x60]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z9.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr2], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr4], #0x60]\n"
                            "ld1w z10.s, p0/z, [x8, #-8, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr5], #0x60]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z8.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr3], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z9.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "ld1w z11.s, p0/z, [x8, #-5, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "ld1w z4.s, p0/z, [%[outptr4], #2, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr2], #2, MUL VL]\n"
                            "addvl %[outptr2], %[outptr2], #3\n"
                            "add z11.s, z11.s, z7.s\n"
                            "ld1w z8.s, p0/z, [x8, #-2, MUL VL]\n"
                            "ld1w z5.s, p0/z, [%[outptr5], #2, MUL VL]\n"
                            "ld1w z9.s, p0/z, [x8, #1, MUL VL]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "st1w z11.s, p0, [%[outptr3], #2, MUL VL]\n"
                            "addvl %[outptr3], %[outptr3], #3\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z8.s, p0, [%[outptr4], #2, MUL VL]\n"
                            "addvl %[outptr4], %[outptr4], #3\n"
                            "st1w z9.s, p0, [%[outptr5], #2, MUL VL]\n"
                            "addvl %[outptr5], %[outptr5], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                case 7:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z4.s, p0/z, [%[outptr0]]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x240]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "ld1w z5.s, p0/z, [%[outptr1]]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr2]]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z10.s, p0/z, [%[inptr], #6, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr3]]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "ld1w z11.s, p0/z, [x8, #-7, MUL VL]\n"
                            "ld1w z4.s, p0/z, [%[outptr4]]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "ld1w z8.s, p0/z, [x8, #-4, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "ld1w z5.s, p0/z, [%[outptr5]]\n"
                            "ld1w z6.s, p0/z, [%[outptr6]]\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z9.s, p0/z, [x8, #-1, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr2]]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "ld1w z10.s, p0/z, [x8, #2, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr3]]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z8.s, p0, [%[outptr4]]\n"
                            "st1w z9.s, p0, [%[outptr5]]\n"
                            "st1w z10.s, p0, [%[outptr6]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z7.s, p0/z, [%[outptr0], #1, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x280]\n"
                            "ld1w z11.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "ld1w z4.s, p0/z, [%[outptr1], #1, MUL VL]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "ld1w z5.s, p0/z, [%[outptr2], #1, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "ld1w z9.s, p0/z, [%[inptr], #7, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr3], #1, MUL VL]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z10.s, p0/z, [x8, #-6, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr4], #1, MUL VL]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z11.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "ld1w z11.s, p0/z, [x8, #-3, MUL VL]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "ld1w z4.s, p0/z, [%[outptr5], #1, MUL VL]\n"
                            "ld1w z5.s, p0/z, [%[outptr6], #1, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "ld1w z8.s, p0/z, [x8]\n"
                            "st1w z9.s, p0, [%[outptr2], #1, MUL VL]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z9.s, p0/z, [x8, #3, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr3], #1, MUL VL]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z11.s, p0, [%[outptr4], #1, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr5], #1, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr6], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PLDL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PLDL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z6.s, p0/z, [%[outptr0], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x200]\n"
                            "ld1w z10.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr2], #0x60]\n"
                            "ld1w z7.s, p0/z, [%[outptr1], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr3], #0x60]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "ld1w z11.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "ld1w z4.s, p0/z, [%[outptr2], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr4], #0x60]\n"
                            "ld1w z8.s, p0/z, [x8, #-8, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr5], #0x60]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z10.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "ld1w z5.s, p0/z, [%[outptr3], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "add z8.s, z8.s, z4.s\n"
                            "st1w z11.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "ld1w z9.s, p0/z, [x8, #-5, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "ld1w z6.s, p0/z, [%[outptr4], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x2c0]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z8.s, p0, [%[outptr2], #2, MUL VL]\n"
                            "ld1w z10.s, p0/z, [x8, #-2, MUL VL]\n"
                            "addvl %[outptr2], %[outptr2], #3\n"
                            "ld1w z7.s, p0/z, [%[outptr5], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr6], #0x60]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z9.s, p0, [%[outptr3], #2, MUL VL]\n"
                            "ld1w z11.s, p0/z, [x8, #1, MUL VL]\n"
                            "addvl %[outptr3], %[outptr3], #3\n"
                            "ld1w z4.s, p0/z, [%[outptr6], #2, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr4], #2, MUL VL]\n"
                            "addvl %[outptr4], %[outptr4], #3\n"
                            "add z11.s, z11.s, z7.s\n"
                            "ld1w z8.s, p0/z, [x8, #4, MUL VL]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "st1w z11.s, p0, [%[outptr5], #2, MUL VL]\n"
                            "addvl %[outptr5], %[outptr5], #3\n"
                            "st1w z8.s, p0, [%[outptr6], #2, MUL VL]\n"
                            "addvl %[outptr6], %[outptr6], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;

                default:
                case 8:
                    {
                        long w = xmax - i;
                        long p = 0;
                        /* Optimized routine to copy an entire block */
                        __asm __volatile (
                            "mov z2.s, %s[alpha]\n"
                            "addvl x8, %[inptr], #16\n"
                            "mov z3.s, %s[beta]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x180]\n"
                            "ld1w z4.s, p0/z, [%[outptr0]]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x240]\n"
                            "ld1w z8.s, p0/z, [%[inptr]]\n"
                            "ld1w z5.s, p0/z, [%[outptr1]]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #3, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr2]]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z10.s, p0/z, [%[inptr], #6, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr3]]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "ld1w z11.s, p0/z, [x8, #-7, MUL VL]\n"
                            "ld1w z4.s, p0/z, [%[outptr4]]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z8.s, p0, [%[outptr0]]\n"
                            "ld1w z8.s, p0/z, [x8, #-4, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "ld1w z5.s, p0/z, [%[outptr5]]\n"
                            "ld1w z6.s, p0/z, [%[outptr6]]\n"
                            "st1w z9.s, p0, [%[outptr1]]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z9.s, p0/z, [x8, #-1, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr7]]\n"
                            "st1w z10.s, p0, [%[outptr2]]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "ld1w z10.s, p0/z, [x8, #2, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr3]]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "ld1w z11.s, p0/z, [x8, #5, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr4]]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z9.s, p0, [%[outptr5]]\n"
                            "st1w z10.s, p0, [%[outptr6]]\n"
                            "st1w z11.s, p0, [%[outptr7]]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "incw %[p], all, mul #1\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x1c0]\n"
                            "ld1w z4.s, p0/z, [%[outptr0], #1, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x280]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #1, MUL VL]\n"
                            "ld1w z5.s, p0/z, [%[outptr1], #1, MUL VL]\n"
                            "ld1w z9.s, p0/z, [%[inptr], #4, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr2], #1, MUL VL]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z10.s, p0/z, [%[inptr], #7, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr3], #1, MUL VL]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "ld1w z11.s, p0/z, [x8, #-6, MUL VL]\n"
                            "ld1w z4.s, p0/z, [%[outptr4], #1, MUL VL]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z8.s, p0, [%[outptr0], #1, MUL VL]\n"
                            "ld1w z8.s, p0/z, [x8, #-3, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "ld1w z5.s, p0/z, [%[outptr5], #1, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr6], #1, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr1], #1, MUL VL]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z9.s, p0/z, [x8]\n"
                            "ld1w z7.s, p0/z, [%[outptr7], #1, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr2], #1, MUL VL]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "ld1w z10.s, p0/z, [x8, #3, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr3], #1, MUL VL]\n"
                            "add z10.s, z10.s, z6.s\n"
                            "ld1w z11.s, p0/z, [x8, #6, MUL VL]\n"
                            "st1w z8.s, p0, [%[outptr4], #1, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z9.s, p0, [%[outptr5], #1, MUL VL]\n"
                            "st1w z10.s, p0, [%[outptr6], #1, MUL VL]\n"
                            "st1w z11.s, p0, [%[outptr7], #1, MUL VL]\n"
                            "whilelt p0.s, %[p], %[w]\n"
                            "b.none 1f\n"
                            "prfm PLDL1KEEP, [%[outptr0], #0x60]\n"
                            "prfm PLDL1KEEP, [%[outptr1], #0x60]\n"
                            "ld1w z4.s, p0/z, [%[outptr0], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x200]\n"
                            "ld1w z8.s, p0/z, [%[inptr], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr2], #0x60]\n"
                            "ld1w z5.s, p0/z, [%[outptr1], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr3], #0x60]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "ld1w z9.s, p0/z, [%[inptr], #5, MUL VL]\n"
                            "ld1w z6.s, p0/z, [%[outptr2], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr4], #0x60]\n"
                            "ld1w z10.s, p0/z, [x8, #-8, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr5], #0x60]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z8.s, p0, [%[outptr0], #2, MUL VL]\n"
                            "ld1w z7.s, p0/z, [%[outptr3], #2, MUL VL]\n"
                            "addvl %[outptr0], %[outptr0], #3\n"
                            "add z10.s, z10.s, z6.s\n"
                            "st1w z9.s, p0, [%[outptr1], #2, MUL VL]\n"
                            "ld1w z11.s, p0/z, [x8, #-5, MUL VL]\n"
                            "addvl %[outptr1], %[outptr1], #3\n"
                            "ld1w z4.s, p0/z, [%[outptr4], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[inptr], #0x2c0]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z10.s, p0, [%[outptr2], #2, MUL VL]\n"
                            "ld1w z8.s, p0/z, [x8, #-2, MUL VL]\n"
                            "addvl %[outptr2], %[outptr2], #3\n"
                            "ld1w z5.s, p0/z, [%[outptr5], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr6], #0x60]\n"
                            "add z8.s, z8.s, z4.s\n"
                            "st1w z11.s, p0, [%[outptr3], #2, MUL VL]\n"
                            "ld1w z9.s, p0/z, [x8, #1, MUL VL]\n"
                            "addvl %[outptr3], %[outptr3], #3\n"
                            "ld1w z6.s, p0/z, [%[outptr6], #2, MUL VL]\n"
                            "prfm PLDL1KEEP, [%[outptr7], #0x60]\n"
                            "add z9.s, z9.s, z5.s\n"
                            "st1w z8.s, p0, [%[outptr4], #2, MUL VL]\n"
                            "ld1w z10.s, p0/z, [x8, #4, MUL VL]\n"
                            "addvl %[outptr4], %[outptr4], #3\n"
                            "ld1w z7.s, p0/z, [%[outptr7], #2, MUL VL]\n"
                            "st1w z9.s, p0, [%[outptr5], #2, MUL VL]\n"
                            "addvl %[outptr5], %[outptr5], #3\n"
                            "add z10.s, z10.s, z6.s\n"
                            "ld1w z11.s, p0/z, [x8, #7, MUL VL]\n"
                            "add z11.s, z11.s, z7.s\n"
                            "st1w z10.s, p0, [%[outptr6], #2, MUL VL]\n"
                            "addvl %[outptr6], %[outptr6], #3\n"
                            "st1w z11.s, p0, [%[outptr7], #2, MUL VL]\n"
                            "addvl %[outptr7], %[outptr7], #3\n"
                            "1:\n"
                            "addvl %[inptr], %[inptr], #24\n"
                        : [outptr0] "+r" (outptr0), [outptr1] "+r" (outptr1), [outptr2] "+r" (outptr2), [outptr3] "+r" (outptr3), [outptr4] "+r" (outptr4), [outptr5] "+r" (outptr5), [outptr6] "+r" (outptr6), [outptr7] "+r" (outptr7),
                          [inptr] "+r" (inptr), [p] "+r" (p)
                        : [alpha] "w" (alpha), [beta] "w" (beta), [w] "r" (w)
                        : "x8", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "memory", "cc"
                        );
                    }
                    break;


                }
            }
        }
    }
}

#endif // __ARM_FEATURE_SVE
