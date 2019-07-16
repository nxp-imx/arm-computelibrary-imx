/*
 * Copyright (c) 2017 ARM Limited.
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
#include "helpers.h"

// [AIR-1608] Original transposition not working correctly
#define TRANSPOSE_4x4(u0, u1, u2, u3) \
    do { \
        VEC_DATA_TYPE(DATA_TYPE, 4) tmp0 = u0; \
        VEC_DATA_TYPE(DATA_TYPE, 4) tmp1 = u1; \
        VEC_DATA_TYPE(DATA_TYPE, 4) tmp2 = u2; \
        VEC_DATA_TYPE(DATA_TYPE, 4) tmp3 = u3; \
        u0.s1 = tmp1.s0; \
        u0.s2 = tmp2.s0; \
        u0.s3 = tmp3.s0; \
        u1.s0 = tmp0.s1; \
        u1.s2 = tmp2.s1; \
        u1.s3 = tmp3.s1; \
        u2.s0 = tmp0.s2; \
        u2.s1 = tmp1.s2; \
        u2.s3 = tmp3.s2; \
        u3.s0 = tmp0.s3; \
        u3.s1 = tmp1.s3; \
        u3.s2 = tmp2.s3; \
    } while(0)

// [AIR-1608] Original transposition not working correctly
#define TRANSPOSE_8x8(u0, u1, u2, u3, u4, u5, u6, u7) \
    do { \
        VEC_DATA_TYPE(DATA_TYPE, 8) tmp0 = u0; \
        VEC_DATA_TYPE(DATA_TYPE, 8) tmp1 = u1; \
        VEC_DATA_TYPE(DATA_TYPE, 8) tmp2 = u2; \
        VEC_DATA_TYPE(DATA_TYPE, 8) tmp3 = u3; \
        VEC_DATA_TYPE(DATA_TYPE, 8) tmp4 = u4; \
        VEC_DATA_TYPE(DATA_TYPE, 8) tmp5 = u5; \
        VEC_DATA_TYPE(DATA_TYPE, 8) tmp6 = u6; \
        VEC_DATA_TYPE(DATA_TYPE, 8) tmp7 = u7; \
        u0.s1 = tmp1.s0; \
        u0.s2 = tmp2.s0; \
        u0.s3 = tmp3.s0; \
        u0.s4 = tmp4.s0; \
        u0.s5 = tmp5.s0; \
        u0.s6 = tmp6.s0; \
        u0.s7 = tmp7.s0; \
        u1.s0 = tmp0.s1; \
        u1.s2 = tmp2.s1; \
        u1.s3 = tmp3.s1; \
        u1.s4 = tmp4.s1; \
        u1.s5 = tmp5.s1; \
        u1.s6 = tmp6.s1; \
        u1.s7 = tmp7.s1; \
        u2.s0 = tmp0.s2; \
        u2.s1 = tmp1.s2; \
        u2.s3 = tmp3.s2; \
        u2.s4 = tmp4.s2; \
        u2.s5 = tmp5.s2; \
        u2.s6 = tmp6.s2; \
        u2.s7 = tmp7.s2; \
        u3.s0 = tmp0.s3; \
        u3.s1 = tmp1.s3; \
        u3.s2 = tmp2.s3; \
        u3.s4 = tmp4.s3; \
        u3.s5 = tmp5.s3; \
        u3.s6 = tmp6.s3; \
        u3.s7 = tmp7.s3; \
        u4.s0 = tmp0.s4; \
        u4.s1 = tmp1.s4; \
        u4.s2 = tmp2.s4; \
        u4.s3 = tmp3.s4; \
        u4.s5 = tmp5.s4; \
        u4.s6 = tmp6.s4; \
        u4.s7 = tmp7.s4; \
        u5.s0 = tmp0.s5; \
        u5.s1 = tmp1.s5; \
        u5.s2 = tmp2.s5; \
        u5.s3 = tmp3.s5; \
        u5.s4 = tmp4.s5; \
        u5.s6 = tmp6.s5; \
        u5.s7 = tmp7.s5; \
        u6.s0 = tmp0.s6; \
        u6.s1 = tmp1.s6; \
        u6.s2 = tmp2.s6; \
        u6.s3 = tmp3.s6; \
        u6.s4 = tmp4.s6; \
        u6.s5 = tmp5.s6; \
        u6.s7 = tmp7.s6; \
        u7.s0 = tmp0.s7; \
        u7.s1 = tmp1.s7; \
        u7.s2 = tmp2.s7; \
        u7.s3 = tmp3.s7; \
        u7.s4 = tmp4.s7; \
        u7.s5 = tmp5.s7; \
        u7.s6 = tmp6.s7; \
    } while(0)

// [AIR-1608] Original transposition not working correctly
#define TRANSPOSE_16x16(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11, u12, u13, u14, u15) \
    do { \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp0 = u0; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp1 = u1; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp2 = u2; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp3 = u3; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp4 = u4; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp5 = u5; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp6 = u6; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp7 = u7; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp8 = u8; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp9 = u9; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp10 = u10; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp11 = u11; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp12 = u12; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp13 = u13; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp14 = u14; \
        VEC_DATA_TYPE(DATA_TYPE, 16) tmp15 = u15; \
        u0.s1 = tmp1.s0; \
        u0.s2 = tmp2.s0; \
        u0.s3 = tmp3.s0; \
        u0.s4 = tmp4.s0; \
        u0.s5 = tmp5.s0; \
        u0.s6 = tmp6.s0; \
        u0.s7 = tmp7.s0; \
        u0.s8 = tmp8.s0; \
        u0.s9 = tmp9.s0; \
        u0.sa = tmp10.s0; \
        u0.sb = tmp11.s0; \
        u0.sc = tmp12.s0; \
        u0.sd = tmp13.s0; \
        u0.se = tmp14.s0; \
        u0.sf = tmp15.s0; \
        u1.s0 = tmp0.s1; \
        u1.s2 = tmp2.s1; \
        u1.s3 = tmp3.s1; \
        u1.s4 = tmp4.s1; \
        u1.s5 = tmp5.s1; \
        u1.s6 = tmp6.s1; \
        u1.s7 = tmp7.s1; \
        u1.s8 = tmp8.s1; \
        u1.s9 = tmp9.s1; \
        u1.sa = tmp10.s1; \
        u1.sb = tmp11.s1; \
        u1.sc = tmp12.s1; \
        u1.sd = tmp13.s1; \
        u1.se = tmp14.s1; \
        u1.sf = tmp15.s1; \
        u2.s0 = tmp0.s2; \
        u2.s1 = tmp1.s2; \
        u2.s3 = tmp3.s2; \
        u2.s4 = tmp4.s2; \
        u2.s5 = tmp5.s2; \
        u2.s6 = tmp6.s2; \
        u2.s7 = tmp7.s2; \
        u2.s8 = tmp8.s2; \
        u2.s9 = tmp9.s2; \
        u2.sa = tmp10.s2; \
        u2.sb = tmp11.s2; \
        u2.sc = tmp12.s2; \
        u2.sd = tmp13.s2; \
        u2.se = tmp14.s2; \
        u2.sf = tmp15.s2; \
        u3.s0 = tmp0.s3; \
        u3.s1 = tmp1.s3; \
        u3.s2 = tmp2.s3; \
        u3.s4 = tmp4.s3; \
        u3.s5 = tmp5.s3; \
        u3.s6 = tmp6.s3; \
        u3.s7 = tmp7.s3; \
        u3.s8 = tmp8.s3; \
        u3.s9 = tmp9.s3; \
        u3.sa = tmp10.s3; \
        u3.sb = tmp11.s3; \
        u3.sc = tmp12.s3; \
        u3.sd = tmp13.s3; \
        u3.se = tmp14.s3; \
        u3.sf = tmp15.s3; \
        u4.s0 = tmp0.s4; \
        u4.s1 = tmp1.s4; \
        u4.s2 = tmp2.s4; \
        u4.s3 = tmp3.s4; \
        u4.s5 = tmp5.s4; \
        u4.s6 = tmp6.s4; \
        u4.s7 = tmp7.s4; \
        u4.s8 = tmp8.s4; \
        u4.s9 = tmp9.s4; \
        u4.sa = tmp10.s4; \
        u4.sb = tmp11.s4; \
        u4.sc = tmp12.s4; \
        u4.sd = tmp13.s4; \
        u4.se = tmp14.s4; \
        u4.sf = tmp15.s4; \
        u5.s0 = tmp0.s5; \
        u5.s1 = tmp1.s5; \
        u5.s2 = tmp2.s5; \
        u5.s3 = tmp3.s5; \
        u5.s4 = tmp4.s5; \
        u5.s6 = tmp6.s5; \
        u5.s7 = tmp7.s5; \
        u5.s8 = tmp8.s5; \
        u5.s9 = tmp9.s5; \
        u5.sa = tmp10.s5; \
        u5.sb = tmp11.s5; \
        u5.sc = tmp12.s5; \
        u5.sd = tmp13.s5; \
        u5.se = tmp14.s5; \
        u5.sf = tmp15.s5; \
        u6.s0 = tmp0.s6; \
        u6.s1 = tmp1.s6; \
        u6.s2 = tmp2.s6; \
        u6.s3 = tmp3.s6; \
        u6.s4 = tmp4.s6; \
        u6.s5 = tmp5.s6; \
        u6.s7 = tmp7.s6; \
        u6.s8 = tmp8.s6; \
        u6.s9 = tmp9.s6; \
        u6.sa = tmp10.s6; \
        u6.sb = tmp11.s6; \
        u6.sc = tmp12.s6; \
        u6.sd = tmp13.s6; \
        u6.se = tmp14.s6; \
        u6.sf = tmp15.s6; \
        u7.s0 = tmp0.s7; \
        u7.s1 = tmp1.s7; \
        u7.s2 = tmp2.s7; \
        u7.s3 = tmp3.s7; \
        u7.s4 = tmp4.s7; \
        u7.s5 = tmp5.s7; \
        u7.s6 = tmp6.s7; \
        u7.s8 = tmp8.s7; \
        u7.s9 = tmp9.s7; \
        u7.sa = tmp10.s7; \
        u7.sb = tmp11.s7; \
        u7.sc = tmp12.s7; \
        u7.sd = tmp13.s7; \
        u7.se = tmp14.s7; \
        u7.sf = tmp15.s7; \
        u8.s0 = tmp0.s8; \
        u8.s1 = tmp1.s8; \
        u8.s2 = tmp2.s8; \
        u8.s3 = tmp3.s8; \
        u8.s4 = tmp4.s8; \
        u8.s5 = tmp5.s8; \
        u8.s6 = tmp6.s8; \
        u8.s7 = tmp7.s8; \
        u8.s9 = tmp9.s8; \
        u8.sa = tmp10.s8; \
        u8.sb = tmp11.s8; \
        u8.sc = tmp12.s8; \
        u8.sd = tmp13.s8; \
        u8.se = tmp14.s8; \
        u8.sf = tmp15.s8; \
        u9.s0 = tmp0.s9; \
        u9.s1 = tmp1.s9; \
        u9.s2 = tmp2.s9; \
        u9.s3 = tmp3.s9; \
        u9.s4 = tmp4.s9; \
        u9.s5 = tmp5.s9; \
        u9.s6 = tmp6.s9; \
        u9.s7 = tmp7.s9; \
        u9.s8 = tmp8.s9; \
        u9.sa = tmp10.s9; \
        u9.sb = tmp11.s9; \
        u9.sc = tmp12.s9; \
        u9.sd = tmp13.s9; \
        u9.se = tmp14.s9; \
        u9.sf = tmp15.s9; \
        u10.s0 = tmp0.sa; \
        u10.s1 = tmp1.sa; \
        u10.s2 = tmp2.sa; \
        u10.s3 = tmp3.sa; \
        u10.s4 = tmp4.sa; \
        u10.s5 = tmp5.sa; \
        u10.s6 = tmp6.sa; \
        u10.s7 = tmp7.sa; \
        u10.s8 = tmp8.sa; \
        u10.s9 = tmp9.sa; \
        u10.sb = tmp11.sa; \
        u10.sc = tmp12.sa; \
        u10.sd = tmp13.sa; \
        u10.se = tmp14.sa; \
        u10.sf = tmp15.sa; \
        u11.s0 = tmp0.sb; \
        u11.s1 = tmp1.sb; \
        u11.s2 = tmp2.sb; \
        u11.s3 = tmp3.sb; \
        u11.s4 = tmp4.sb; \
        u11.s5 = tmp5.sb; \
        u11.s6 = tmp6.sb; \
        u11.s7 = tmp7.sb; \
        u11.s8 = tmp8.sb; \
        u11.s9 = tmp9.sb; \
        u11.sa = tmp10.sb; \
        u11.sc = tmp12.sb; \
        u11.sd = tmp13.sb; \
        u11.se = tmp14.sb; \
        u11.sf = tmp15.sb; \
        u12.s0 = tmp0.sc; \
        u12.s1 = tmp1.sc; \
        u12.s2 = tmp2.sc; \
        u12.s3 = tmp3.sc; \
        u12.s4 = tmp4.sc; \
        u12.s5 = tmp5.sc; \
        u12.s6 = tmp6.sc; \
        u12.s7 = tmp7.sc; \
        u12.s8 = tmp8.sc; \
        u12.s9 = tmp9.sc; \
        u12.sa = tmp10.sc; \
        u12.sb = tmp11.sc; \
        u12.sd = tmp13.sc; \
        u12.se = tmp14.sc; \
        u12.sf = tmp15.sc; \
        u13.s0 = tmp0.sd; \
        u13.s1 = tmp1.sd; \
        u13.s2 = tmp2.sd; \
        u13.s3 = tmp3.sd; \
        u13.s4 = tmp4.sd; \
        u13.s5 = tmp5.sd; \
        u13.s6 = tmp6.sd; \
        u13.s7 = tmp7.sd; \
        u13.s8 = tmp8.sd; \
        u13.s9 = tmp9.sd; \
        u13.sa = tmp10.sd; \
        u13.sb = tmp11.sd; \
        u13.sc = tmp12.sd; \
        u13.se = tmp14.sd; \
        u13.sf = tmp15.sd; \
        u14.s0 = tmp0.se; \
        u14.s1 = tmp1.se; \
        u14.s2 = tmp2.se; \
        u14.s3 = tmp3.se; \
        u14.s4 = tmp4.se; \
        u14.s5 = tmp5.se; \
        u14.s6 = tmp6.se; \
        u14.s7 = tmp7.se; \
        u14.s8 = tmp8.se; \
        u14.s9 = tmp9.se; \
        u14.sa = tmp10.se; \
        u14.sb = tmp11.se; \
        u14.sc = tmp12.se; \
        u14.sd = tmp13.se; \
        u14.sf = tmp15.se; \
        u15.s0 = tmp0.sf; \
        u15.s1 = tmp1.sf; \
        u15.s2 = tmp2.sf; \
        u15.s3 = tmp3.sf; \
        u15.s4 = tmp4.sf; \
        u15.s5 = tmp5.sf; \
        u15.s6 = tmp6.sf; \
        u15.s7 = tmp7.sf; \
        u15.s8 = tmp8.sf; \
        u15.s9 = tmp9.sf; \
        u15.sa = tmp10.sf; \
        u15.sb = tmp11.sf; \
        u15.sc = tmp12.sf; \
        u15.sd = tmp13.sf; \
        u15.se = tmp14.sf; \
    } while(0)

#ifndef DATA_TYPE_IN_BYTES
#error DATA_TYPE_IN_BYTES not set for the transpose OpenCL kernel
#endif /* not DATA_TYPE_IN_BYTES */

