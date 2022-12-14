/*
 * Copyright (c) 2018-2021 Arm Limited.
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

#if defined(SRC_DIM_Z)
/** This OpenCL kernel performs Winograd filter transform 3x3/3x1/1x3 when the data layout is NCHW and the output tile is 2x2/2x1/1x2
 *
 * @note In order to correctly split the input tensor in batches, its dimension across the Z axis (channels for NCHW, height for NHWC) must be passed at compile time using -DSRC_DIM_Z: e.g. -DSRC_DIM_Z=64
 * @note If this kernel is used to perform Winograd filter transform 3x1, -DWINOGRAD_FILTER_TRANSFORM_HORIZONTAL has to be passed at compile time
 * @note If this kernel is used to perform Winograd filter transform 1x3, -DWINOGRAD_FILTER_TRANSFORM_VERTICAL has to be passed at compile time
 * @note The data type must be passed at compile time using -DDATA_TYPE e.g. -DDATA_TYPE=float. Supported data types: float/half.
 *
 * @param[in]  src_ptr                           Pointer to the source tensor. Supported data types: F32/F16
 * @param[in]  src_stride_x                      Stride of the source tensor in X dimension (in bytes)
 * @param[in]  src_step_x                        src_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  src_stride_y                      Stride of the source tensor in Y dimension (in bytes)
 * @param[in]  src_step_y                        src_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  src_stride_w                      Stride of the source tensor in W dimension (in bytes)
 * @param[in]  src_step_w                        src_stride_w * number of elements along W processed per workitem(in bytes)
 * @param[in]  src_offset_first_element_in_bytes The offset of the first element in the source tensor
 * @param[out] dst_ptr                           Pointer to the destination tensor. Supported data types: same as @p src_ptr
 * @param[in]  dst_stride_x                      Stride of the destination tensor in X dimension (in bytes)
 * @param[in]  dst_step_x                        dst_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  dst_stride_y                      Stride of the destination tensor in Y dimension (in bytes)
 * @param[in]  dst_step_y                        dst_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  dst_offset_first_element_in_bytes The offset of the first element in the destination tensor
 */
__kernel void winograd_filter_transform_2x2_3x3_nchw(
    TENSOR4D_DECLARATION(src),
    TENSOR3D_DECLARATION(dst))
{
    Tensor4D src = CONVERT_TO_TENSOR4D_STRUCT(src, SRC_DIM_Z);

    const __global uchar *src_addr = tensor4D_offset(&src, 0, 0, 0, 0);

    // Load the values from the input tensor
#if defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL)
    VEC_DATA_TYPE(DATA_TYPE, 3)
    w0 = vload3(0, (__global DATA_TYPE *)(src_addr));
#elif defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
    VEC_DATA_TYPE(DATA_TYPE, 3)
    w0 = (VEC_DATA_TYPE(DATA_TYPE, 3))(*((__global DATA_TYPE *)(src_addr + 0 * src_stride_y)),
                                       *((__global DATA_TYPE *)(src_addr + 1 * src_stride_y)),
                                       *((__global DATA_TYPE *)(src_addr + 2 * src_stride_y)));
#else  // defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
    VEC_DATA_TYPE(DATA_TYPE, 3)
    w0 = vload3(0, (__global DATA_TYPE *)(src_addr + 0 * src_stride_y));
    VEC_DATA_TYPE(DATA_TYPE, 3)
    w1 = vload3(0, (__global DATA_TYPE *)(src_addr + 1 * src_stride_y));
    VEC_DATA_TYPE(DATA_TYPE, 3)
    w2 = vload3(0, (__global DATA_TYPE *)(src_addr + 2 * src_stride_y));
#endif // defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL)

    // Row 0
    VEC_DATA_TYPE(DATA_TYPE, 4)
    out0    = 0.0f;
    out0.s0 = (w0.s0);
    out0.s1 = (w0.s0 + w0.s1 + w0.s2) * 0.5f;
    out0.s2 = (w0.s0 + w0.s2 - w0.s1) * 0.5f;
    out0.s3 = (w0.s2);

#if !defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL) && !defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
    // Row 1
    VEC_DATA_TYPE(DATA_TYPE, 4)
    out1    = 0.0f;
    out1.s0 = (w0.s0 + w1.s0 + w2.s0) * 0.5f;
    out1.s1 = (w0.s0 + w1.s0 + w2.s0 + w0.s1 + w1.s1 + w2.s1 + w0.s2 + w1.s2 + w2.s2) * 0.25f;
    out1.s2 = (w0.s0 + w1.s0 + w2.s0 + w0.s2 + w1.s2 + w2.s2 - w0.s1 - w1.s1 - w2.s1) * 0.25f;
    out1.s3 = (w0.s2 + w1.s2 + w2.s2) * 0.5f;

    // Row 2
    VEC_DATA_TYPE(DATA_TYPE, 4)
    out2    = 0.0f;
    out2.s0 = (w0.s0 + w2.s0 - w1.s0) * 0.5f;
    out2.s1 = (w0.s0 + w2.s0 + w0.s1 + w2.s1 + w0.s2 + w2.s2 - w1.s0 - w1.s1 - w1.s2) * 0.25f;
    out2.s2 = (w0.s0 + w2.s0 + w1.s1 + w0.s2 + w2.s2 - w1.s0 - w0.s1 - w2.s1 - w1.s2) * 0.25f;
    out2.s3 = (w0.s2 + w2.s2 - w1.s2) * 0.5f;

    // Row 3
    VEC_DATA_TYPE(DATA_TYPE, 4)
    out3    = 0.0f;
    out3.s0 = (w2.s0);
    out3.s1 = (w2.s0 + w2.s1 + w2.s2) * 0.5f;
    out3.s2 = (w2.s0 + w2.s2 - w2.s1) * 0.5f;
    out3.s3 = (w2.s2);
#endif // !defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL) && !defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)

    int z  = get_global_id(2);
    int x0 = z / SRC_DIM_Z; // idx filter
    int y0 = z % SRC_DIM_Z; // idx channel

    // Get output address
    __global uchar *dst_addr = dst_ptr + dst_offset_first_element_in_bytes + x0 * dst_stride_x + y0 * dst_stride_y;

    // Store the values across the channels
    // 16 channels for 3x3 kernels
    // 4 channels for 3x1 or 1x3 kernels
    *(__global DATA_TYPE *)(dst_addr + 0 * dst_stride_z) = out0.s0;
    *(__global DATA_TYPE *)(dst_addr + 1 * dst_stride_z) = out0.s1;
    *(__global DATA_TYPE *)(dst_addr + 2 * dst_stride_z) = out0.s2;
    *(__global DATA_TYPE *)(dst_addr + 3 * dst_stride_z) = out0.s3;

#if !defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL) && !defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
    *(__global DATA_TYPE *)(dst_addr + 4 * dst_stride_z)  = out1.s0;
    *(__global DATA_TYPE *)(dst_addr + 5 * dst_stride_z)  = out1.s1;
    *(__global DATA_TYPE *)(dst_addr + 6 * dst_stride_z)  = out1.s2;
    *(__global DATA_TYPE *)(dst_addr + 7 * dst_stride_z)  = out1.s3;
    *(__global DATA_TYPE *)(dst_addr + 8 * dst_stride_z)  = out2.s0;
    *(__global DATA_TYPE *)(dst_addr + 9 * dst_stride_z)  = out2.s1;
    *(__global DATA_TYPE *)(dst_addr + 10 * dst_stride_z) = out2.s2;
    *(__global DATA_TYPE *)(dst_addr + 11 * dst_stride_z) = out2.s3;
    *(__global DATA_TYPE *)(dst_addr + 12 * dst_stride_z) = out3.s0;
    *(__global DATA_TYPE *)(dst_addr + 13 * dst_stride_z) = out3.s1;
    *(__global DATA_TYPE *)(dst_addr + 14 * dst_stride_z) = out3.s2;
    *(__global DATA_TYPE *)(dst_addr + 15 * dst_stride_z) = out3.s3;
#endif // !defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL) && !defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
}

/** This OpenCL kernel performs Winograd filter transform 3x3/3x1/1x3 when the data layout is NCHW and the output tile is 4x4/4x1/1x4
 *
 * @note In order to correctly split the input tensor in batches, its dimension across the Z axis (channels for NCHW, height for NHWC) must be passed at compile time using -DSRC_DIM_Z: e.g. -DSRC_DIM_Z=64
 * @note If this kernel is used to perform Winograd filter transform 3x1, -DWINOGRAD_FILTER_TRANSFORM_HORIZONTAL has to be passed at compile time
 * @note If this kernel is used to perform Winograd filter transform 1x3, -DWINOGRAD_FILTER_TRANSFORM_VERTICAL has to be passed at compile time
 * @note The data type must be passed at compile time using -DDATA_TYPE e.g. -DDATA_TYPE=float. Supported data types: float/half.
 *
 * @param[in]  src_ptr                           Pointer to the source tensor. Supported data types: F32/F16
 * @param[in]  src_stride_x                      Stride of the source tensor in X dimension (in bytes)
 * @param[in]  src_step_x                        src_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  src_stride_y                      Stride of the source tensor in Y dimension (in bytes)
 * @param[in]  src_step_y                        src_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  src_stride_w                      Stride of the source tensor in W dimension (in bytes)
 * @param[in]  src_step_w                        src_stride_w * number of elements along W processed per workitem(in bytes)
 * @param[in]  src_offset_first_element_in_bytes The offset of the first element in the source tensor
 * @param[out] dst_ptr                           Pointer to the destination tensor. Supported data types: same as @p src_ptr
 * @param[in]  dst_stride_x                      Stride of the destination tensor in X dimension (in bytes)
 * @param[in]  dst_step_x                        dst_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  dst_stride_y                      Stride of the destination tensor in Y dimension (in bytes)
 * @param[in]  dst_step_y                        dst_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  dst_offset_first_element_in_bytes The offset of the first element in the destination tensor
 */
