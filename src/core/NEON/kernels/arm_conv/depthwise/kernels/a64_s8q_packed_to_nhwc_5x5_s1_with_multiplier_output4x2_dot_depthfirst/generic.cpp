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


#include "arm_gemm.hpp"
#include <cstddef>
#include <cstdint>

namespace arm_conv {
namespace depthwise {

void a64_s8q_packed_to_nhwc_5x5_s1_with_multiplier_output4x2_dot_depthfirst_impl(
  const int8_t *const *const inptrs,
  int8_t *const *const outptrs,
  const void *params,
  unsigned int n_output_channels,
  const arm_gemm::Requantize32& qp
)
{
  __asm__ __volatile__(
    "movi v15.16b, #0x1\n"
    "ldr x21, [%x[inptrs], #0x0]\n"
    "add SP, SP, #-0x80\n"
    "movi v14.4s, #0x1\n"
    "ldr x20, [%x[inptrs], #0x8]\n"
    "add x22, %x[qp], %[offsetof_Requantize32_b_offset]\n"
    "movi v28.4s, #0x0\n"
    "ldr x19, [%x[inptrs], #0x10]\n"
    "mov x11, #0x0\n"
    "movi v27.4s, #0x0\n"
    "ld1 { v13.16b }, [x21]\n"
    "mov x10, #0x0\n"
    "movi v26.4s, #0x0\n"
    "ld1 { v12.16b }, [x20]\n"
    "add x9, %x[qp], %[offsetof_Requantize32_c_offset]\n"
    "movi v25.4s, #0x0\n"
    "ld1 { v7.16b }, [x19]\n"
    "add x28, %x[qp], %[offsetof_Requantize32_minval]\n"
    "movi v24.4s, #0x0\n"
    "ldr x21, [%x[inptrs], #0x18]\n"
    "add x27, %x[qp], %[offsetof_Requantize32_maxval]\n"
    "mov v18.16b, v13.16b\n"
    "ldr x20, [%x[inptrs], #0x20]\n"
    "cmp %x[n_channels], #0x4\n"
    "ext v18.16b, v18.16b, v18.16b, #0x1\n"
    "ldr x19, [%x[inptrs], #0x28]\n"
    "mov v17.16b, v12.16b\n"
    "ld1 { v6.16b }, [x21]\n"
    "ext v17.16b, v17.16b, v17.16b, #0x1\n"
    "ld1 { v5.16b }, [x20]\n"
    "mov v16.16b, v7.16b\n"
    "ld1 { v4.16b }, [x19]\n"
    "ext v16.16b, v16.16b, v16.16b, #0x1\n"
    "ldr x20, [%x[inptrs], #0x30]\n"
    "zip1 v13.2d, v13.2d, v18.2d\n"
    "ldr x19, [%x[inptrs], #0x38]\n"
    "zip1 v12.2d, v12.2d, v17.2d\n"
    "ld1r { v3.4s }, [x22]\n"
    "mov v18.16b, v6.16b\n"
    "ld1 { v2.16b }, [x20]\n"
    "zip1 v7.2d, v7.2d, v16.2d\n"
    "ld1 { v1.16b }, [x19]\n"
    "ext v18.16b, v18.16b, v18.16b, #0x1\n"
    "ldp x26, x25, [%x[outptrs], #0x0]\n"
    "mov v17.16b, v5.16b\n"
    "ldp x24, x23, [%x[outptrs], #0x10]\n"
    "ext v17.16b, v17.16b, v17.16b, #0x1\n"
    "ldp x22, x21, [%x[outptrs], #0x20]\n"
    "mov v16.16b, v4.16b\n"
    "ldp x20, x19, [%x[outptrs], #0x30]\n"
    "zip1 v6.2d, v6.2d, v18.2d\n"
    "ld1r { v0.4s }, [x9]\n"
    "ext v16.16b, v16.16b, v16.16b, #0x1\n"
    "ld1r { v31.4s }, [x28]\n"
    "zip1 v5.2d, v5.2d, v17.2d\n"
    "ld1r { v30.4s }, [x27]\n"
    "mov v17.16b, v2.16b\n"
    "ldr q29, [%x[params], #0x0]\n"
    "ext v17.16b, v17.16b, v17.16b, #0x1\n"
    "ldr q8, [%x[params], #0x10]\n"
    "zip1 v4.2d, v4.2d, v16.2d\n"
    "ldr q9, [%x[params], #0x20]\n"
    "mov v16.16b, v1.16b\n"
    "ldr q10, [%x[params], #0x30]\n"
    "ext v16.16b, v16.16b, v16.16b, #0x1\n"
    "ldr q11, [%x[params], #0x40]\n"
    "add %x[params], %x[params], #0x50\n"
    "zip1 v2.2d, v2.2d, v17.2d\n"
    "movi v23.4s, #0x0\n"
    "movi v22.4s, #0x0\n"
    "zip1 v1.2d, v1.2d, v16.2d\n"
    "movi v21.4s, #0x0\n"
    "movi v18.4s, #0x0\n"
    "movi v17.4s, #0x0\n"
    "movi v16.4s, #0x0\n"
    "movi v20.4s, #0x0\n"
    "movi v19.4s, #0x0\n"
    ".inst 0x4f8de1fc  // sdot v28.4s, v15.16b, v13.4b[0]\n"
    ".inst 0x4f8de9fb  // sdot v27.4s, v15.16b, v13.4b[2]\n"
    ".inst 0x4f8ce1fa  // sdot v26.4s, v15.16b, v12.4b[0]\n"
    ".inst 0x4f8ce9f9  // sdot v25.4s, v15.16b, v12.4b[2]\n"
    ".inst 0x4fade1dc  // sdot v28.4s, v14.16b, v13.4b[1]\n"
    ".inst 0x4fade9db  // sdot v27.4s, v14.16b, v13.4b[3]\n"
    ".inst 0x4face1da  // sdot v26.4s, v14.16b, v12.4b[1]\n"
    ".inst 0x4face9d9  // sdot v25.4s, v14.16b, v12.4b[3]\n"
    ".inst 0x4f87e1f8  // sdot v24.4s, v15.16b, v7.4b[0]\n"
    ".inst 0x4f87e9f7  // sdot v23.4s, v15.16b, v7.4b[2]\n"
    ".inst 0x4f86e1f6  // sdot v22.4s, v15.16b, v6.4b[0]\n"
    ".inst 0x4f86e9f5  // sdot v21.4s, v15.16b, v6.4b[2]\n"
    ".inst 0x4fa7e1d8  // sdot v24.4s, v14.16b, v7.4b[1]\n"
    ".inst 0x4fa7e9d7  // sdot v23.4s, v14.16b, v7.4b[3]\n"
    ".inst 0x4fa6e1d6  // sdot v22.4s, v14.16b, v6.4b[1]\n"
    ".inst 0x4fa6e9d5  // sdot v21.4s, v14.16b, v6.4b[3]\n"
    ".inst 0x4f85e1f2  // sdot v18.4s, v15.16b, v5.4b[0]\n"
    ".inst 0x4f85e9f1  // sdot v17.4s, v15.16b, v5.4b[2]\n"
    ".inst 0x4f84e1f0  // sdot v16.4s, v15.16b, v4.4b[0]\n"
    ".inst 0x4f84e9f4  // sdot v20.4s, v15.16b, v4.4b[2]\n"
    ".inst 0x4fa5e1d2  // sdot v18.4s, v14.16b, v5.4b[1]\n"
    ".inst 0x4fa5e9d1  // sdot v17.4s, v14.16b, v5.4b[3]\n"
    ".inst 0x4fa4e1d0  // sdot v16.4s, v14.16b, v4.4b[1]\n"
    ".inst 0x4fa4e9d4  // sdot v20.4s, v14.16b, v4.4b[3]\n"
    ".inst 0x4f82e1f3  // sdot v19.4s, v15.16b, v2.4b[0]\n"
    "mov v28.16b, v28.16b\n"
    "mov v27.16b, v27.16b\n"
    "add v28.4s, v28.4s, v26.4s\n"
    ".inst 0x4fa2e1d3  // sdot v19.4s, v14.16b, v2.4b[1]\n"
    "add v27.4s, v27.4s, v25.4s\n"
    "add v28.4s, v28.4s, v24.4s\n"
    "mov v26.16b, v26.16b\n"
    "add v27.4s, v27.4s, v23.4s\n"
    "add v28.4s, v28.4s, v22.4s\n"
    "mov v25.16b, v25.16b\n"
    "add v27.4s, v27.4s, v21.4s\n"
    "add v28.4s, v28.4s, v18.4s\n"
    "add v26.4s, v26.4s, v24.4s\n"
    "add v27.4s, v27.4s, v17.4s\n"
    "add v25.4s, v25.4s, v23.4s\n"
    "add v26.4s, v26.4s, v22.4s\n"
    "mov v24.16b, v24.16b\n"
    "add v25.4s, v25.4s, v21.4s\n"
    "add v26.4s, v26.4s, v18.4s\n"
    "mov v23.16b, v23.16b\n"
    "add v25.4s, v25.4s, v17.4s\n"
    "add v26.4s, v26.4s, v16.4s\n"
    "add v24.4s, v24.4s, v22.4s\n"
    "add v25.4s, v25.4s, v20.4s\n"
    "add v23.4s, v23.4s, v21.4s\n"
    "add v24.4s, v24.4s, v18.4s\n"
    "mov v22.16b, v22.16b\n"
    "add v23.4s, v23.4s, v17.4s\n"
    "add v24.4s, v24.4s, v16.4s\n"
    "mov v21.16b, v21.16b\n"
    "add v23.4s, v23.4s, v20.4s\n"
    "add v24.4s, v24.4s, v19.4s\n"
    "add v22.4s, v22.4s, v18.4s\n"
    "movi v18.4s, #0x0\n"
    ".inst 0x4f82e9f2  // sdot v18.4s, v15.16b, v2.4b[2]\n"
    "add v21.4s, v21.4s, v17.4s\n"
    "movi v17.4s, #0x0\n"
    ".inst 0x4f81e1f1  // sdot v17.4s, v15.16b, v1.4b[0]\n"
    ".inst 0x4fa2e9d2  // sdot v18.4s, v14.16b, v2.4b[3]\n"
    "add v22.4s, v22.4s, v16.4s\n"
    "movi v16.4s, #0x0\n"
    ".inst 0x4fa1e1d1  // sdot v17.4s, v14.16b, v1.4b[1]\n"
    ".inst 0x4f81e9f0  // sdot v16.4s, v15.16b, v1.4b[2]\n"
    "add v23.4s, v23.4s, v18.4s\n"
    "add v21.4s, v21.4s, v20.4s\n"
    "add v22.4s, v22.4s, v19.4s\n"
    ".inst 0x4fa1e9d0  // sdot v16.4s, v14.16b, v1.4b[3]\n"
    "add v21.4s, v21.4s, v18.4s\n"
    "add v22.4s, v22.4s, v17.4s\n"
    "neg v3.4s, v3.4s\n"
    "add v21.4s, v21.4s, v16.4s\n"
    "mul v28.4s, v28.4s, v3.4s\n"
    "str q28, [SP, #0x0]\n"
    "mul v27.4s, v27.4s, v3.4s\n"
    "mul v26.4s, v26.4s, v3.4s\n"
    "str q27, [SP, #0x10]\n"
    "mul v25.4s, v25.4s, v3.4s\n"
    "mul v24.4s, v24.4s, v3.4s\n"
    "str q26, [SP, #0x20]\n"
    "mul v23.4s, v23.4s, v3.4s\n"
    "str q25, [SP, #0x30]\n"
    "mul v22.4s, v22.4s, v3.4s\n"
    "mul v21.4s, v21.4s, v3.4s\n"
    "str q24, [SP, #0x40]\n"
    "add v28.4s, v28.4s, v29.4s\n"
    "str q23, [SP, #0x50]\n"
    "add v27.4s, v27.4s, v29.4s\n"
    "str q22, [SP, #0x60]\n"
    "add v26.4s, v26.4s, v29.4s\n"
    "add v25.4s, v25.4s, v29.4s\n"
    "str q21, [SP, #0x70]\n"
    "add v24.4s, v24.4s, v29.4s\n"
    "add v23.4s, v23.4s, v29.4s\n"
    "add v22.4s, v22.4s, v29.4s\n"
    "add v21.4s, v21.4s, v29.4s\n"
    "ble 2f\n"
    "1:"  // Loop
    ".inst 0x4f8de11c  // sdot v28.4s, v8.16b, v13.4b[0]\n"
    "ldr q20, [%x[params], #0x60]\n"
    "add x11, x11, #0x10\n"
    ".inst 0x4f8de91b  // sdot v27.4s, v8.16b, v13.4b[2]\n"
    "ldr q19, [%x[params], #0x70]\n"
    "sub %x[n_channels], %x[n_channels], #0x4\n"
    ".inst 0x4f8ce11a  // sdot v26.4s, v8.16b, v12.4b[0]\n"
    "ldr q29, [%x[params], #0x80]\n"
    "cmp %x[n_channels], #0x4\n"
    ".inst 0x4f8ce919  // sdot v25.4s, v8.16b, v12.4b[2]\n"
    ".inst 0x4f87e118  // sdot v24.4s, v8.16b, v7.4b[0]\n"
    ".inst 0x4f87e917  // sdot v23.4s, v8.16b, v7.4b[2]\n"
    ".inst 0x4f86e116  // sdot v22.4s, v8.16b, v6.4b[0]\n"
    ".inst 0x4f86e915  // sdot v21.4s, v8.16b, v6.4b[2]\n"
    "ldr q8, [%x[params], #0x0]\n"
    ".inst 0x4fade13c  // sdot v28.4s, v9.16b, v13.4b[1]\n"
    ".inst 0x4fade93b  // sdot v27.4s, v9.16b, v13.4b[3]\n"
    ".inst 0x4face13a  // sdot v26.4s, v9.16b, v12.4b[1]\n"
    ".inst 0x4face939  // sdot v25.4s, v9.16b, v12.4b[3]\n"
    ".inst 0x4fa7e138  // sdot v24.4s, v9.16b, v7.4b[1]\n"
    ".inst 0x4fa7e937  // sdot v23.4s, v9.16b, v7.4b[3]\n"
    ".inst 0x4fa6e136  // sdot v22.4s, v9.16b, v6.4b[1]\n"
    ".inst 0x4fa6e935  // sdot v21.4s, v9.16b, v6.4b[3]\n"
    "ldr q9, [%x[params], #0x10]\n"
    ".inst 0x4f8ce15c  // sdot v28.4s, v10.16b, v12.4b[0]\n"
    ".inst 0x4f8ce95b  // sdot v27.4s, v10.16b, v12.4b[2]\n"
    ".inst 0x4f87e15a  // sdot v26.4s, v10.16b, v7.4b[0]\n"
    ".inst 0x4f87e959  // sdot v25.4s, v10.16b, v7.4b[2]\n"
    ".inst 0x4f86e158  // sdot v24.4s, v10.16b, v6.4b[0]\n"
    ".inst 0x4f86e957  // sdot v23.4s, v10.16b, v6.4b[2]\n"
    ".inst 0x4f85e156  // sdot v22.4s, v10.16b, v5.4b[0]\n"
    ".inst 0x4f85e955  // sdot v21.4s, v10.16b, v5.4b[2]\n"
    "ldr q10, [%x[params], #0x20]\n"
    ".inst 0x4face17c  // sdot v28.4s, v11.16b, v12.4b[1]\n"
    ".inst 0x4face97b  // sdot v27.4s, v11.16b, v12.4b[3]\n"
    ".inst 0x4fa7e17a  // sdot v26.4s, v11.16b, v7.4b[1]\n"
    ".inst 0x4fa7e979  // sdot v25.4s, v11.16b, v7.4b[3]\n"
    ".inst 0x4fa6e178  // sdot v24.4s, v11.16b, v6.4b[1]\n"
    ".inst 0x4fa6e977  // sdot v23.4s, v11.16b, v6.4b[3]\n"
    ".inst 0x4fa5e176  // sdot v22.4s, v11.16b, v5.4b[1]\n"
    ".inst 0x4fa5e975  // sdot v21.4s, v11.16b, v5.4b[3]\n"
    "ldr q11, [%x[params], #0x30]\n"
    ".inst 0x4f87e11c  // sdot v28.4s, v8.16b, v7.4b[0]\n"
    ".inst 0x4f87e91b  // sdot v27.4s, v8.16b, v7.4b[2]\n"
    ".inst 0x4f86e11a  // sdot v26.4s, v8.16b, v6.4b[0]\n"
    ".inst 0x4f86e919  // sdot v25.4s, v8.16b, v6.4b[2]\n"
    ".inst 0x4f85e118  // sdot v24.4s, v8.16b, v5.4b[0]\n"
    ".inst 0x4f85e917  // sdot v23.4s, v8.16b, v5.4b[2]\n"
    ".inst 0x4f84e116  // sdot v22.4s, v8.16b, v4.4b[0]\n"
    ".inst 0x4f84e915  // sdot v21.4s, v8.16b, v4.4b[2]\n"
    "ldr q8, [%x[params], #0x40]\n"
    ".inst 0x4fa7e13c  // sdot v28.4s, v9.16b, v7.4b[1]\n"
    ".inst 0x4fa7e93b  // sdot v27.4s, v9.16b, v7.4b[3]\n"
    ".inst 0x4fa6e13a  // sdot v26.4s, v9.16b, v6.4b[1]\n"
    ".inst 0x4fa6e939  // sdot v25.4s, v9.16b, v6.4b[3]\n"
    ".inst 0x4fa5e138  // sdot v24.4s, v9.16b, v5.4b[1]\n"
    ".inst 0x4fa5e937  // sdot v23.4s, v9.16b, v5.4b[3]\n"
    ".inst 0x4fa4e136  // sdot v22.4s, v9.16b, v4.4b[1]\n"
    ".inst 0x4fa4e935  // sdot v21.4s, v9.16b, v4.4b[3]\n"
    "ldr q9, [%x[params], #0x50]\n"
    ".inst 0x4f86e15c  // sdot v28.4s, v10.16b, v6.4b[0]\n"
    ".inst 0x4f86e95b  // sdot v27.4s, v10.16b, v6.4b[2]\n"
    ".inst 0x4f85e15a  // sdot v26.4s, v10.16b, v5.4b[0]\n"
    ".inst 0x4f85e959  // sdot v25.4s, v10.16b, v5.4b[2]\n"
    ".inst 0x4f84e158  // sdot v24.4s, v10.16b, v4.4b[0]\n"
    ".inst 0x4f84e957  // sdot v23.4s, v10.16b, v4.4b[2]\n"
    ".inst 0x4f82e156  // sdot v22.4s, v10.16b, v2.4b[0]\n"
    ".inst 0x4f82e955  // sdot v21.4s, v10.16b, v2.4b[2]\n"
    "ldr q10, [%x[params], #0xb0]\n"
    ".inst 0x4fa6e17c  // sdot v28.4s, v11.16b, v6.4b[1]\n"
    ".inst 0x4fa6e97b  // sdot v27.4s, v11.16b, v6.4b[3]\n"
    ".inst 0x4fa5e17a  // sdot v26.4s, v11.16b, v5.4b[1]\n"
    ".inst 0x4fa5e979  // sdot v25.4s, v11.16b, v5.4b[3]\n"
    ".inst 0x4fa4e178  // sdot v24.4s, v11.16b, v4.4b[1]\n"
    ".inst 0x4fa4e977  // sdot v23.4s, v11.16b, v4.4b[3]\n"
    ".inst 0x4fa2e176  // sdot v22.4s, v11.16b, v2.4b[1]\n"
    ".inst 0x4fa2e975  // sdot v21.4s, v11.16b, v2.4b[3]\n"
    "ldr q11, [%x[params], #0xc0]\n"
    ".inst 0x4f85e11c  // sdot v28.4s, v8.16b, v5.4b[0]\n"
    ".inst 0x4f85e91b  // sdot v27.4s, v8.16b, v5.4b[2]\n"
    ".inst 0x4f84e11a  // sdot v26.4s, v8.16b, v4.4b[0]\n"
    ".inst 0x4f84e919  // sdot v25.4s, v8.16b, v4.4b[2]\n"
    ".inst 0x4f82e118  // sdot v24.4s, v8.16b, v2.4b[0]\n"
    ".inst 0x4f82e917  // sdot v23.4s, v8.16b, v2.4b[2]\n"
    ".inst 0x4f81e116  // sdot v22.4s, v8.16b, v1.4b[0]\n"
    ".inst 0x4f81e915  // sdot v21.4s, v8.16b, v1.4b[2]\n"
    "ldr q8, [%x[params], #0x90]\n"
    ".inst 0x4fa5e13c  // sdot v28.4s, v9.16b, v5.4b[1]\n"
    ".inst 0x4fa5e93b  // sdot v27.4s, v9.16b, v5.4b[3]\n"
    ".inst 0x4fa4e13a  // sdot v26.4s, v9.16b, v4.4b[1]\n"
    ".inst 0x4fa4e939  // sdot v25.4s, v9.16b, v4.4b[3]\n"
    ".inst 0x4fa2e138  // sdot v24.4s, v9.16b, v2.4b[1]\n"
    ".inst 0x4fa2e937  // sdot v23.4s, v9.16b, v2.4b[3]\n"
    ".inst 0x4fa1e136  // sdot v22.4s, v9.16b, v1.4b[1]\n"
    ".inst 0x4fa1e935  // sdot v21.4s, v9.16b, v1.4b[3]\n"
    "ldr q9, [%x[params], #0xa0]\n"
    "add %x[params], %x[params], #0xd0\n"
    "sqrdmulh v28.4s, v28.4s, v20.4s\n"
    "sqrdmulh v27.4s, v27.4s, v20.4s\n"
    "sqrdmulh v26.4s, v26.4s, v20.4s\n"
    "sqrdmulh v25.4s, v25.4s, v20.4s\n"
    "sqrdmulh v24.4s, v24.4s, v20.4s\n"
    "and v18.16b, v28.16b, v19.16b\n"
    "and v17.16b, v27.16b, v19.16b\n"
    "and v16.16b, v26.16b, v19.16b\n"
    "sshr v18.4s, v18.4s, #0x1f\n"
    "sshr v17.4s, v17.4s, #0x1f\n"
    "sshr v16.4s, v16.4s, #0x1f\n"
    "sqadd v28.4s, v28.4s, v18.4s\n"
    "sqadd v27.4s, v27.4s, v17.4s\n"
    "sqadd v26.4s, v26.4s, v16.4s\n"
    "and v16.16b, v25.16b, v19.16b\n"
    "srshl v28.4s, v28.4s, v19.4s\n"
    "srshl v27.4s, v27.4s, v19.4s\n"
    "srshl v26.4s, v26.4s, v19.4s\n"
    "sshr v16.4s, v16.4s, #0x1f\n"
    "add v28.4s, v28.4s, v0.4s\n"
    "add v27.4s, v27.4s, v0.4s\n"
    "add v26.4s, v26.4s, v0.4s\n"
    "smin v28.4s, v28.4s, v30.4s\n"
    "smin v27.4s, v27.4s, v30.4s\n"
    "smin v26.4s, v26.4s, v30.4s\n"
    "smax v28.4s, v28.4s, v31.4s\n"
    "smax v27.4s, v27.4s, v31.4s\n"
    "smax v26.4s, v26.4s, v31.4s\n"
    "uzp1 v28.16b, v28.16b, v28.16b\n"
    "uzp1 v27.16b, v27.16b, v27.16b\n"
    "uzp1 v28.16b, v28.16b, v28.16b\n"
    "str s28, [x26, x10]\n"
    "uzp1 v27.16b, v27.16b, v27.16b\n"
    "uzp1 v26.16b, v26.16b, v26.16b\n"
    "ldr q28, [SP, #0x0]\n"
    "sqadd v25.4s, v25.4s, v16.4s\n"
    "str s27, [x25, x10]\n"
    "uzp1 v26.16b, v26.16b, v26.16b\n"
    "ldr q27, [SP, #0x10]\n"
    "and v16.16b, v24.16b, v19.16b\n"
    "str s26, [x24, x10]\n"
    "sqrdmulh v23.4s, v23.4s, v20.4s\n"
    "ldr q26, [SP, #0x20]\n"
    "srshl v25.4s, v25.4s, v19.4s\n"
    "sshr v16.4s, v16.4s, #0x1f\n"
    "sqrdmulh v22.4s, v22.4s, v20.4s\n"
    "and v17.16b, v23.16b, v19.16b\n"
    "add v25.4s, v25.4s, v0.4s\n"
    "sqadd v24.4s, v24.4s, v16.4s\n"
    "sshr v17.4s, v17.4s, #0x1f\n"
    "smin v25.4s, v25.4s, v30.4s\n"
    "and v16.16b, v22.16b, v19.16b\n"
    "srshl v24.4s, v24.4s, v19.4s\n"
    "smax v25.4s, v25.4s, v31.4s\n"
    "sqadd v23.4s, v23.4s, v17.4s\n"
    "sshr v16.4s, v16.4s, #0x1f\n"
    "uzp1 v25.16b, v25.16b, v25.16b\n"
    "add v24.4s, v24.4s, v0.4s\n"
    "uzp1 v25.16b, v25.16b, v25.16b\n"
    "str s25, [x23, x10]\n"
    "smin v24.4s, v24.4s, v30.4s\n"
    "srshl v23.4s, v23.4s, v19.4s\n"
    "ldr q25, [SP, #0x30]\n"
    "sqadd v22.4s, v22.4s, v16.4s\n"
    "sqrdmulh v21.4s, v21.4s, v20.4s\n"
    "smax v24.4s, v24.4s, v31.4s\n"
    "add v23.4s, v23.4s, v0.4s\n"
    "srshl v22.4s, v22.4s, v19.4s\n"
    "uzp1 v24.16b, v24.16b, v24.16b\n"
    "smin v23.4s, v23.4s, v30.4s\n"
    "uzp1 v24.16b, v24.16b, v24.16b\n"
    "str s24, [x22, x10]\n"
    "smax v23.4s, v23.4s, v31.4s\n"
    "add v22.4s, v22.4s, v0.4s\n"
    "ldr q24, [SP, #0x40]\n"
    "and v16.16b, v21.16b, v19.16b\n"
    "add v28.4s, v28.4s, v29.4s\n"
    "uzp1 v23.16b, v23.16b, v23.16b\n"
    "smin v22.4s, v22.4s, v30.4s\n"
    "uzp1 v23.16b, v23.16b, v23.16b\n"
    "str s23, [x21, x10]\n"
    "smax v22.4s, v22.4s, v31.4s\n"
    "sshr v16.4s, v16.4s, #0x1f\n"
    "ldr q23, [SP, #0x50]\n"
    "add v27.4s, v27.4s, v29.4s\n"
    "add v26.4s, v26.4s, v29.4s\n"
    "uzp1 v22.16b, v22.16b, v22.16b\n"
    "sqadd v21.4s, v21.4s, v16.4s\n"
    "uzp1 v22.16b, v22.16b, v22.16b\n"
    "str s22, [x20, x10]\n"
    "add v25.4s, v25.4s, v29.4s\n"
    "add v24.4s, v24.4s, v29.4s\n"
    "ldr q22, [SP, #0x60]\n"
    "srshl v21.4s, v21.4s, v19.4s\n"
    "add v23.4s, v23.4s, v29.4s\n"
    "add v21.4s, v21.4s, v0.4s\n"
    "add v22.4s, v22.4s, v29.4s\n"
    "smin v21.4s, v21.4s, v30.4s\n"
    "smax v21.4s, v21.4s, v31.4s\n"
    "uzp1 v21.16b, v21.16b, v21.16b\n"
    "uzp1 v21.16b, v21.16b, v21.16b\n"
    "str s21, [x19, x10]\n"
    "add x10, x10, #0x4\n"
    "ldr q21, [SP, #0x70]\n"
    "add v21.4s, v21.4s, v29.4s\n"
    "bgt 1b\n"
    "2:"  // Tail
    ".inst 0x4f8de11c  // sdot v28.4s, v8.16b, v13.4b[0]\n"
    "ldr q20, [%x[params], #0x60]\n"
    "add x26, x26, x10\n"
    ".inst 0x4f8de91b  // sdot v27.4s, v8.16b, v13.4b[2]\n"
    "ldr q19, [%x[params], #0x70]\n"
    "add x25, x25, x10\n"
    ".inst 0x4f8ce11a  // sdot v26.4s, v8.16b, v12.4b[0]\n"
    "add x24, x24, x10\n"
    ".inst 0x4f8ce919  // sdot v25.4s, v8.16b, v12.4b[2]\n"
    "add x23, x23, x10\n"
    ".inst 0x4f87e118  // sdot v24.4s, v8.16b, v7.4b[0]\n"
    "add x22, x22, x10\n"
    ".inst 0x4f87e917  // sdot v23.4s, v8.16b, v7.4b[2]\n"
    "add x21, x21, x10\n"
    ".inst 0x4f86e116  // sdot v22.4s, v8.16b, v6.4b[0]\n"
    "add x20, x20, x10\n"
    ".inst 0x4f86e915  // sdot v21.4s, v8.16b, v6.4b[2]\n"
    "ldr q8, [%x[params], #0x0]\n"
    "add x19, x19, x10\n"
    ".inst 0x4fade13c  // sdot v28.4s, v9.16b, v13.4b[1]\n"
    "cmp %x[n_channels], #0x4\n"
    ".inst 0x4fade93b  // sdot v27.4s, v9.16b, v13.4b[3]\n"
    ".inst 0x4face13a  // sdot v26.4s, v9.16b, v12.4b[1]\n"
    ".inst 0x4face939  // sdot v25.4s, v9.16b, v12.4b[3]\n"
    ".inst 0x4fa7e138  // sdot v24.4s, v9.16b, v7.4b[1]\n"
    ".inst 0x4fa7e937  // sdot v23.4s, v9.16b, v7.4b[3]\n"
    ".inst 0x4fa6e136  // sdot v22.4s, v9.16b, v6.4b[1]\n"
    ".inst 0x4fa6e935  // sdot v21.4s, v9.16b, v6.4b[3]\n"
    "ldr q9, [%x[params], #0x10]\n"
    ".inst 0x4f8ce15c  // sdot v28.4s, v10.16b, v12.4b[0]\n"
    ".inst 0x4f8ce95b  // sdot v27.4s, v10.16b, v12.4b[2]\n"
    ".inst 0x4f87e15a  // sdot v26.4s, v10.16b, v7.4b[0]\n"
    ".inst 0x4f87e959  // sdot v25.4s, v10.16b, v7.4b[2]\n"
    ".inst 0x4f86e158  // sdot v24.4s, v10.16b, v6.4b[0]\n"
    ".inst 0x4f86e957  // sdot v23.4s, v10.16b, v6.4b[2]\n"
    ".inst 0x4f85e156  // sdot v22.4s, v10.16b, v5.4b[0]\n"
    ".inst 0x4f85e955  // sdot v21.4s, v10.16b, v5.4b[2]\n"
    "ldr q10, [%x[params], #0x20]\n"
    ".inst 0x4face17c  // sdot v28.4s, v11.16b, v12.4b[1]\n"
    ".inst 0x4face97b  // sdot v27.4s, v11.16b, v12.4b[3]\n"
    ".inst 0x4fa7e17a  // sdot v26.4s, v11.16b, v7.4b[1]\n"
    ".inst 0x4fa7e979  // sdot v25.4s, v11.16b, v7.4b[3]\n"
    ".inst 0x4fa6e178  // sdot v24.4s, v11.16b, v6.4b[1]\n"
    ".inst 0x4fa6e977  // sdot v23.4s, v11.16b, v6.4b[3]\n"
    ".inst 0x4fa5e176  // sdot v22.4s, v11.16b, v5.4b[1]\n"
    ".inst 0x4fa5e975  // sdot v21.4s, v11.16b, v5.4b[3]\n"
    "ldr q11, [%x[params], #0x30]\n"
    ".inst 0x4f87e11c  // sdot v28.4s, v8.16b, v7.4b[0]\n"
    ".inst 0x4f87e91b  // sdot v27.4s, v8.16b, v7.4b[2]\n"
    ".inst 0x4f86e11a  // sdot v26.4s, v8.16b, v6.4b[0]\n"
    ".inst 0x4f86e919  // sdot v25.4s, v8.16b, v6.4b[2]\n"
    ".inst 0x4f85e118  // sdot v24.4s, v8.16b, v5.4b[0]\n"
    ".inst 0x4f85e917  // sdot v23.4s, v8.16b, v5.4b[2]\n"
    ".inst 0x4f84e116  // sdot v22.4s, v8.16b, v4.4b[0]\n"
    ".inst 0x4f84e915  // sdot v21.4s, v8.16b, v4.4b[2]\n"
    "ldr q8, [%x[params], #0x40]\n"
    ".inst 0x4fa7e13c  // sdot v28.4s, v9.16b, v7.4b[1]\n"
    ".inst 0x4fa7e93b  // sdot v27.4s, v9.16b, v7.4b[3]\n"
    ".inst 0x4fa6e13a  // sdot v26.4s, v9.16b, v6.4b[1]\n"
    ".inst 0x4fa6e939  // sdot v25.4s, v9.16b, v6.4b[3]\n"
    ".inst 0x4fa5e138  // sdot v24.4s, v9.16b, v5.4b[1]\n"
    ".inst 0x4fa5e937  // sdot v23.4s, v9.16b, v5.4b[3]\n"
    ".inst 0x4fa4e136  // sdot v22.4s, v9.16b, v4.4b[1]\n"
    ".inst 0x4fa4e935  // sdot v21.4s, v9.16b, v4.4b[3]\n"
    "ldr q9, [%x[params], #0x50]\n"
    "add %x[params], %x[params], #0x80\n"
    ".inst 0x4f86e15c  // sdot v28.4s, v10.16b, v6.4b[0]\n"
    ".inst 0x4f86e95b  // sdot v27.4s, v10.16b, v6.4b[2]\n"
    ".inst 0x4f85e15a  // sdot v26.4s, v10.16b, v5.4b[0]\n"
    ".inst 0x4f85e959  // sdot v25.4s, v10.16b, v5.4b[2]\n"
    ".inst 0x4f84e158  // sdot v24.4s, v10.16b, v4.4b[0]\n"
    ".inst 0x4f84e957  // sdot v23.4s, v10.16b, v4.4b[2]\n"
    ".inst 0x4f82e156  // sdot v22.4s, v10.16b, v2.4b[0]\n"
    ".inst 0x4f82e955  // sdot v21.4s, v10.16b, v2.4b[2]\n"
    ".inst 0x4fa6e17c  // sdot v28.4s, v11.16b, v6.4b[1]\n"
    ".inst 0x4fa6e97b  // sdot v27.4s, v11.16b, v6.4b[3]\n"
    ".inst 0x4fa5e17a  // sdot v26.4s, v11.16b, v5.4b[1]\n"
    ".inst 0x4fa5e979  // sdot v25.4s, v11.16b, v5.4b[3]\n"
    ".inst 0x4fa4e178  // sdot v24.4s, v11.16b, v4.4b[1]\n"
    ".inst 0x4fa4e977  // sdot v23.4s, v11.16b, v4.4b[3]\n"
    ".inst 0x4fa2e176  // sdot v22.4s, v11.16b, v2.4b[1]\n"
    ".inst 0x4fa2e975  // sdot v21.4s, v11.16b, v2.4b[3]\n"
    ".inst 0x4f85e11c  // sdot v28.4s, v8.16b, v5.4b[0]\n"
    ".inst 0x4f85e91b  // sdot v27.4s, v8.16b, v5.4b[2]\n"
    ".inst 0x4f84e11a  // sdot v26.4s, v8.16b, v4.4b[0]\n"
    ".inst 0x4f84e919  // sdot v25.4s, v8.16b, v4.4b[2]\n"
    ".inst 0x4f82e118  // sdot v24.4s, v8.16b, v2.4b[0]\n"
    ".inst 0x4f82e917  // sdot v23.4s, v8.16b, v2.4b[2]\n"
    ".inst 0x4f81e116  // sdot v22.4s, v8.16b, v1.4b[0]\n"
    ".inst 0x4f81e915  // sdot v21.4s, v8.16b, v1.4b[2]\n"
    ".inst 0x4fa5e13c  // sdot v28.4s, v9.16b, v5.4b[1]\n"
    ".inst 0x4fa5e93b  // sdot v27.4s, v9.16b, v5.4b[3]\n"
    ".inst 0x4fa4e13a  // sdot v26.4s, v9.16b, v4.4b[1]\n"
    ".inst 0x4fa4e939  // sdot v25.4s, v9.16b, v4.4b[3]\n"
    ".inst 0x4fa2e138  // sdot v24.4s, v9.16b, v2.4b[1]\n"
    ".inst 0x4fa2e937  // sdot v23.4s, v9.16b, v2.4b[3]\n"
    ".inst 0x4fa1e136  // sdot v22.4s, v9.16b, v1.4b[1]\n"
    ".inst 0x4fa1e935  // sdot v21.4s, v9.16b, v1.4b[3]\n"
    "sqrdmulh v28.4s, v28.4s, v20.4s\n"
    "sqrdmulh v27.4s, v27.4s, v20.4s\n"
    "sqrdmulh v26.4s, v26.4s, v20.4s\n"
    "sqrdmulh v25.4s, v25.4s, v20.4s\n"
    "and v18.16b, v28.16b, v19.16b\n"
    "and v17.16b, v27.16b, v19.16b\n"
    "and v16.16b, v26.16b, v19.16b\n"
    "sshr v18.4s, v18.4s, #0x1f\n"
    "sshr v17.4s, v17.4s, #0x1f\n"
    "sshr v16.4s, v16.4s, #0x1f\n"
    "sqadd v28.4s, v28.4s, v18.4s\n"
    "sqadd v27.4s, v27.4s, v17.4s\n"
    "sqadd v26.4s, v26.4s, v16.4s\n"
    "and v16.16b, v25.16b, v19.16b\n"
    "srshl v28.4s, v28.4s, v19.4s\n"
    "srshl v27.4s, v27.4s, v19.4s\n"
    "srshl v26.4s, v26.4s, v19.4s\n"
    "sshr v16.4s, v16.4s, #0x1f\n"
    "add v28.4s, v28.4s, v0.4s\n"
    "add v27.4s, v27.4s, v0.4s\n"
    "add v26.4s, v26.4s, v0.4s\n"
    "smin v28.4s, v28.4s, v30.4s\n"
    "smin v27.4s, v27.4s, v30.4s\n"
    "smin v26.4s, v26.4s, v30.4s\n"
    "smax v28.4s, v28.4s, v31.4s\n"
    "smax v27.4s, v27.4s, v31.4s\n"
    "smax v26.4s, v26.4s, v31.4s\n"
    "uzp1 v28.16b, v28.16b, v28.16b\n"
    "uzp1 v27.16b, v27.16b, v27.16b\n"
    "uzp1 v28.16b, v28.16b, v28.16b\n"
    "uzp1 v27.16b, v27.16b, v27.16b\n"
    "uzp1 v26.16b, v26.16b, v26.16b\n"
    "sqadd v25.4s, v25.4s, v16.4s\n"
    "uzp1 v26.16b, v26.16b, v26.16b\n"
    "sqrdmulh v24.4s, v24.4s, v20.4s\n"
    "sqrdmulh v23.4s, v23.4s, v20.4s\n"
    "srshl v25.4s, v25.4s, v19.4s\n"
    "sqrdmulh v22.4s, v22.4s, v20.4s\n"
    "and v16.16b, v24.16b, v19.16b\n"
    "and v17.16b, v23.16b, v19.16b\n"
    "add v25.4s, v25.4s, v0.4s\n"
    "sshr v16.4s, v16.4s, #0x1f\n"
    "sshr v17.4s, v17.4s, #0x1f\n"
    "smin v25.4s, v25.4s, v30.4s\n"
    "sqadd v24.4s, v24.4s, v16.4s\n"
    "sqadd v23.4s, v23.4s, v17.4s\n"
    "smax v25.4s, v25.4s, v31.4s\n"
    "and v16.16b, v22.16b, v19.16b\n"
    "srshl v24.4s, v24.4s, v19.4s\n"
    "uzp1 v25.16b, v25.16b, v25.16b\n"
    "srshl v23.4s, v23.4s, v19.4s\n"
    "uzp1 v25.16b, v25.16b, v25.16b\n"
    "add v24.4s, v24.4s, v0.4s\n"
    "sshr v16.4s, v16.4s, #0x1f\n"
    "add v23.4s, v23.4s, v0.4s\n"
    "smin v24.4s, v24.4s, v30.4s\n"
    "sqadd v22.4s, v22.4s, v16.4s\n"
    "smin v23.4s, v23.4s, v30.4s\n"
    "smax v24.4s, v24.4s, v31.4s\n"
    "sqrdmulh v21.4s, v21.4s, v20.4s\n"
    "smax v23.4s, v23.4s, v31.4s\n"
    "uzp1 v24.16b, v24.16b, v24.16b\n"
    "srshl v22.4s, v22.4s, v19.4s\n"
    "uzp1 v24.16b, v24.16b, v24.16b\n"
    "uzp1 v23.16b, v23.16b, v23.16b\n"
    "and v16.16b, v21.16b, v19.16b\n"
    "uzp1 v23.16b, v23.16b, v23.16b\n"
    "add v22.4s, v22.4s, v0.4s\n"
    "sshr v16.4s, v16.4s, #0x1f\n"
    "smin v22.4s, v22.4s, v30.4s\n"
    "sqadd v21.4s, v21.4s, v16.4s\n"
    "smax v22.4s, v22.4s, v31.4s\n"
    "srshl v21.4s, v21.4s, v19.4s\n"
    "uzp1 v22.16b, v22.16b, v22.16b\n"
    "uzp1 v22.16b, v22.16b, v22.16b\n"
    "add v21.4s, v21.4s, v0.4s\n"
    "smin v21.4s, v21.4s, v30.4s\n"
    "smax v21.4s, v21.4s, v31.4s\n"
    "uzp1 v21.16b, v21.16b, v21.16b\n"
    "uzp1 v21.16b, v21.16b, v21.16b\n"
    "blt 3f\n"
    "str s28, [x26, #0x0]\n"
    "str s27, [x25, #0x0]\n"
    "str s26, [x24, #0x0]\n"
    "str s25, [x23, #0x0]\n"
    "str s24, [x22, #0x0]\n"
    "str s23, [x21, #0x0]\n"
    "str s22, [x20, #0x0]\n"
    "str s21, [x19, #0x0]\n"
    "b 4f\n"
    "3:"  // Tail: Oddments
    "st1 { v28.b }[0], [x26], #0x1\n"
    "subs %x[n_channels], %x[n_channels], #0x1\n"
    "st1 { v27.b }[0], [x25], #0x1\n"
    "st1 { v26.b }[0], [x24], #0x1\n"
    "st1 { v25.b }[0], [x23], #0x1\n"
    "st1 { v24.b }[0], [x22], #0x1\n"
    "st1 { v23.b }[0], [x21], #0x1\n"
    "st1 { v22.b }[0], [x20], #0x1\n"
    "st1 { v21.b }[0], [x19], #0x1\n"
    "beq 4f\n"
    "st1 { v28.b }[1], [x26], #0x1\n"
    "subs %x[n_channels], %x[n_channels], #0x1\n"
    "st1 { v27.b }[1], [x25], #0x1\n"
    "st1 { v26.b }[1], [x24], #0x1\n"
    "st1 { v25.b }[1], [x23], #0x1\n"
    "st1 { v24.b }[1], [x22], #0x1\n"
    "st1 { v23.b }[1], [x21], #0x1\n"
    "st1 { v22.b }[1], [x20], #0x1\n"
    "st1 { v21.b }[1], [x19], #0x1\n"
    "beq 4f\n"
    "st1 { v28.b }[2], [x26], #0x1\n"
    "subs %x[n_channels], %x[n_channels], #0x1\n"
    "st1 { v27.b }[2], [x25], #0x1\n"
    "st1 { v26.b }[2], [x24], #0x1\n"
    "st1 { v25.b }[2], [x23], #0x1\n"
    "st1 { v24.b }[2], [x22], #0x1\n"
    "st1 { v23.b }[2], [x21], #0x1\n"
    "st1 { v22.b }[2], [x20], #0x1\n"
    "st1 { v21.b }[2], [x19], #0x1\n"
    "beq 4f\n"
    "st1 { v28.b }[3], [x26], #0x1\n"
    "subs %x[n_channels], %x[n_channels], #0x1\n"
    "st1 { v27.b }[3], [x25], #0x1\n"
    "st1 { v26.b }[3], [x24], #0x1\n"
    "st1 { v25.b }[3], [x23], #0x1\n"
    "st1 { v24.b }[3], [x22], #0x1\n"
    "st1 { v23.b }[3], [x21], #0x1\n"
    "st1 { v22.b }[3], [x20], #0x1\n"
    "st1 { v21.b }[3], [x19], #0x1\n"
    "4:"  // Tail: End
    "add SP, SP, #0x80\n"
    : [n_channels] "+&r" (n_output_channels), [params] "+&r" (params)
    : [inptrs] "r" (inptrs), [offsetof_Requantize32_b_offset] "I" (offsetof(arm_gemm::Requantize32, b_offset)), [offsetof_Requantize32_c_offset] "I" (offsetof(arm_gemm::Requantize32, c_offset)), [offsetof_Requantize32_maxval] "I" (offsetof(arm_gemm::Requantize32, maxval)), [offsetof_Requantize32_minval] "I" (offsetof(arm_gemm::Requantize32, minval)), [outptrs] "r" (outptrs), [qp] "r" (&qp)
    : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31", "x9", "x10", "x11", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28"
  );
}

}  // namespace depthwise
}  // namespace arm_conv