#undef VLOAD
#undef VSTORE

#if DATA_TYPE_IN_BYTES == 4
#define DATA_TYPE uint
#define TRANSPOSE() TRANSPOSE_4x4(u0, u1, u2, u3)
#define VLOAD(x, y) vload4(x, y)
#define VSTORE(x, y, z) vstore4(x, y, z)
#define BLOCK_SIZE 4
#elif DATA_TYPE_IN_BYTES == 2
#define DATA_TYPE ushort
#define TRANSPOSE() TRANSPOSE_8x8(u0, u1, u2, u3, u4, u5, u6, u7)
#define VLOAD(x, y) vload8(x, y)
#define VSTORE(x, y, z) vstore8(x, y, z)
#define BLOCK_SIZE 8
#elif DATA_TYPE_IN_BYTES == 1
#define DATA_TYPE uchar
#define TRANSPOSE() TRANSPOSE_16x16(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11, u12, u13, u14, u15)
#define VLOAD(x, y) vload16(x, y)
#define VSTORE(x, y, z) vstore16(x, y, z)
#define BLOCK_SIZE 16
#else /* switch DATA_TYPE_IN_BYTES */
#error DATA_TYPE_IN_BYTES not supported for transpose
#endif /* switch DATA_TYPE_IN_BYTES */