__kernel void winograd_filter_transform_4x4_3x3_nchw(
    TENSOR4D_DECLARATION(src),
    TENSOR3D_DECLARATION(dst))
{
    Tensor4D src = CONVERT_TO_TENSOR4D_STRUCT(src, SRC_DIM_Z);

    const __global uchar *src_addr = tensor4D_offset(&src, 0, 0, 0, 0);

    // Load the values from the input tensor
#if defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL)
    VEC_DATA_TYPE(DATA_TYPE, 3)
    w0 = vload3(0, (__global DATA_TYPE *)(src_addr));
#elif defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
    VEC_DATA_TYPE(DATA_TYPE, 3)
    w0 = (VEC_DATA_TYPE(DATA_TYPE, 3))(*((__global DATA_TYPE *)(src_addr + 0 * src_stride_y)),
                                       *((__global DATA_TYPE *)(src_addr + 1 * src_stride_y)),
                                       *((__global DATA_TYPE *)(src_addr + 2 * src_stride_y)));
#else  // defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
    VEC_DATA_TYPE(DATA_TYPE, 3)
    w0 = vload3(0, (__global DATA_TYPE *)(src_addr + 0 * src_stride_y));
    VEC_DATA_TYPE(DATA_TYPE, 3)
    w1 = vload3(0, (__global DATA_TYPE *)(src_addr + 1 * src_stride_y));
    VEC_DATA_TYPE(DATA_TYPE, 3)
    w2 = vload3(0, (__global DATA_TYPE *)(src_addr + 2 * src_stride_y));
#endif // defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL)

    // Row 0
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out0    = 0.0f;
    out0.s0 = (w0.s0) / 16.f;
    out0.s1 = (-w0.s0 - w0.s1 - w0.s2) / 24.f;
    out0.s2 = (-w0.s0 + w0.s1 - w0.s2) / 24.f;
    out0.s3 = (w0.s0 + 2.f * w0.s1 + 4.f * w0.s2) / 96.f;
    out0.s4 = (w0.s0 - 2.f * w0.s1 + 4.f * w0.s2) / 96.f;
    out0.s5 = (w0.s2) / 4.f;

#if !defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL) && !defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
    // Row 1
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out1    = 0.0f;
    out1.s0 = (-w0.s0 - w1.s0 - w2.s0) / 24.f;
    out1.s1 = (w0.s0 + w1.s0 + w2.s0 + w0.s1 + w1.s1 + w2.s1 + w0.s2 + w1.s2 + w2.s2) / 36.f;
    out1.s2 = (w0.s0 + w1.s0 + w2.s0 - w0.s1 - w1.s1 - w2.s1 + w0.s2 + w1.s2 + w2.s2) / 36.f;
    out1.s3 = (-w0.s0 - w1.s0 - w2.s0 + 2.f * (-w0.s1 - w1.s1 - w2.s1) + 4.f * (-w0.s2 - w1.s2 - w2.s2)) / 144.f;
    out1.s4 = (-w0.s0 - w1.s0 - w2.s0 + 2.f * (w0.s1 + w1.s1 + w2.s1) + 4.f * (-w0.s2 - w1.s2 - w2.s2)) / 144.f;
    out1.s5 = (-w0.s2 - w1.s2 - w2.s2) / 6.f;

    // Row 2
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out2    = 0.0f;
    out2.s0 = (-w0.s0 + w1.s0 - w2.s0) / 24.f;
    out2.s1 = (w0.s0 - w1.s0 + w2.s0 + w0.s1 - w1.s1 + w2.s1 + w0.s2 - w1.s2 + w2.s2) / 36.f;
    out2.s2 = (w0.s0 - w1.s0 + w2.s0 - w0.s1 + w1.s1 - w2.s1 + w0.s2 - w1.s2 + w2.s2) / 36.f;
    out2.s3 = (-w0.s0 + w1.s0 - w2.s0 + 2.f * (-w0.s1 + w1.s1 - w2.s1) + 4.f * (-w0.s2 + w1.s2 - w2.s2)) / 144.f;
    out2.s4 = (-w0.s0 + w1.s0 - w2.s0 + 2.f * (w0.s1 - w1.s1 + w2.s1) + 4.f * (-w0.s2 + w1.s2 - w2.s2)) / 144.f;
    out2.s5 = (-w0.s2 + w1.s2 - w2.s2) / 6.f;

    // Row 3
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out3    = 0.0f;
    out3.s0 = (w0.s0 + 2.f * w1.s0 + 4.f * w2.s0) / 96.f;
    out3.s1 = (-w0.s0 - 2.f * w1.s0 - 4.f * w2.s0 - w0.s1 - 2.f * w1.s1 - 4.f * w2.s1 - w0.s2 - 2.f * w1.s2 - 4.f * w2.s2) / 144.f;
    out3.s2 = (-w0.s0 - 2.f * w1.s0 - 4.f * w2.s0 + w0.s1 + 2.f * w1.s1 + 4.f * w2.s1 - w0.s2 - 2.f * w1.s2 - 4.f * w2.s2) / 144.f;
    out3.s3 = ((w0.s0 + 2.f * w1.s0 + 4.f * w2.s0) + 2.f * (w0.s1 + 2.f * w1.s1 + 4.f * w2.s1) + 4.f * (w0.s2 + 2.f * w1.s2 + 4.f * w2.s2)) / 576.f;
    out3.s4 = ((w0.s0 + 2.f * w1.s0 + 4.f * w2.s0) + 2.f * (-w0.s1 - 2.f * w1.s1 - 4.f * w2.s1) + 4.f * (w0.s2 + 2.f * w1.s2 + 4.f * w2.s2)) / 576.f;
    out3.s5 = (w0.s2 + 2.f * w1.s2 + 4.f * w2.s2) / 24.f;

    // Row 4
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out4    = 0.0f;
    out4.s0 = (w0.s0 - 2.f * w1.s0 + 4.f * w2.s0) / 96.f;
    out4.s1 = (-w0.s0 + 2.f * w1.s0 - 4.f * w2.s0 - w0.s1 + 2.f * w1.s1 - 4.f * w2.s1 - w0.s2 + 2.f * w1.s2 - 4.f * w2.s2) / 144.f;
    out4.s2 = (-w0.s0 + 2.f * w1.s0 - 4.f * w2.s0 + w0.s1 - 2.f * w1.s1 + 4.f * w2.s1 - w0.s2 + 2.f * w1.s2 - 4.f * w2.s2) / 144.f;
    out4.s3 = ((w0.s0 - 2.f * w1.s0 + 4.f * w2.s0) + 2.f * (w0.s1 - 2.f * w1.s1 + 4.f * w2.s1) + 4.f * (w0.s2 - 2.f * w1.s2 + 4.f * w2.s2)) / 576.f;
    out4.s4 = ((w0.s0 - 2.f * w1.s0 + 4.f * w2.s0) + 2.f * (-w0.s1 + 2.f * w1.s1 - 4.f * w2.s1) + 4.f * (w0.s2 - 2.f * w1.s2 + 4.f * w2.s2)) / 576.f;
    out4.s5 = (w0.s2 - 2.f * w1.s2 + 4.f * w2.s2) / 24.f;

    // Row 5
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out5    = 0.0f;
    out5.s0 = (w2.s0) / 4.f;
    out5.s1 = (-w2.s0 - w2.s1 - w2.s2) / 6.f;
    out5.s2 = (-w2.s0 + w2.s1 - w2.s2) / 6.f;
    out5.s3 = (w2.s0 + 2.f * w2.s1 + 4.f * w2.s2) / 24.f;
    out5.s4 = (w2.s0 - 2.f * w2.s1 + 4.f * w2.s2) / 24.f;
    out5.s5 = (w2.s2);
#endif // !defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL) && !defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)

    int z  = get_global_id(2);
    int x0 = z / SRC_DIM_Z; // idx filter
    int y0 = z % SRC_DIM_Z; // idx channel

    // Get output address
    __global uchar *dst_addr = dst_ptr + dst_offset_first_element_in_bytes + x0 * dst_stride_x + y0 * dst_stride_y;

    // Store the values across the channels
    // 36 channels for 3x3 kernels
    // 6 channels for 3x1 or 1x3 kernels
    *(__global DATA_TYPE *)(dst_addr + 0 * dst_stride_z) = out0.s0;
    *(__global DATA_TYPE *)(dst_addr + 1 * dst_stride_z) = out0.s1;
    *(__global DATA_TYPE *)(dst_addr + 2 * dst_stride_z) = out0.s2;
    *(__global DATA_TYPE *)(dst_addr + 3 * dst_stride_z) = out0.s3;
    *(__global DATA_TYPE *)(dst_addr + 4 * dst_stride_z) = out0.s4;
    *(__global DATA_TYPE *)(dst_addr + 5 * dst_stride_z) = out0.s5;

#if !defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL) && !defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
    *(__global DATA_TYPE *)(dst_addr + 6 * dst_stride_z)  = out1.s0;
    *(__global DATA_TYPE *)(dst_addr + 7 * dst_stride_z)  = out1.s1;
    *(__global DATA_TYPE *)(dst_addr + 8 * dst_stride_z)  = out1.s2;
    *(__global DATA_TYPE *)(dst_addr + 9 * dst_stride_z)  = out1.s3;
    *(__global DATA_TYPE *)(dst_addr + 10 * dst_stride_z) = out1.s4;
    *(__global DATA_TYPE *)(dst_addr + 11 * dst_stride_z) = out1.s5;
    *(__global DATA_TYPE *)(dst_addr + 12 * dst_stride_z) = out2.s0;
    *(__global DATA_TYPE *)(dst_addr + 13 * dst_stride_z) = out2.s1;
    *(__global DATA_TYPE *)(dst_addr + 14 * dst_stride_z) = out2.s2;
    *(__global DATA_TYPE *)(dst_addr + 15 * dst_stride_z) = out2.s3;
    *(__global DATA_TYPE *)(dst_addr + 16 * dst_stride_z) = out2.s4;
    *(__global DATA_TYPE *)(dst_addr + 17 * dst_stride_z) = out2.s5;
    *(__global DATA_TYPE *)(dst_addr + 18 * dst_stride_z) = out3.s0;
    *(__global DATA_TYPE *)(dst_addr + 19 * dst_stride_z) = out3.s1;
    *(__global DATA_TYPE *)(dst_addr + 20 * dst_stride_z) = out3.s2;
    *(__global DATA_TYPE *)(dst_addr + 21 * dst_stride_z) = out3.s3;
    *(__global DATA_TYPE *)(dst_addr + 22 * dst_stride_z) = out3.s4;
    *(__global DATA_TYPE *)(dst_addr + 23 * dst_stride_z) = out3.s5;
    *(__global DATA_TYPE *)(dst_addr + 24 * dst_stride_z) = out4.s0;
    *(__global DATA_TYPE *)(dst_addr + 25 * dst_stride_z) = out4.s1;
    *(__global DATA_TYPE *)(dst_addr + 26 * dst_stride_z) = out4.s2;
    *(__global DATA_TYPE *)(dst_addr + 27 * dst_stride_z) = out4.s3;
    *(__global DATA_TYPE *)(dst_addr + 28 * dst_stride_z) = out4.s4;
    *(__global DATA_TYPE *)(dst_addr + 29 * dst_stride_z) = out4.s5;
    *(__global DATA_TYPE *)(dst_addr + 30 * dst_stride_z) = out5.s0;
    *(__global DATA_TYPE *)(dst_addr + 31 * dst_stride_z) = out5.s1;
    *(__global DATA_TYPE *)(dst_addr + 32 * dst_stride_z) = out5.s2;
    *(__global DATA_TYPE *)(dst_addr + 33 * dst_stride_z) = out5.s3;
    *(__global DATA_TYPE *)(dst_addr + 34 * dst_stride_z) = out5.s4;
    *(__global DATA_TYPE *)(dst_addr + 35 * dst_stride_z) = out5.s5;
#endif // !defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL) && !defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
}

