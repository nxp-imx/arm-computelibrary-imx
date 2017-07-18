/*
 * Copyright (c) 2016, 2017 ARM Limited.
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

#if STRIDE_X == 2
#define CONVOLVE1x3(left_pixel_position, left_coeff, middle_coeff, right_coeff) convolution1x3_stride2(left_pixel_position, left_coeff, middle_coeff, right_coeff)
#elif STRIDE_X == 1 /* STRIDE_X == 1 */
#define CONVOLVE1x3(left_pixel_position, left_coeff, middle_coeff, right_coeff) convolution1x3_stride1(left_pixel_position, left_coeff, middle_coeff, right_coeff)
#else /* STRIDE_X not equals 1 or 2 */
#error "STRIDE_X larger than 2 is not supported"
#endif /* STRIDE_X == 2 */

/** Compute a 1D horizontal convolution of size 3 with stride as 1.
 *
 * @param[in] left_pixel   Pointer to the left pixel.
 * @param[in] left_coeff   Weight of the left pixel
 * @param[in] middle_coeff Weight of the middle pixel
 * @param[in] right_coeff  Weight of the right pixel
 *
 * @return a convoluted values.
 */
inline VEC_DATA_TYPE(DATA_TYPE, 8) convolution1x3_stride1(__global const DATA_TYPE *left_pixel,
                                                          const DATA_TYPE left_coeff,
                                                          const DATA_TYPE middle_coeff,
                                                          const DATA_TYPE right_coeff)
{
    VEC_DATA_TYPE(DATA_TYPE, 16)
    temp = vload16(0, left_pixel);

    VEC_DATA_TYPE(DATA_TYPE, 8)
    left = temp.s01234567;
    VEC_DATA_TYPE(DATA_TYPE, 8)
    middle = temp.s12345678;
    VEC_DATA_TYPE(DATA_TYPE, 8)
    right = temp.s23456789;

    return left * (VEC_DATA_TYPE(DATA_TYPE, 8))left_coeff + middle * (VEC_DATA_TYPE(DATA_TYPE, 8))middle_coeff + right * (VEC_DATA_TYPE(DATA_TYPE, 8))right_coeff;
}

/** Compute a 1D horizontal convolution of size 3 with stride as 2.
 *
 * @param[in] left_pixel   Pointer to the left pixel.
 * @param[in] left_coeff   Weight of the left pixel
 * @param[in] middle_coeff Weight of the middle pixel
 * @param[in] right_coeff  Weight of the right pixel
 *
 * @return a convoluted values.
 */
inline VEC_DATA_TYPE(DATA_TYPE, 8) convolution1x3_stride2(__global const DATA_TYPE *left_pixel,
                                                          const DATA_TYPE left_coeff,
                                                          const DATA_TYPE middle_coeff,
                                                          const DATA_TYPE right_coeff)
{
    const int stride_size = 2;

    VEC_DATA_TYPE(DATA_TYPE, 16)
    temp1 = vload16(0, left_pixel);

    VEC_DATA_TYPE(DATA_TYPE, 16)
    temp2 = vload16(0, left_pixel + 8);

    VEC_DATA_TYPE(DATA_TYPE, 8)
    left = (VEC_DATA_TYPE(DATA_TYPE, 8))(temp1.s0246, temp2.s0246);

    VEC_DATA_TYPE(DATA_TYPE, 8)
    middle = (VEC_DATA_TYPE(DATA_TYPE, 8))(temp1.s1357, temp2.s1357);

    VEC_DATA_TYPE(DATA_TYPE, 8)
    right = (VEC_DATA_TYPE(DATA_TYPE, 8))(temp1.s2468, temp2.s2468);

    return left * (VEC_DATA_TYPE(DATA_TYPE, 8))left_coeff + middle * (VEC_DATA_TYPE(DATA_TYPE, 8))middle_coeff + right * (VEC_DATA_TYPE(DATA_TYPE, 8))right_coeff;
}

/** Apply a 3x3 2D convolution matrix on the input and return the result.
 *
 * Convolution matrix layout:
 *
 * [ mat0, mat1, mat2 ]\n
 * [ mat3, mat4, mat5 ]\n
 * [ mat6, mat7, mat8 ]\n
 *
 * @param[in] src  A pointer to source Image structure
 * @param[in] mat0 Coefficient from the convolution matrix
 * @param[in] mat1 Coefficient from the convolution matrix
 * @param[in] mat2 Coefficient from the convolution matrix
 * @param[in] mat3 Coefficient from the convolution matrix
 * @param[in] mat4 Coefficient from the convolution matrix
 * @param[in] mat5 Coefficient from the convolution matrix
 * @param[in] mat6 Coefficient from the convolution matrix
 * @param[in] mat0 Coefficient from the convolution matrix
 * @param[in] mat7 Coefficient from the convolution matrix
 * @param[in] mat8 Coefficient from the convolution matrix
 *
 * @return convoluted values.
 */
inline VEC_DATA_TYPE(DATA_TYPE, 8) convolution3x3(
    Image          *src,
    const DATA_TYPE mat0, const DATA_TYPE mat1, const DATA_TYPE mat2,
    const DATA_TYPE mat3, const DATA_TYPE mat4, const DATA_TYPE mat5,
    const DATA_TYPE mat6, const DATA_TYPE mat7, const DATA_TYPE mat8)
{
    // Output pixels
    VEC_DATA_TYPE(DATA_TYPE, 8)
    pixels;

    // Row 0
    pixels = CONVOLVE1x3((__global DATA_TYPE *)offset(src, 0, 0), mat0, mat1, mat2);
    // Row
    pixels += CONVOLVE1x3((__global DATA_TYPE *)offset(src, 0, 1), mat3, mat4, mat5);
    // Row 2
    pixels += CONVOLVE1x3((__global DATA_TYPE *)offset(src, 0, 2), mat6, mat7, mat8);

    return pixels;
}