/** This OpenCL kernel computes the matrix transposition of input matrix
 *
 * @attention The number of bytes of the data type need to be passed at compile time using -DDATA_TYPE_IN_BYTES. DATA_TYPE_IN_BYTES can be:
 *  -# -DDATA_TYPE_IN_BYTES=1 for transposing U8 or S8 matrices
 *  -# -DDATA_TYPE_IN_BYTES=2 for transposing U16, S16 or FP16 matrices
 *  -# -DDATA_TYPE_IN_BYTES=4 for transposing U32, S32 or FP32 matrices
 *
 * @param[in]  src_ptr                           Pointer to the source matrix. Supported data types: U8/S8/U16/S16/F16/U32/S32/F32
 * @param[in]  src_stride_x                      Stride of the source matrix in X dimension (in bytes)
 * @param[in]  src_step_x                        src_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  src_stride_y                      Stride of the source matrix in Y dimension (in bytes)
 * @param[in]  src_step_y                        src_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_offset_first_element_in_bytes The offset of the first element in the source matrix
 * @param[out] dst_ptr                           Pointer to the destination matrix Supported data type: same as src_ptr
 * @param[in]  dst_stride_x                      Stride of the destination matrix in X dimension (in bytes)
 * @param[in]  dst_step_x                        dst_gx_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  dst_stride_y                      Stride of the destination matrix in Y dimension (in bytes)
 * @param[in]  dst_step_y                        dst_gx_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  dst_offset_first_element_in_bytes The offset of the first element in the destination matrix
 */