/** This OpenCL kernel performs Winograd filter transform 5x5/5x1 or 1x5 when the data layout is NCHW and the output tile is 4x4/4x1 or 1x4
 *
 * @note In order to correctly split the input tensor in batches, its dimension across the Z axis (channels for NCHW, height for NHWC) must be passed at compile time using -DSRC_DIM_Z: e.g. -DSRC_DIM_Z=64
 *
 * @note If this kernel is used to perform Winograd filter transform 5x1, -DWINOGRAD_FILTER_TRANSFORM_HORIZONTAL has to be passed at compile time
 * @note If this kernel is used to perform Winograd filter transform 1x5, -DWINOGRAD_FILTER_TRANSFORM_VERTICAL has to be passed at compile time
 * @note The data type must be passed at compile time using -DDATA_TYPE e.g. -DDATA_TYPE=float. Supported data types: float/half.
 *
 * @param[in]  src_ptr                           Pointer to the source tensor. Supported data types: F32/F16
 * @param[in]  src_stride_x                      Stride of the source tensor in X dimension (in bytes)
 * @param[in]  src_step_x                        src_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  src_stride_y                      Stride of the source tensor in Y dimension (in bytes)
 * @param[in]  src_step_y                        src_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  src_stride_w                      Stride of the source tensor in W dimension (in bytes)
 * @param[in]  src_step_w                        src_stride_w * number of elements along W processed per workitem(in bytes)
 * @param[in]  src_offset_first_element_in_bytes The offset of the first element in the source tensor
 * @param[out] dst_ptr                           Pointer to the destination tensor. Supported data types: same as @p src_ptr
 * @param[in]  dst_stride_x                      Stride of the destination tensor in X dimension (in bytes)
 * @param[in]  dst_step_x                        dst_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  dst_stride_y                      Stride of the destination tensor in Y dimension (in bytes)
 * @param[in]  dst_step_y                        dst_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  dst_offset_first_element_in_bytes The offset of the first element in the destination tensor
 */