/** This kernel performs a direct convolution to convolve the low three dimensions.
 *
 * @note The data type must be passed at compile time using -DDATA_TYPE: e.g. -DDATA_TYPE=float
 * @note The convolution stride x and stride y must be passed at compile time using -DSTRIDE_X and -DSTRIDE_Y: e.g. -DSTRIDE_X=1, _DSTRIDE_Y=1
 * @note In case biases will be added to the convolution -DHAS_BIAS has to be passed to append the final matrix with 1 in each row.
 *
 * @param[in]  src_ptr                               Pointer to the source tensor. Supported data types: QS8/F16/F32
 * @param[in]  src_stride_x                          Stride of the source tensor in X dimension (in bytes)
 * @param[in]  src_step_x                            src_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  src_stride_y                          Stride of the source tensor in Y dimension (in bytes)
 * @param[in]  src_step_y                            src_stride_y * number of elements along Y processed per workitem(in bytes)
 * @param[in]  src_stride_z                          Stride of the source tensor in Z dimension (in bytes)
 * @param[in]  src_step_z                            src_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  src_offset_first_element_in_bytes     The offset of the first element in the source tensor
 * @param[out] dst_ptr                               Pointer to the destination tensor. Supported data types: same as @p src_ptr
 * @param[in]  dst_stride_x                          Stride of the destination tensor in X dimension (in bytes)
 * @param[in]  dst_step_x                            dst_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  dst_stride_y                          Stride of the destination tensor in Y dimension (in bytes)
 * @param[in]  dst_step_y                            dst_stride_y * number of elements along Z processed per workitem(in bytes)
 * @param[in]  dst_stride_z                          Stride of the destination tensor in Z dimension (in bytes)
 * @param[in]  dst_step_z                            dst_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  dst_offset_first_element_in_bytes     The offset of the first element in the destination tensor
 * @param[out] weights_ptr                           Pointer to the weights tensor. Supported data types: same as @p weights_ptr
 * @param[in]  weights_stride_x                      Stride of the weights tensor in X dimension (in bytes)
 * @param[in]  weights_step_x                        weights_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  weights_stride_y                      Stride of the weights tensor in Y dimension (in bytes)
 * @param[in]  weights_step_y                        weights_stride_y * number of elements along y processed per workitem(in bytes)
 * @param[in]  weights_stride_z                      Stride of the weights tensor in Z dimension (in bytes)
 * @param[in]  weights_step_z                        weights_stride_z * number of elements along Z processed per workitem(in bytes)
 * @param[in]  weights_offset_first_element_in_bytes The offset of the first element in the weights tensor
 * @param[in]  biases_ptr                            Pointer to the biases tensor. Same as @p src_ptr
 * @param[in]  biases_stride_x                       Stride of the biases tensor in X dimension (in bytes)
 * @param[in]  biases_step_x                         biases_stride_x * number of elements along X processed per workitem(in bytes)
 * @param[in]  biases_offset_first_element_in_bytes  The offset of the first element in the biases tensor
 * @param[in]  weights_stride_w                      Stride of the weights tensor in W dimension
 * @param[in]  filter_depth                          The depth size of the filter
 */
__kernel void direct_convolution3x3(
    TENSOR3D_DECLARATION(src),
    TENSOR3D_DECLARATION(dst),
    TENSOR3D_DECLARATION(weights),
#ifdef HAS_BIAS
    VECTOR_DECLARATION(biases),
#endif /* defined(HAS_BIAS) */
    unsigned int weights_stride_w,
    unsigned int filter_depth)
{
    Image    src     = CONVERT_TO_IMAGE_STRUCT(src);
    Tensor3D weights = CONVERT_TO_TENSOR3D_STRUCT_NO_STEP(weights);
    Tensor3D dst     = CONVERT_TO_TENSOR3D_STRUCT(dst);

#ifdef HAS_BIAS
    Vector biases = CONVERT_TO_VECTOR_STRUCT_NO_STEP(biases);
#endif /* defined(HAS_BIAS) */

    VEC_DATA_TYPE(DATA_TYPE, 8)
    pixels = 0;

    const uint z_index = get_global_id(2);

    weights.ptr += z_index * weights_stride_w;

    for(int d = 0; d < filter_depth; ++d)
    {
        VEC_DATA_TYPE(DATA_TYPE, 4)
        weights_row1 = vload4(0, (__global DATA_TYPE *)tensor3D_offset(&weights, 0, 0, 0));
        VEC_DATA_TYPE(DATA_TYPE, 4)
        weights_row2 = vload4(0, (__global DATA_TYPE *)tensor3D_offset(&weights, 0, 1, 0));
        VEC_DATA_TYPE(DATA_TYPE, 4)
        weights_row3 = vload4(0, (__global DATA_TYPE *)tensor3D_offset(&weights, 0, 2, 0));

        pixels += convolution3x3(&src, weights_row1.s0,
                                 weights_row1.s1,
                                 weights_row1.s2,
                                 weights_row2.s0,
                                 weights_row2.s1,
                                 weights_row2.s2,
                                 weights_row3.s0,
                                 weights_row3.s1,
                                 weights_row3.s2);

        src.ptr += src_stride_z;
        weights.ptr += weights_stride_z;
    }

#ifdef HAS_BIAS
    pixels += (VEC_DATA_TYPE(DATA_TYPE, 8)) * ((__global DATA_TYPE *)(vector_offset(&biases, z_index)));
#endif /* defined(HAS_BIAS) */

    vstore8(pixels, 0, (__global DATA_TYPE *)dst.ptr);
}