__kernel void transpose(IMAGE_DECLARATION(src),
                        IMAGE_DECLARATION(dst))
{
    uint x = get_global_id(0) * BLOCK_SIZE;
    uint y = get_global_id(1) * BLOCK_SIZE;

    // Compute source address
    Image src = CONVERT_TO_IMAGE_STRUCT(src);

    // Load the NxN block at (x, y)
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u0 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 0)));
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u1 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 1)));
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u2 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 2)));
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u3 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 3)));
#if BLOCK_SIZE > 4
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u4 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 4)));
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u5 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 5)));
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u6 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 6)));
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u7 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 7)));
#if BLOCK_SIZE == 16
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u8 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 8)));
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u9 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 9)));
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u10 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 10)));
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u11 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 11)));
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u12 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 12)));
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u13 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 13)));
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u14 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 14)));
    VEC_DATA_TYPE(DATA_TYPE, BLOCK_SIZE)
    u15 = VLOAD(0, (__global DATA_TYPE *)(offset(&src, 0, 15)));
#endif /* BLOCK_SIZE == 16 */
#endif /* BLOCK_SIZE > 4 */

    // Transpose the block
    TRANSPOSE();

    // Store the block at (y, x)
    uint dst_offset_in_bytes = y * DATA_TYPE_IN_BYTES + x * dst_stride_y + dst_offset_first_element_in_bytes;
    VSTORE(u0, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 0 * dst_stride_y));
    VSTORE(u1, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 1 * dst_stride_y));
    VSTORE(u2, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 2 * dst_stride_y));
    VSTORE(u3, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 3 * dst_stride_y));
#if BLOCK_SIZE > 4
    VSTORE(u4, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 4 * dst_stride_y));
    VSTORE(u5, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 5 * dst_stride_y));
    VSTORE(u6, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 6 * dst_stride_y));
    VSTORE(u7, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 7 * dst_stride_y));
#if BLOCK_SIZE == 16
    VSTORE(u8, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 8 * dst_stride_y));
    VSTORE(u9, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 9 * dst_stride_y));
    VSTORE(u10, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 10 * dst_stride_y));
    VSTORE(u11, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 11 * dst_stride_y));
    VSTORE(u12, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 12 * dst_stride_y));
    VSTORE(u13, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 13 * dst_stride_y));
    VSTORE(u14, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 14 * dst_stride_y));
    VSTORE(u15, 0, (__global DATA_TYPE *)(dst_ptr + dst_offset_in_bytes + 15 * dst_stride_y));
#endif /* BLOCK_SIZE == 16 */
#endif /* BLOCK_SIZE > 4 */
}