__kernel void winograd_filter_transform_4x4_5x5_nchw(
    TENSOR4D_DECLARATION(src),
    TENSOR3D_DECLARATION(dst))
{
    Tensor4D src = CONVERT_TO_TENSOR4D_STRUCT(src, SRC_DIM_Z);

    const __global uchar *src_addr = tensor4D_offset(&src, 0, 0, 0, 0);

    // Load the values from the input tensor
#if defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL)
    VEC_DATA_TYPE(DATA_TYPE, 4)
    w00           = vload4(0, (__global DATA_TYPE *)(src_addr + 0 * src_stride_y));
    DATA_TYPE w01 = *((__global DATA_TYPE *)(src_addr + 0 * src_stride_y) + 4);
#elif defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
    VEC_DATA_TYPE(DATA_TYPE, 4)
    w00 = (VEC_DATA_TYPE(DATA_TYPE, 4))(*((__global DATA_TYPE *)(src_addr + 0 * src_stride_y)),
                                        *((__global DATA_TYPE *)(src_addr + 1 * src_stride_y)),
                                        *((__global DATA_TYPE *)(src_addr + 2 * src_stride_y)),
                                        *((__global DATA_TYPE *)(src_addr + 3 * src_stride_y)));
    DATA_TYPE w01 = *((__global DATA_TYPE *)(src_addr + 4 * src_stride_y));
#else  // defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
    VEC_DATA_TYPE(DATA_TYPE, 4)
    w00           = vload4(0, (__global DATA_TYPE *)(src_addr + 0 * src_stride_y));
    DATA_TYPE w01 = *((__global DATA_TYPE *)(src_addr + 0 * src_stride_y) + 4);
    VEC_DATA_TYPE(DATA_TYPE, 4)
    w10           = vload4(0, (__global DATA_TYPE *)(src_addr + 1 * src_stride_y));
    DATA_TYPE w11 = *((__global DATA_TYPE *)(src_addr + 1 * src_stride_y) + 4);
    VEC_DATA_TYPE(DATA_TYPE, 4)
    w20           = vload4(0, (__global DATA_TYPE *)(src_addr + 2 * src_stride_y));
    DATA_TYPE w21 = *((__global DATA_TYPE *)(src_addr + 2 * src_stride_y) + 4);
    VEC_DATA_TYPE(DATA_TYPE, 4)
    w30           = vload4(0, (__global DATA_TYPE *)(src_addr + 3 * src_stride_y));
    DATA_TYPE w31 = *((__global DATA_TYPE *)(src_addr + 3 * src_stride_y) + 4);
    VEC_DATA_TYPE(DATA_TYPE, 4)
    w40           = vload4(0, (__global DATA_TYPE *)(src_addr + 4 * src_stride_y));
    DATA_TYPE w41 = *((__global DATA_TYPE *)(src_addr + 4 * src_stride_y) + 4);
#endif // defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL)

    // Transform the input tile

    // Row 0
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out0    = 0.0f;
    out0.s0 = w00.s0;
    out0.s1 = -2.f * (w00.s0 + w00.s1 + w00.s2 + w00.s3 + w01) / 9.f;
    out0.s2 = -2.f * (w00.s0 - w00.s1 + w00.s2 - w00.s3 + w01) / 9.f;
    out0.s3 = (w00.s0 + 2.f * w00.s1 + 4.f * w00.s2 + 8.f * w00.s3 + 16.f * w01) / 90.f;
    out0.s4 = (w00.s0 - 2.f * w00.s1 + 4.f * w00.s2 - 8.f * w00.s3 + 16.f * w01) / 90.f;
    out0.s5 = (16.f * w00.s0 + 8.f * w00.s1 + 4.f * w00.s2 + 2.f * w00.s3 + w01) / 180.f;
    out0.s6 = (16.f * w00.s0 - 8.f * w00.s1 + 4.f * w00.s2 - 2.f * w00.s3 + w01) / 180.f;
    out0.s7 = w01;

#if !defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL) && !defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
    // Row 1
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out1    = 0.0f;
    out1.s0 = -2.f * (w00.s0 + w10.s0 + w20.s0 + w30.s0 + w40.s0) / 9.f;
    out1.s1 = 4.f * ((w00.s0 + w10.s0 + w20.s0 + w30.s0 + w40.s0) + (w00.s1 + w10.s1 + w20.s1 + w30.s1 + w40.s1) + (w00.s2 + w10.s2 + w20.s2 + w30.s2 + w40.s2) +
                     (w00.s3 + w10.s3 + w20.s3 + w30.s3 + w40.s3) + (w01 + w11 + w21 + w31 + w41)) / 81.f;
    out1.s2 = 4.f * ((w00.s0 + w10.s0 + w20.s0 + w30.s0 + w40.s0) - (w00.s1 + w10.s1 + w20.s1 + w30.s1 + w40.s1) + (w00.s2 + w10.s2 + w20.s2 + w30.s2 + w40.s2) -
                     (w00.s3 + w10.s3 + w20.s3 + w30.s3 + w40.s3) + (w01 + w11 + w21 + w31 + w41)) / 81.f;
    out1.s3 = -((w00.s0 + w10.s0 + w20.s0 + w30.s0 + w40.s0) + 2.f * (w00.s1 + w10.s1 + w20.s1 + w30.s1 + w40.s1) + 4.f * (w00.s2 + w10.s2 + w20.s2 + w30.s2 + w40.s2) + 8.f *
                (w00.s3 + w10.s3 + w20.s3 + w30.s3 + w40.s3) + 16.f * (w01 + w11 + w21 + w31 + w41)) / 405.f;
    out1.s4 = -((w00.s0 + w10.s0 + w20.s0 + w30.s0 + w40.s0) - 2.f * (w00.s1 + w10.s1 + w20.s1 + w30.s1 + w40.s1) + 4.f * (w00.s2 + w10.s2 + w20.s2 + w30.s2 + w40.s2) - 8.f *
                (w00.s3 + w10.s3 + w20.s3 + w30.s3 + w40.s3) + 16.f * (w01 + w11 + w21 + w31 + w41)) / 405.f;
    out1.s5 = -(16.f * (w00.s0 + w10.s0 + w20.s0 + w30.s0 + w40.s0) + 8.f * (w00.s1 + w10.s1 + w20.s1 + w30.s1 + w40.s1) + 4.f * (w00.s2 + w10.s2 + w20.s2 + w30.s2 + w40.s2) + 2.f *
                (w00.s3 + w10.s3 + w20.s3 + w30.s3 + w40.s3) + (w01 + w11 + w21 + w31 + w41)) / 810.f;
    out1.s6 = -(16.f * (w00.s0 + w10.s0 + w20.s0 + w30.s0 + w40.s0) - 8.f * (w00.s1 + w10.s1 + w20.s1 + w30.s1 + w40.s1) + 4.f * (w00.s2 + w10.s2 + w20.s2 + w30.s2 + w40.s2) - 2.f *
                (w00.s3 + w10.s3 + w20.s3 + w30.s3 + w40.s3) + (w01 + w11 + w21 + w31 + w41)) / 810.f;
    out1.s7 = -2.f * (w01 + w11 + w21 + w31 + w41) / 9.f;

    // Row 2
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out2    = 0.0f;
    out2.s0 = -2.f * (w00.s0 - w10.s0 + w20.s0 - w30.s0 + w40.s0) / 9.f;
    out2.s1 = 4.f * ((w00.s0 - w10.s0 + w20.s0 - w30.s0 + w40.s0) + (w00.s1 - w10.s1 + w20.s1 - w30.s1 + w40.s1) + (w00.s2 - w10.s2 + w20.s2 - w30.s2 + w40.s2) +
                     (w00.s3 - w10.s3 + w20.s3 - w30.s3 + w40.s3) + (w01 - w11 + w21 - w31 + w41)) / 81.f;
    out2.s2 = 4.f * ((w00.s0 - w10.s0 + w20.s0 - w30.s0 + w40.s0) - (w00.s1 - w10.s1 + w20.s1 - w30.s1 + w40.s1) + (w00.s2 - w10.s2 + w20.s2 - w30.s2 + w40.s2) -
                     (w00.s3 - w10.s3 + w20.s3 - w30.s3 + w40.s3) + (w01 - w11 + w21 - w31 + w41)) / 81.f;
    out2.s3 = -((w00.s0 - w10.s0 + w20.s0 - w30.s0 + w40.s0) + 2.f * (w00.s1 - w10.s1 + w20.s1 - w30.s1 + w40.s1) + 4.f * (w00.s2 - w10.s2 + w20.s2 - w30.s2 + w40.s2) + 8.f *
                (w00.s3 - w10.s3 + w20.s3 - w30.s3 + w40.s3) + 16.f * (w01 - w11 + w21 - w31 + w41)) / 405.f;
    out2.s4 = -((w00.s0 - w10.s0 + w20.s0 - w30.s0 + w40.s0) - 2.f * (w00.s1 - w10.s1 + w20.s1 - w30.s1 + w40.s1) + 4.f * (w00.s2 - w10.s2 + w20.s2 - w30.s2 + w40.s2) - 8.f *
                (w00.s3 - w10.s3 + w20.s3 - w30.s3 + w40.s3) + 16.f * (w01 - w11 + w21 - w31 + w41)) / 405.f;
    out2.s5 = -(16.f * (w00.s0 - w10.s0 + w20.s0 - w30.s0 + w40.s0) + 8.f * (w00.s1 - w10.s1 + w20.s1 - w30.s1 + w40.s1) + 4.f * (w00.s2 - w10.s2 + w20.s2 - w30.s2 + w40.s2) + 2.f *
                (w00.s3 - w10.s3 + w20.s3 - w30.s3 + w40.s3) + (w01 - w11 + w21 - w31 + w41)) / 810.f;
    out2.s6 = -(16.f * (w00.s0 - w10.s0 + w20.s0 - w30.s0 + w40.s0) - 8.f * (w00.s1 - w10.s1 + w20.s1 - w30.s1 + w40.s1) + 4.f * (w00.s2 - w10.s2 + w20.s2 - w30.s2 + w40.s2) - 2.f *
                (w00.s3 - w10.s3 + w20.s3 - w30.s3 + w40.s3) + (w01 - w11 + w21 - w31 + w41)) / 810.f;
    out2.s7 = -2.f * (w01 - w11 + w21 - w31 + w41) / 9.f;

    // Row 3
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out3    = 0.0f;
    out3.s0 = (w00.s0 + 2.f * w10.s0 + 4.f * w20.s0 + 8.f * w30.s0 + 16.f * w40.s0) / 90.f;
    out3.s1 = -((w00.s0 + 2.f * w10.s0 + 4.f * w20.s0 + 8.f * w30.s0 + 16.f * w40.s0) + (w00.s1 + 2.f * w10.s1 + 4.f * w20.s1 + 8.f * w30.s1 + 16.f * w40.s1) +
                (w00.s2 + 2.f * w10.s2 + 4.f * w20.s2 + 8.f * w30.s2 + 16.f * w40.s2) + (w00.s3 + 2.f * w10.s3 + 4.f * w20.s3 + 8.f * w30.s3 + 16.f * w40.s3) +
                (w01 + 2.f * w11 + 4.f * w21 + 8.f * w31 + 16.f * w41)) / 405.f;
    out3.s2 = -((w00.s0 + 2.f * w10.s0 + 4.f * w20.s0 + 8.f * w30.s0 + 16.f * w40.s0) - (w00.s1 + 2.f * w10.s1 + 4.f * w20.s1 + 8.f * w30.s1 + 16.f * w40.s1) +
                (w00.s2 + 2.f * w10.s2 + 4.f * w20.s2 + 8.f * w30.s2 + 16.f * w40.s2) - (w00.s3 + 2.f * w10.s3 + 4.f * w20.s3 + 8.f * w30.s3 + 16.f * w40.s3) +
                (w01 + 2.f * w11 + 4.f * w21 + 8.f * w31 + 16.f * w41)) / 405.f;
    out3.s3 = ((w00.s0 + 2.f * w10.s0 + 4.f * w20.s0 + 8.f * w30.s0 + 16.f * w40.s0) + 2.f * (w00.s1 + 2.f * w10.s1 + 4.f * w20.s1 + 8.f * w30.s1 + 16.f * w40.s1) + 4.f *
               (w00.s2 + 2.f * w10.s2 + 4.f * w20.s2 + 8.f * w30.s2 + 16.f * w40.s2) + 8.f * (w00.s3 + 2.f * w10.s3 + 4.f * w20.s3 + 8.f * w30.s3 + 16.f * w40.s3) + 16.f *
               (w01 + 2.f * w11 + 4.f * w21 + 8.f * w31 + 16.f * w41)) / 8100.f;
    out3.s4 = ((w00.s0 + 2.f * w10.s0 + 4.f * w20.s0 + 8.f * w30.s0 + 16.f * w40.s0) - 2.f * (w00.s1 + 2.f * w10.s1 + 4.f * w20.s1 + 8.f * w30.s1 + 16.f * w40.s1) + 4.f *
               (w00.s2 + 2.f * w10.s2 + 4.f * w20.s2 + 8.f * w30.s2 + 16.f * w40.s2) - 8.f * (w00.s3 + 2.f * w10.s3 + 4.f * w20.s3 + 8.f * w30.s3 + 16.f * w40.s3) + 16.f *
               (w01 + 2.f * w11 + 4.f * w21 + 8.f * w31 + 16.f * w41)) / 8100.f;
    out3.s5 = (16.f * (w00.s0 + 2.f * w10.s0 + 4.f * w20.s0 + 8.f * w30.s0 + 16.f * w40.s0) + 8.f * (w00.s1 + 2.f * w10.s1 + 4.f * w20.s1 + 8.f * w30.s1 + 16.f * w40.s1) + 4.f *
               (w00.s2 + 2.f * w10.s2 + 4.f * w20.s2 + 8.f * w30.s2 + 16.f * w40.s2) + 2.f * (w00.s3 + 2.f * w10.s3 + 4.f * w20.s3 + 8.f * w30.s3 + 16.f * w40.s3) +
               (w01 + 2.f * w11 + 4.f * w21 + 8.f * w31 + 16.f * w41)) / 16200.f;
    out3.s6 = (16.f * (w00.s0 + 2.f * w10.s0 + 4.f * w20.s0 + 8.f * w30.s0 + 16.f * w40.s0) - 8.f * (w00.s1 + 2.f * w10.s1 + 4.f * w20.s1 + 8.f * w30.s1 + 16.f * w40.s1) + 4.f *
               (w00.s2 + 2.f * w10.s2 + 4.f * w20.s2 + 8.f * w30.s2 + 16.f * w40.s2) - 2.f * (w00.s3 + 2.f * w10.s3 + 4.f * w20.s3 + 8.f * w30.s3 + 16.f * w40.s3) +
               (w01 + 2.f * w11 + 4.f * w21 + 8.f * w31 + 16.f * w41)) / 16200.f;
    out3.s7 = (w01 + 2.f * w11 + 4.f * w21 + 8.f * w31 + 16.f * w41) / 90.f;

    // Row 4
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out4    = 0.0f;
    out4.s0 = (w00.s0 - 2.f * w10.s0 + 4.f * w20.s0 - 8.f * w30.s0 + 16.f * w40.s0) / 90.f;
    out4.s1 = -((w00.s0 - 2.f * w10.s0 + 4.f * w20.s0 - 8.f * w30.s0 + 16.f * w40.s0) + (w00.s1 - 2.f * w10.s1 + 4.f * w20.s1 - 8.f * w30.s1 + 16.f * w40.s1) +
                (w00.s2 - 2.f * w10.s2 + 4.f * w20.s2 - 8.f * w30.s2 + 16.f * w40.s2) + (w00.s3 - 2.f * w10.s3 + 4.f * w20.s3 - 8.f * w30.s3 + 16.f * w40.s3) +
                (w01 - 2.f * w11 + 4.f * w21 - 8.f * w31 + 16.f * w41)) / 405.f;
    out4.s2 = -((w00.s0 - 2.f * w10.s0 + 4.f * w20.s0 - 8.f * w30.s0 + 16.f * w40.s0) - (w00.s1 - 2.f * w10.s1 + 4.f * w20.s1 - 8.f * w30.s1 + 16.f * w40.s1) +
                (w00.s2 - 2.f * w10.s2 + 4.f * w20.s2 - 8.f * w30.s2 + 16.f * w40.s2) - (w00.s3 - 2.f * w10.s3 + 4.f * w20.s3 - 8.f * w30.s3 + 16.f * w40.s3) +
                (w01 - 2.f * w11 + 4.f * w21 - 8.f * w31 + 16.f * w41)) / 405.f;
    out4.s3 = ((w00.s0 - 2.f * w10.s0 + 4.f * w20.s0 - 8.f * w30.s0 + 16.f * w40.s0) + 2.f * (w00.s1 - 2.f * w10.s1 + 4.f * w20.s1 - 8.f * w30.s1 + 16.f * w40.s1) + 4.f *
               (w00.s2 - 2.f * w10.s2 + 4.f * w20.s2 - 8.f * w30.s2 + 16.f * w40.s2) + 8.f * (w00.s3 - 2.f * w10.s3 + 4.f * w20.s3 - 8.f * w30.s3 + 16.f * w40.s3) + 16.f *
               (w01 - 2.f * w11 + 4.f * w21 - 8.f * w31 + 16.f * w41)) / 8100.f;
    out4.s4 = ((w00.s0 - 2.f * w10.s0 + 4.f * w20.s0 - 8.f * w30.s0 + 16.f * w40.s0) - 2.f * (w00.s1 - 2.f * w10.s1 + 4.f * w20.s1 - 8.f * w30.s1 + 16.f * w40.s1) + 4.f *
               (w00.s2 - 2.f * w10.s2 + 4.f * w20.s2 - 8.f * w30.s2 + 16.f * w40.s2) - 8.f * (w00.s3 - 2.f * w10.s3 + 4.f * w20.s3 - 8.f * w30.s3 + 16.f * w40.s3) + 16.f *
               (w01 - 2.f * w11 + 4.f * w21 - 8.f * w31 + 16.f * w41)) / 8100.f;
    out4.s5 = (16.f * (w00.s0 - 2.f * w10.s0 + 4.f * w20.s0 - 8.f * w30.s0 + 16.f * w40.s0) + 8.f * (w00.s1 - 2.f * w10.s1 + 4.f * w20.s1 - 8.f * w30.s1 + 16.f * w40.s1) + 4.f *
               (w00.s2 - 2.f * w10.s2 + 4.f * w20.s2 - 8.f * w30.s2 + 16.f * w40.s2) + 2.f * (w00.s3 - 2.f * w10.s3 + 4.f * w20.s3 - 8.f * w30.s3 + 16.f * w40.s3) +
               (w01 - 2.f * w11 + 4.f * w21 - 8.f * w31 + 16.f * w41)) / 16200.f;
    out4.s6 = (16.f * (w00.s0 - 2.f * w10.s0 + 4.f * w20.s0 - 8.f * w30.s0 + 16.f * w40.s0) - 8.f * (w00.s1 - 2.f * w10.s1 + 4.f * w20.s1 - 8.f * w30.s1 + 16.f * w40.s1) + 4.f *
               (w00.s2 - 2.f * w10.s2 + 4.f * w20.s2 - 8.f * w30.s2 + 16.f * w40.s2) - 2.f * (w00.s3 - 2.f * w10.s3 + 4.f * w20.s3 - 8.f * w30.s3 + 16.f * w40.s3) +
               (w01 - 2.f * w11 + 4.f * w21 - 8.f * w31 + 16.f * w41)) / 16200.f;
    out4.s7 = (w01 - 2.f * w11 + 4.f * w21 - 8.f * w31 + 16.f * w41) / 90.f;

    // Row 5
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out5    = 0.0f;
    out5.s0 = (16.f * w00.s0 + 8.f * w10.s0 + 4.f * w20.s0 + 2.f * w30.s0 + w40.s0) / 180.f;
    out5.s1 = -((16.f * w00.s0 + 8.f * w10.s0 + 4.f * w20.s0 + 2.f * w30.s0 + w40.s0) + (16.f * w00.s1 + 8.f * w10.s1 + 4.f * w20.s1 + 2.f * w30.s1 + w40.s1) +
                (16.f * w00.s2 + 8.f * w10.s2 + 4.f * w20.s2 + 2.f * w30.s2 + w40.s2) + (16.f * w00.s3 + 8.f * w10.s3 + 4.f * w20.s3 + 2.f * w30.s3 + w40.s3) +
                (16.f * w01 + 8.f * w11 + 4.f * w21 + 2.f * w31 + w41)) / 810.f;
    out5.s2 = -((16.f * w00.s0 + 8.f * w10.s0 + 4.f * w20.s0 + 2.f * w30.s0 + w40.s0) - (16.f * w00.s1 + 8.f * w10.s1 + 4.f * w20.s1 + 2.f * w30.s1 + w40.s1) +
                (16.f * w00.s2 + 8.f * w10.s2 + 4.f * w20.s2 + 2.f * w30.s2 + w40.s2) - (16.f * w00.s3 + 8.f * w10.s3 + 4.f * w20.s3 + 2.f * w30.s3 + w40.s3) +
                (16.f * w01 + 8.f * w11 + 4.f * w21 + 2.f * w31 + w41)) / 810.f;
    out5.s3 = ((16.f * w00.s0 + 8.f * w10.s0 + 4.f * w20.s0 + 2.f * w30.s0 + w40.s0) + 2.f * (16.f * w00.s1 + 8.f * w10.s1 + 4.f * w20.s1 + 2.f * w30.s1 + w40.s1) + 4.f *
               (16.f * w00.s2 + 8.f * w10.s2 + 4.f * w20.s2 + 2.f * w30.s2 + w40.s2) + 8.f * (16.f * w00.s3 + 8.f * w10.s3 + 4.f * w20.s3 + 2.f * w30.s3 + w40.s3) + 16.f *
               (16.f * w01 + 8.f * w11 + 4.f * w21 + 2.f * w31 + w41)) / 16200.f;
    out5.s4 = ((16.f * w00.s0 + 8.f * w10.s0 + 4.f * w20.s0 + 2.f * w30.s0 + w40.s0) - 2.f * (16.f * w00.s1 + 8.f * w10.s1 + 4.f * w20.s1 + 2.f * w30.s1 + w40.s1) + 4.f *
               (16.f * w00.s2 + 8.f * w10.s2 + 4.f * w20.s2 + 2.f * w30.s2 + w40.s2) - 8.f * (16.f * w00.s3 + 8.f * w10.s3 + 4.f * w20.s3 + 2.f * w30.s3 + w40.s3) + 16.f *
               (16.f * w01 + 8.f * w11 + 4.f * w21 + 2.f * w31 + w41)) / 16200.f;
    out5.s5 = (16.f * (16.f * w00.s0 + 8.f * w10.s0 + 4.f * w20.s0 + 2.f * w30.s0 + w40.s0) + 8.f * (16.f * w00.s1 + 8.f * w10.s1 + 4.f * w20.s1 + 2.f * w30.s1 + w40.s1) + 4.f *
               (16.f * w00.s2 + 8.f * w10.s2 + 4.f * w20.s2 + 2.f * w30.s2 + w40.s2) + 2.f * (16.f * w00.s3 + 8.f * w10.s3 + 4.f * w20.s3 + 2.f * w30.s3 + w40.s3) +
               (16.f * w01 + 8.f * w11 + 4.f * w21 + 2.f * w31 + w41)) / 32400.f;
    out5.s6 = (16.f * (16.f * w00.s0 + 8.f * w10.s0 + 4.f * w20.s0 + 2.f * w30.s0 + w40.s0) - 8.f * (16.f * w00.s1 + 8.f * w10.s1 + 4.f * w20.s1 + 2.f * w30.s1 + w40.s1) + 4.f *
               (16.f * w00.s2 + 8.f * w10.s2 + 4.f * w20.s2 + 2.f * w30.s2 + w40.s2) - 2.f * (16.f * w00.s3 + 8.f * w10.s3 + 4.f * w20.s3 + 2.f * w30.s3 + w40.s3) +
               (16.f * w01 + 8.f * w11 + 4.f * w21 + 2.f * w31 + w41)) / 32400.f;
    out5.s7 = (16.f * w01 + 8.f * w11 + 4.f * w21 + 2.f * w31 + w41) / 180.f;

    // Row 6
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out6    = 0.0f;
    out6.s0 = (16.f * w00.s0 - 8.f * w10.s0 + 4.f * w20.s0 - 2.f * w30.s0 + w40.s0) / 180.f;
    out6.s1 = -((16.f * w00.s0 - 8.f * w10.s0 + 4.f * w20.s0 - 2.f * w30.s0 + w40.s0) + (16.f * w00.s1 - 8.f * w10.s1 + 4.f * w20.s1 - 2.f * w30.s1 + w40.s1) +
                (16.f * w00.s2 - 8.f * w10.s2 + 4.f * w20.s2 - 2.f * w30.s2 + w40.s2) + (16.f * w00.s3 - 8.f * w10.s3 + 4.f * w20.s3 - 2.f * w30.s3 + w40.s3) +
                (16.f * w01 - 8.f * w11 + 4.f * w21 - 2.f * w31 + w41)) / 810.f;
    out6.s2 = -((16.f * w00.s0 - 8.f * w10.s0 + 4.f * w20.s0 - 2.f * w30.s0 + w40.s0) - (16.f * w00.s1 - 8.f * w10.s1 + 4.f * w20.s1 - 2.f * w30.s1 + w40.s1) +
                (16.f * w00.s2 - 8.f * w10.s2 + 4.f * w20.s2 - 2.f * w30.s2 + w40.s2) - (16.f * w00.s3 - 8.f * w10.s3 + 4.f * w20.s3 - 2.f * w30.s3 + w40.s3) +
                (16.f * w01 - 8.f * w11 + 4.f * w21 - 2.f * w31 + w41)) / 810.f;
    out6.s3 = ((16.f * w00.s0 - 8.f * w10.s0 + 4.f * w20.s0 - 2.f * w30.s0 + w40.s0) + 2.f * (16.f * w00.s1 - 8.f * w10.s1 + 4.f * w20.s1 - 2.f * w30.s1 + w40.s1) + 4.f *
               (16.f * w00.s2 - 8.f * w10.s2 + 4.f * w20.s2 - 2.f * w30.s2 + w40.s2) + 8.f * (16.f * w00.s3 - 8.f * w10.s3 + 4.f * w20.s3 - 2.f * w30.s3 + w40.s3) + 16.f *
               (16.f * w01 - 8.f * w11 + 4.f * w21 - 2.f * w31 + w41)) / 16200.f;
    out6.s4 = ((16.f * w00.s0 - 8.f * w10.s0 + 4.f * w20.s0 - 2.f * w30.s0 + w40.s0) - 2.f * (16.f * w00.s1 - 8.f * w10.s1 + 4.f * w20.s1 - 2.f * w30.s1 + w40.s1) + 4.f *
               (16.f * w00.s2 - 8.f * w10.s2 + 4.f * w20.s2 - 2.f * w30.s2 + w40.s2) - 8.f * (16.f * w00.s3 - 8.f * w10.s3 + 4.f * w20.s3 - 2.f * w30.s3 + w40.s3) + 16.f *
               (16.f * w01 - 8.f * w11 + 4.f * w21 - 2.f * w31 + w41)) / 16200.f;
    out6.s5 = (16.f * (16.f * w00.s0 - 8.f * w10.s0 + 4.f * w20.s0 - 2.f * w30.s0 + w40.s0) + 8.f * (16.f * w00.s1 - 8.f * w10.s1 + 4.f * w20.s1 - 2.f * w30.s1 + w40.s1) + 4.f *
               (16.f * w00.s2 - 8.f * w10.s2 + 4.f * w20.s2 - 2.f * w30.s2 + w40.s2) + 2.f * (16.f * w00.s3 - 8.f * w10.s3 + 4.f * w20.s3 - 2.f * w30.s3 + w40.s3) +
               (16.f * w01 - 8.f * w11 + 4.f * w21 - 2.f * w31 + w41)) / 32400.f;
    out6.s6 = (16.f * (16.f * w00.s0 - 8.f * w10.s0 + 4.f * w20.s0 - 2.f * w30.s0 + w40.s0) - 8.f * (16.f * w00.s1 - 8.f * w10.s1 + 4.f * w20.s1 - 2.f * w30.s1 + w40.s1) + 4.f *
               (16.f * w00.s2 - 8.f * w10.s2 + 4.f * w20.s2 - 2.f * w30.s2 + w40.s2) - 2.f * (16.f * w00.s3 - 8.f * w10.s3 + 4.f * w20.s3 - 2.f * w30.s3 + w40.s3) +
               (16.f * w01 - 8.f * w11 + 4.f * w21 - 2.f * w31 + w41)) / 32400.f;
    out6.s7 = (16.f * w01 - 8.f * w11 + 4.f * w21 - 2.f * w31 + w41) / 180.f;

    // Row 7
    VEC_DATA_TYPE(DATA_TYPE, 8)
    out7    = 0.0f;
    out7.s0 = w40.s0;
    out7.s1 = -2.f * (w40.s0 + w40.s1 + w40.s2 + w40.s3 + w41) / 9.f;
    out7.s2 = -2.f * (w40.s0 - w40.s1 + w40.s2 - w40.s3 + w41) / 9.f;
    out7.s3 = (w40.s0 + 2.f * w40.s1 + 4.f * w40.s2 + 8.f * w40.s3 + 16.f * w41) / 90.f;
    out7.s4 = (w40.s0 - 2.f * w40.s1 + 4.f * w40.s2 - 8.f * w40.s3 + 16.f * w41) / 90.f;
    out7.s5 = (16.f * w40.s0 + 8.f * w40.s1 + 4.f * w40.s2 + 2.f * w40.s3 + w41) / 180.f;
    out7.s6 = (16.f * w40.s0 - 8.f * w40.s1 + 4.f * w40.s2 - 2.f * w40.s3 + w41) / 180.f;
    out7.s7 = w41;
#endif // !defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL) && !defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)

    int z  = get_global_id(2);
    int x0 = z / SRC_DIM_Z; // idx filter
    int y0 = z % SRC_DIM_Z; // idx channel

    // Get output address
    __global uchar *dst_addr = dst_ptr + dst_offset_first_element_in_bytes + x0 * sizeof(DATA_TYPE) + y0 * dst_stride_y;

    // Store the values across the channels
    *(__global DATA_TYPE *)(dst_addr + 0 * dst_stride_z) = out0.s0;
    *(__global DATA_TYPE *)(dst_addr + 1 * dst_stride_z) = out0.s1;
    *(__global DATA_TYPE *)(dst_addr + 2 * dst_stride_z) = out0.s2;
    *(__global DATA_TYPE *)(dst_addr + 3 * dst_stride_z) = out0.s3;
    *(__global DATA_TYPE *)(dst_addr + 4 * dst_stride_z) = out0.s4;
    *(__global DATA_TYPE *)(dst_addr + 5 * dst_stride_z) = out0.s5;
    *(__global DATA_TYPE *)(dst_addr + 6 * dst_stride_z) = out0.s6;
    *(__global DATA_TYPE *)(dst_addr + 7 * dst_stride_z) = out0.s7;

#if !defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL) && !defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
    *(__global DATA_TYPE *)(dst_addr + 8 * dst_stride_z)  = out1.s0;
    *(__global DATA_TYPE *)(dst_addr + 9 * dst_stride_z)  = out1.s1;
    *(__global DATA_TYPE *)(dst_addr + 10 * dst_stride_z) = out1.s2;
    *(__global DATA_TYPE *)(dst_addr + 11 * dst_stride_z) = out1.s3;
    *(__global DATA_TYPE *)(dst_addr + 12 * dst_stride_z) = out1.s4;
    *(__global DATA_TYPE *)(dst_addr + 13 * dst_stride_z) = out1.s5;
    *(__global DATA_TYPE *)(dst_addr + 14 * dst_stride_z) = out1.s6;
    *(__global DATA_TYPE *)(dst_addr + 15 * dst_stride_z) = out1.s7;
    *(__global DATA_TYPE *)(dst_addr + 16 * dst_stride_z) = out2.s0;
    *(__global DATA_TYPE *)(dst_addr + 17 * dst_stride_z) = out2.s1;
    *(__global DATA_TYPE *)(dst_addr + 18 * dst_stride_z) = out2.s2;
    *(__global DATA_TYPE *)(dst_addr + 19 * dst_stride_z) = out2.s3;
    *(__global DATA_TYPE *)(dst_addr + 20 * dst_stride_z) = out2.s4;
    *(__global DATA_TYPE *)(dst_addr + 21 * dst_stride_z) = out2.s5;
    *(__global DATA_TYPE *)(dst_addr + 22 * dst_stride_z) = out2.s6;
    *(__global DATA_TYPE *)(dst_addr + 23 * dst_stride_z) = out2.s7;
    *(__global DATA_TYPE *)(dst_addr + 24 * dst_stride_z) = out3.s0;
    *(__global DATA_TYPE *)(dst_addr + 25 * dst_stride_z) = out3.s1;
    *(__global DATA_TYPE *)(dst_addr + 26 * dst_stride_z) = out3.s2;
    *(__global DATA_TYPE *)(dst_addr + 27 * dst_stride_z) = out3.s3;
    *(__global DATA_TYPE *)(dst_addr + 28 * dst_stride_z) = out3.s4;
    *(__global DATA_TYPE *)(dst_addr + 29 * dst_stride_z) = out3.s5;
    *(__global DATA_TYPE *)(dst_addr + 30 * dst_stride_z) = out3.s6;
    *(__global DATA_TYPE *)(dst_addr + 31 * dst_stride_z) = out3.s7;
    *(__global DATA_TYPE *)(dst_addr + 32 * dst_stride_z) = out4.s0;
    *(__global DATA_TYPE *)(dst_addr + 33 * dst_stride_z) = out4.s1;
    *(__global DATA_TYPE *)(dst_addr + 34 * dst_stride_z) = out4.s2;
    *(__global DATA_TYPE *)(dst_addr + 35 * dst_stride_z) = out4.s3;
    *(__global DATA_TYPE *)(dst_addr + 36 * dst_stride_z) = out4.s4;
    *(__global DATA_TYPE *)(dst_addr + 37 * dst_stride_z) = out4.s5;
    *(__global DATA_TYPE *)(dst_addr + 38 * dst_stride_z) = out4.s6;
    *(__global DATA_TYPE *)(dst_addr + 39 * dst_stride_z) = out4.s7;
    *(__global DATA_TYPE *)(dst_addr + 40 * dst_stride_z) = out5.s0;
    *(__global DATA_TYPE *)(dst_addr + 41 * dst_stride_z) = out5.s1;
    *(__global DATA_TYPE *)(dst_addr + 42 * dst_stride_z) = out5.s2;
    *(__global DATA_TYPE *)(dst_addr + 43 * dst_stride_z) = out5.s3;
    *(__global DATA_TYPE *)(dst_addr + 44 * dst_stride_z) = out5.s4;
    *(__global DATA_TYPE *)(dst_addr + 45 * dst_stride_z) = out5.s5;
    *(__global DATA_TYPE *)(dst_addr + 46 * dst_stride_z) = out5.s6;
    *(__global DATA_TYPE *)(dst_addr + 47 * dst_stride_z) = out5.s7;
    *(__global DATA_TYPE *)(dst_addr + 48 * dst_stride_z) = out6.s0;
    *(__global DATA_TYPE *)(dst_addr + 49 * dst_stride_z) = out6.s1;
    *(__global DATA_TYPE *)(dst_addr + 50 * dst_stride_z) = out6.s2;
    *(__global DATA_TYPE *)(dst_addr + 51 * dst_stride_z) = out6.s3;
    *(__global DATA_TYPE *)(dst_addr + 52 * dst_stride_z) = out6.s4;
    *(__global DATA_TYPE *)(dst_addr + 53 * dst_stride_z) = out6.s5;
    *(__global DATA_TYPE *)(dst_addr + 54 * dst_stride_z) = out6.s6;
    *(__global DATA_TYPE *)(dst_addr + 55 * dst_stride_z) = out6.s7;
    *(__global DATA_TYPE *)(dst_addr + 56 * dst_stride_z) = out7.s0;
    *(__global DATA_TYPE *)(dst_addr + 57 * dst_stride_z) = out7.s1;
    *(__global DATA_TYPE *)(dst_addr + 58 * dst_stride_z) = out7.s2;
    *(__global DATA_TYPE *)(dst_addr + 59 * dst_stride_z) = out7.s3;
    *(__global DATA_TYPE *)(dst_addr + 60 * dst_stride_z) = out7.s4;
    *(__global DATA_TYPE *)(dst_addr + 61 * dst_stride_z) = out7.s5;
    *(__global DATA_TYPE *)(dst_addr + 62 * dst_stride_z) = out7.s6;
    *(__global DATA_TYPE *)(dst_addr + 63 * dst_stride_z) = out7.s7;
#endif // !defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL) && !defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
}

#endif // defined(SRC_DIM_Z)

#if defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL)
/** This OpenCL kernel performs Winograd filter transform 3x1 when the data layout is NCHW and the output tile is 2x1
 *
 * @note In order to correctly split the input tensor in batches, its dimension across the Z axis (channels for NCHW, height for NHWC) must be passed at compile time using -DSRC_DIM_Z: e.g. -DSRC_DIM_Z=64
 * @note -DWINOGRAD_FILTER_TRANSFORM_HORIZONTAL has to be passed at compile time to perform Winograd Filter Transform
 * @note The data type must be passed at compile time using -DDATA_TYPE e.g. -DDATA_TYPE=float. Supported data types: float/half.
 *
 * @param[in]  src_ptr                           Pointer to the source tensor. Supported data types: F32/F16
 * @param[in]  src_stride_x                      Stride of the source tensor in X dimension (in bytes)
 * @param[in]  src_step_x                        src_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  src_stride_y                      Stride of the source tensor in Y dimension (in bytes)
 * @param[in]  src_step_y                        src_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  src_stride_w                      Stride of the source tensor in W dimension (in bytes)
 * @param[in]  src_step_w                        src_stride_w * number of elements along W processed per workitem(in bytes)
 * @param[in]  src_offset_first_element_in_bytes The offset of the first element in the source tensor
 * @param[out] dst_ptr                           Pointer to the destination tensor. Supported data types: same as @p src_ptr
 * @param[in]  dst_stride_x                      Stride of the destination tensor in X dimension (in bytes)
 * @param[in]  dst_step_x                        dst_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  dst_stride_y                      Stride of the destination tensor in Y dimension (in bytes)
 * @param[in]  dst_step_y                        dst_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  dst_offset_first_element_in_bytes The offset of the first element in the destination tensor
 */
__kernel void winograd_filter_transform_2x1_3x1_nchw(
    TENSOR4D_DECLARATION(src),
    TENSOR3D_DECLARATION(dst))
{
    winograd_filter_transform_2x2_3x3_nchw(src_ptr,
                                           src_stride_x,
                                           src_step_x,
                                           src_stride_y,
                                           src_step_y,
                                           src_stride_z,
                                           src_step_z,
                                           src_stride_w,
                                           src_step_w,
                                           src_offset_first_element_in_bytes,
                                           dst_ptr,
                                           dst_stride_x,
                                           dst_step_x,
                                           dst_stride_y,
                                           dst_step_y,
                                           dst_stride_z,
                                           dst_step_z,
                                           dst_offset_first_element_in_bytes);
}

/** This OpenCL kernel performs Winograd filter transform 3x1 when the data layout is NCHW and the output tile is 4x1
 *
 * @note In order to correctly split the input tensor in batches, its dimension across the Z axis (channels for NCHW, height for NHWC) must be passed at compile time using -DSRC_DIM_Z: e.g. -DSRC_DIM_Z=64
 * @note -DWINOGRAD_FILTER_TRANSFORM_HORIZONTAL has to be passed at compile time to perform Winograd Filter Transform
 * @note The data type must be passed at compile time using -DDATA_TYPE e.g. -DDATA_TYPE=float. Supported data types: float/half.
 *
 * @param[in]  src_ptr                           Pointer to the source tensor. Supported data types: F32/F16
 * @param[in]  src_stride_x                      Stride of the source tensor in X dimension (in bytes)
 * @param[in]  src_step_x                        src_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  src_stride_y                      Stride of the source tensor in Y dimension (in bytes)
 * @param[in]  src_step_y                        src_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  src_stride_w                      Stride of the source tensor in W dimension (in bytes)
 * @param[in]  src_step_w                        src_stride_w * number of elements along W processed per workitem(in bytes)
 * @param[in]  src_offset_first_element_in_bytes The offset of the first element in the source tensor
 * @param[out] dst_ptr                           Pointer to the destination tensor. Supported data types: same as @p src_ptr
 * @param[in]  dst_stride_x                      Stride of the destination tensor in X dimension (in bytes)
 * @param[in]  dst_step_x                        dst_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  dst_stride_y                      Stride of the destination tensor in Y dimension (in bytes)
 * @param[in]  dst_step_y                        dst_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  dst_offset_first_element_in_bytes The offset of the first element in the destination tensor
 */
__kernel void winograd_filter_transform_4x1_3x1_nchw(
    TENSOR4D_DECLARATION(src),
    TENSOR3D_DECLARATION(dst))
{
    winograd_filter_transform_4x4_3x3_nchw(src_ptr,
                                           src_stride_x,
                                           src_step_x,
                                           src_stride_y,
                                           src_step_y,
                                           src_stride_z,
                                           src_step_z,
                                           src_stride_w,
                                           src_step_w,
                                           src_offset_first_element_in_bytes,
                                           dst_ptr,
                                           dst_stride_x,
                                           dst_step_x,
                                           dst_stride_y,
                                           dst_step_y,
                                           dst_stride_z,
                                           dst_step_z,
                                           dst_offset_first_element_in_bytes);
}

/** This OpenCL kernel performs Winograd filter transform 5x1 when the data layout is NCHW and the output tile is 4x1
 *
 * @note In order to correctly split the input tensor in batches, its dimension across the Z axis (channels for NCHW, height for NHWC) must be passed at compile time using -DSRC_DIM_Z: e.g. -DSRC_DIM_Z=64
 * @note -DWINOGRAD_FILTER_TRANSFORM_HORIZONTAL has to be passed at compile time to perform Winograd Filter Transform
 * @note The data type must be passed at compile time using -DDATA_TYPE e.g. -DDATA_TYPE=float. Supported data types: float/half.
 *
 * @param[in]  src_ptr                           Pointer to the source tensor. Supported data types: F32/F16
 * @param[in]  src_stride_x                      Stride of the source tensor in X dimension (in bytes)
 * @param[in]  src_step_x                        src_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  src_stride_y                      Stride of the source tensor in Y dimension (in bytes)
 * @param[in]  src_step_y                        src_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  src_stride_w                      Stride of the source tensor in W dimension (in bytes)
 * @param[in]  src_step_w                        src_stride_w * number of elements along W processed per workitem(in bytes)
 * @param[in]  src_offset_first_element_in_bytes The offset of the first element in the source tensor
 * @param[out] dst_ptr                           Pointer to the destination tensor. Supported data types: same as @p src_ptr
 * @param[in]  dst_stride_x                      Stride of the destination tensor in X dimension (in bytes)
 * @param[in]  dst_step_x                        dst_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  dst_stride_y                      Stride of the destination tensor in Y dimension (in bytes)
 * @param[in]  dst_step_y                        dst_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  dst_offset_first_element_in_bytes The offset of the first element in the destination tensor
 */
__kernel void winograd_filter_transform_4x1_5x1_nchw(
    TENSOR4D_DECLARATION(src),
    TENSOR3D_DECLARATION(dst))
{
    winograd_filter_transform_4x4_5x5_nchw(src_ptr,
                                           src_stride_x,
                                           src_step_x,
                                           src_stride_y,
                                           src_step_y,
                                           src_stride_z,
                                           src_step_z,
                                           src_stride_w,
                                           src_step_w,
                                           src_offset_first_element_in_bytes,
                                           dst_ptr,
                                           dst_stride_x,
                                           dst_step_x,
                                           dst_stride_y,
                                           dst_step_y,
                                           dst_stride_z,
                                           dst_step_z,
                                           dst_offset_first_element_in_bytes);
}

#endif // defined(WINOGRAD_FILTER_TRANSFORM_HORIZONTAL)

#if defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
/** This OpenCL kernel performs Winograd filter transform 1x3 when the data layout is NCHW and the output tile is 1x2
 *
 * @note In order to correctly split the input tensor in batches, its dimension across the Z axis (channels for NCHW, height for NHWC) must be passed at compile time using -DSRC_DIM_Z: e.g. -DSRC_DIM_Z=64
 * @note -DWINOGRAD_FILTER_TRANSFORM_VERTICAL has to be passed at compile time to perform Winograd Filter Transform
 * @note The data type must be passed at compile time using -DDATA_TYPE e.g. -DDATA_TYPE=float. Supported data types: float/half.
 *
 * @param[in]  src_ptr                           Pointer to the source tensor. Supported data types: F32/F16
 * @param[in]  src_stride_x                      Stride of the source tensor in X dimension (in bytes)
 * @param[in]  src_step_x                        src_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  src_stride_y                      Stride of the source tensor in Y dimension (in bytes)
 * @param[in]  src_step_y                        src_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  src_stride_w                      Stride of the source tensor in W dimension (in bytes)
 * @param[in]  src_step_w                        src_stride_w * number of elements along W processed per workitem(in bytes)
 * @param[in]  src_offset_first_element_in_bytes The offset of the first element in the source tensor
 * @param[out] dst_ptr                           Pointer to the destination tensor. Supported data types: same as @p src_ptr
 * @param[in]  dst_stride_x                      Stride of the destination tensor in X dimension (in bytes)
 * @param[in]  dst_step_x                        dst_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  dst_stride_y                      Stride of the destination tensor in Y dimension (in bytes)
 * @param[in]  dst_step_y                        dst_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  dst_offset_first_element_in_bytes The offset of the first element in the destination tensor
 */
__kernel void winograd_filter_transform_1x2_1x3_nchw(
    TENSOR4D_DECLARATION(src),
    TENSOR3D_DECLARATION(dst))
{
    winograd_filter_transform_2x2_3x3_nchw(src_ptr,
                                           src_stride_x,
                                           src_step_x,
                                           src_stride_y,
                                           src_step_y,
                                           src_stride_z,
                                           src_step_z,
                                           src_stride_w,
                                           src_step_w,
                                           src_offset_first_element_in_bytes,
                                           dst_ptr,
                                           dst_stride_x,
                                           dst_step_x,
                                           dst_stride_y,
                                           dst_step_y,
                                           dst_stride_z,
                                           dst_step_z,
                                           dst_offset_first_element_in_bytes);
}

/** This OpenCL kernel performs Winograd filter transform 1x3 when the data layout is NCHW and the output tile is 1x4
 *
 * @note In order to correctly split the input tensor in batches, its dimension across the Z axis (channels for NCHW, height for NHWC) must be passed at compile time using -DSRC_DIM_Z: e.g. -DSRC_DIM_Z=64
 * @note -DWINOGRAD_FILTER_TRANSFORM_VERTICAL has to be passed at compile time to perform Winograd Filter Transform
 * @note The data type must be passed at compile time using -DDATA_TYPE e.g. -DDATA_TYPE=float. Supported data types: float/half.
 *
 * @param[in]  src_ptr                           Pointer to the source tensor. Supported data types: F32/F16
 * @param[in]  src_stride_x                      Stride of the source tensor in X dimension (in bytes)
 * @param[in]  src_step_x                        src_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  src_stride_y                      Stride of the source tensor in Y dimension (in bytes)
 * @param[in]  src_step_y                        src_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  src_stride_w                      Stride of the source tensor in W dimension (in bytes)
 * @param[in]  src_step_w                        src_stride_w * number of elements along W processed per workitem(in bytes)
 * @param[in]  src_offset_first_element_in_bytes The offset of the first element in the source tensor
 * @param[out] dst_ptr                           Pointer to the destination tensor. Supported data types: same as @p src_ptr
 * @param[in]  dst_stride_x                      Stride of the destination tensor in X dimension (in bytes)
 * @param[in]  dst_step_x                        dst_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  dst_stride_y                      Stride of the destination tensor in Y dimension (in bytes)
 * @param[in]  dst_step_y                        dst_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  dst_offset_first_element_in_bytes The offset of the first element in the destination tensor
 */
__kernel void winograd_filter_transform_1x4_1x3_nchw(
    TENSOR4D_DECLARATION(src),
    TENSOR3D_DECLARATION(dst))
{
    winograd_filter_transform_4x4_3x3_nchw(src_ptr,
                                           src_stride_x,
                                           src_step_x,
                                           src_stride_y,
                                           src_step_y,
                                           src_stride_z,
                                           src_step_z,
                                           src_stride_w,
                                           src_step_w,
                                           src_offset_first_element_in_bytes,
                                           dst_ptr,
                                           dst_stride_x,
                                           dst_step_x,
                                           dst_stride_y,
                                           dst_step_y,
                                           dst_stride_z,
                                           dst_step_z,
                                           dst_offset_first_element_in_bytes);
}

/** This OpenCL kernel performs Winograd filter transform 1x5 when the data layout is NCHW and the output tile is 1x4
 *
 * @note In order to correctly split the input tensor in batches, its dimension across the Z axis (channels for NCHW, height for NHWC) must be passed at compile time using -DSRC_DIM_Z: e.g. -DSRC_DIM_Z=64
 * @note -DWINOGRAD_FILTER_TRANSFORM_VERTICAL has to be passed at compile time to perform Winograd Filter Transform
 * @note The data type must be passed at compile time using -DDATA_TYPE e.g. -DDATA_TYPE=float. Supported data types: float/half.
 *
 * @param[in]  src_ptr                           Pointer to the source tensor. Supported data types: F32/F16
 * @param[in]  src_stride_x                      Stride of the source tensor in X dimension (in bytes)
 * @param[in]  src_step_x                        src_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  src_stride_y                      Stride of the source tensor in Y dimension (in bytes)
 * @param[in]  src_step_y                        src_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  src_stride_w                      Stride of the source tensor in W dimension (in bytes)
 * @param[in]  src_step_w                        src_stride_w * number of elements along W processed per workitem(in bytes)
 * @param[in]  src_offset_first_element_in_bytes The offset of the first element in the source tensor
 * @param[out] dst_ptr                           Pointer to the destination tensor. Supported data types: same as @p src_ptr
 * @param[in]  dst_stride_x                      Stride of the destination tensor in X dimension (in bytes)
 * @param[in]  dst_step_x                        dst_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  dst_stride_y                      Stride of the destination tensor in Y dimension (in bytes)
 * @param[in]  dst_step_y                        dst_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                      Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                        src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  dst_offset_first_element_in_bytes The offset of the first element in the destination tensor
 */
__kernel void winograd_filter_transform_1x4_1x5_nchw(
    TENSOR4D_DECLARATION(src),
    TENSOR3D_DECLARATION(dst))
{
    winograd_filter_transform_4x4_5x5_nchw(src_ptr,
                                           src_stride_x,
                                           src_step_x,
                                           src_stride_y,
                                           src_step_y,
                                           src_stride_z,
                                           src_step_z,
                                           src_stride_w,
                                           src_step_w,
                                           src_offset_first_element_in_bytes,
                                           dst_ptr,
                                           dst_stride_x,
                                           dst_step_x,
                                           dst_stride_y,
                                           dst_step_y,
                                           dst_stride_z,
                                           dst_step_z,
                                           dst_offset_first_element_in_bytes);
}

#endif // defined(WINOGRAD_FILTER_TRANSFORM_VERTICAL)
