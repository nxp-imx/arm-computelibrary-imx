/*
 * Copyright (c) 2019-2020, 2023 Arm Limited.
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

#include "arm_gemm.hpp"
#include "quantized.hpp"
#include "utils.hpp"

#include <cassert>

namespace arm_gemm {

template<>
void row_sums_indirect(
    unsigned int num_strings, const unsigned int *string_lengths, IndirectInputArg<int8_t> A_arg,
    size_t M, int32_t *out_ptr, const Requantize32 *qp
)
{
    struct KernelArgs {
        unsigned int num_strings;
        const unsigned int *string_lengths;
        unsigned int input_initial_col;
    } ka;

    unsigned long flags=0;
    void *input_ptr;
    size_t input_offset;

    if (A_arg.is_indirect) {
        input_ptr=(void *)(A_arg.indirect.ptr);
        input_offset=A_arg.indirect.start_row;
        ka.input_initial_col=A_arg.indirect.start_col;
        flags |= 0x8;
    } else {
        assert(num_strings==1);
        input_ptr=(void *)(A_arg.direct.base);
        input_offset=A_arg.direct.stride;
    }

    ka.num_strings = num_strings;
    ka.string_lengths = string_lengths;

    __asm__ __volatile__(
      "add x20, %x[qp], %[b_offset]\n"
      "ld1r { v2.4s }, [x20]\n"
      "neg v2.4s, v2.4s\n"
      "1:"  // Row loop
      "cmp %x[M], #0x6\n"
      "bge 86f\n"
      "cmp %x[M], #0x4\n"
      "bgt 69f\n"
      "beq 52f\n"
      "cmp %x[M], #0x2\n"
      "bgt 35f\n"
      "beq 18f\n"
      "movi v1.8h, #0x0\n"
      "movi v0.4s, #0x0\n"
      "mov x10, #0x0\n"
      "ldr w21, [%x[args_ptr], %[offsetof_num_strings]]\n"
      "mov x9, #0x0\n"
      "2:"  // Height 1: String loop
      "ldr x20, [%x[args_ptr], %[offsetof_string_lengths]]\n"
      "ldr w28, [x20, x9, LSL #0x2]\n"
      "tbz %x[flags], #3, 3f\n"
      "ldr x20, [%x[input_ptr], x9, LSL #0x3]\n"
      "add x20, x20, %x[input_offset], LSL #3\n"
      "ldr x27, [x20, #0x0]\n"
      "cbnz x9, 4f\n"
      "ldr w20, [%x[args_ptr], %[offsetof_input_initial_col]]\n"
      "add x27, x27, x20\n"
      "b 4f\n"
      "3:"  // Height 1: setup direct input
      "mov x27, %x[input_ptr]\n"
      "4:"  // Height 1: input setup done
      "cmp x28, #0x10\n"
      "blt 8f\n"
      "cmp x28, #0x20\n"
      "blt 7f\n"
      "5:"  // Height 1: Multiply loop: Main loop head
      "ldr q31, [x27, #0x0]\n"
      "cmp x10, #0x7e\n"
      "add x27, x27, #0x10\n"
      "blt 6f\n"
      "sadalp v0.4s, v1.8h\n"
      "movi v1.8h, #0x0\n"
      "mov x10, #0x0\n"
      "6:"  // Height 1: Multiply loop: unique 1: no collapse
      "sub x28, x28, #0x10\n"
      "cmp x28, #0x20\n"
      "sadalp v1.8h, v31.16b\n"
      "add x10, x10, #0x1\n"
      "bge 5b\n"
      "7:"  // Height 1: Multiply loop: Single iteration only
      "ldr q31, [x27, #0x0]\n"
      "sub x28, x28, #0x10\n"
      "sadalp v1.8h, v31.16b\n"
      "add x27, x27, #0x10\n"
      "8:"  // Height 1: Multiply loop: Main loop skip
      "cbz x28, 17f\n"
      "tbz x28, #3, 12f\n"
      "ldr d31, [x27], #0x8\n"
      "tbz x28, #2, 10f\n"
      "ld1 { v31.s }[2], [x27], #0x4\n"
      "tbz x28, #1, 9f\n"
      "ld1 { v31.h }[6], [x27], #0x2\n"
      "tbz x28, #0, 16f\n"
      "ld1 { v31.b }[14], [x27]\n"
      "b 16f\n"
      "9:"  // Height 1: Multiply loop: Ragged operand read: partial_1_12
      "tbz x28, #0, 16f\n"
      "ld1 { v31.b }[12], [x27]\n"
      "b 16f\n"
      "10:"  // Height 1: Multiply loop: Ragged operand read: partial_2_8
      "tbz x28, #1, 11f\n"
      "ld1 { v31.h }[4], [x27], #0x2\n"
      "tbz x28, #0, 16f\n"
      "ld1 { v31.b }[10], [x27]\n"
      "b 16f\n"
      "11:"  // Height 1: Multiply loop: Ragged operand read: partial_1_8
      "tbz x28, #0, 16f\n"
      "ld1 { v31.b }[8], [x27]\n"
      "b 16f\n"
      "12:"  // Height 1: Multiply loop: Ragged operand read: partial_4_0
      "tbz x28, #2, 14f\n"
      "ldr s31, [x27], #0x4\n"
      "tbz x28, #1, 13f\n"
      "ld1 { v31.h }[2], [x27], #0x2\n"
      "tbz x28, #0, 16f\n"
      "ld1 { v31.b }[6], [x27]\n"
      "b 16f\n"
      "13:"  // Height 1: Multiply loop: Ragged operand read: partial_1_4
      "tbz x28, #0, 16f\n"
      "ld1 { v31.b }[4], [x27]\n"
      "b 16f\n"
      "14:"  // Height 1: Multiply loop: Ragged operand read: partial_2_0
      "tbz x28, #1, 15f\n"
      "ldr h31, [x27], #0x2\n"
      "tbz x28, #0, 16f\n"
      "ld1 { v31.b }[2], [x27]\n"
      "b 16f\n"
      "15:"  // Height 1: Multiply loop: Ragged operand read: partial_1_0
      "ldr b31, [x27, #0x0]\n"
      "16:"  // Height 1: Multiply loop: Ragged operand read: Done
      "sadalp v1.8h, v31.16b\n"
      "17:"  // Height 1: Multiply loop: No odd multiplies
      "add x9, x9, #0x1\n"
      "cmp x9, x21\n"
      "bne 2b\n"
      "sadalp v0.4s, v1.8h\n"
      "addp v0.4s, v0.4s, v0.4s\n"
      "addp v0.4s, v0.4s, v0.4s\n"
      "mul v0.4s, v0.4s, v2.4s\n"
      "str s0, [%x[out_ptr]], #0x4\n"
      "b 104f\n"
      "18:"  // Height 2
      "movi v1.8h, #0x0\n"
      "movi v0.4s, #0x0\n"
      "mov x10, #0x0\n"
      "ldr w21, [%x[args_ptr], %[offsetof_num_strings]]\n"
      "movi v30.8h, #0x0\n"
      "movi v29.4s, #0x0\n"
      "mov x9, #0x0\n"
      "19:"  // Height 2: String loop
      "ldr x20, [%x[args_ptr], %[offsetof_string_lengths]]\n"
      "ldr w28, [x20, x9, LSL #0x2]\n"
      "tbz %x[flags], #3, 20f\n"
      "ldr x20, [%x[input_ptr], x9, LSL #0x3]\n"
      "add x20, x20, %x[input_offset], LSL #3\n"
      "ldr x27, [x20, #0x0]\n"
      "ldr x26, [x20, #0x8]\n"
      "cbnz x9, 21f\n"
      "ldr w20, [%x[args_ptr], %[offsetof_input_initial_col]]\n"
      "add x27, x27, x20\n"
      "add x26, x26, x20\n"
      "b 21f\n"
      "20:"  // Height 2: setup direct input
      "mov x27, %x[input_ptr]\n"
      "add x26, x27, %x[input_offset]\n"
      "21:"  // Height 2: input setup done
      "cmp x28, #0x10\n"
      "blt 25f\n"
      "cmp x28, #0x20\n"
      "blt 24f\n"
      "22:"  // Height 2: Multiply loop: Main loop head
      "ldr q31, [x27, #0x0]\n"
      "ldr q28, [x26, #0x0]\n"
      "cmp x10, #0x7e\n"
      "add x27, x27, #0x10\n"
      "add x26, x26, #0x10\n"
      "blt 23f\n"
      "sadalp v0.4s, v1.8h\n"
      "movi v1.8h, #0x0\n"
      "mov x10, #0x0\n"
      "sadalp v29.4s, v30.8h\n"
      "movi v30.8h, #0x0\n"
      "23:"  // Height 2: Multiply loop: unique 2: no collapse
      "sub x28, x28, #0x10\n"
      "cmp x28, #0x20\n"
      "sadalp v1.8h, v31.16b\n"
      "sadalp v30.8h, v28.16b\n"
      "add x10, x10, #0x1\n"
      "bge 22b\n"
      "24:"  // Height 2: Multiply loop: Single iteration only
      "ldr q31, [x27, #0x0]\n"
      "ldr q28, [x26, #0x0]\n"
      "sub x28, x28, #0x10\n"
      "sadalp v1.8h, v31.16b\n"
      "sadalp v30.8h, v28.16b\n"
      "add x27, x27, #0x10\n"
      "add x26, x26, #0x10\n"
      "25:"  // Height 2: Multiply loop: Main loop skip
      "cbz x28, 34f\n"
      "tbz x28, #3, 29f\n"
      "ldr d31, [x27], #0x8\n"
      "ldr d28, [x26], #0x8\n"
      "tbz x28, #2, 27f\n"
      "ld1 { v31.s }[2], [x27], #0x4\n"
      "ld1 { v28.s }[2], [x26], #0x4\n"
      "tbz x28, #1, 26f\n"
      "ld1 { v31.h }[6], [x27], #0x2\n"
      "ld1 { v28.h }[6], [x26], #0x2\n"
      "tbz x28, #0, 33f\n"
      "ld1 { v31.b }[14], [x27]\n"
      "ld1 { v28.b }[14], [x26]\n"
      "b 33f\n"
      "26:"  // Height 2: Multiply loop: Ragged operand read: partial_1_12
      "tbz x28, #0, 33f\n"
      "ld1 { v31.b }[12], [x27]\n"
      "ld1 { v28.b }[12], [x26]\n"
      "b 33f\n"
      "27:"  // Height 2: Multiply loop: Ragged operand read: partial_2_8
      "tbz x28, #1, 28f\n"
      "ld1 { v31.h }[4], [x27], #0x2\n"
      "ld1 { v28.h }[4], [x26], #0x2\n"
      "tbz x28, #0, 33f\n"
      "ld1 { v31.b }[10], [x27]\n"
      "ld1 { v28.b }[10], [x26]\n"
      "b 33f\n"
      "28:"  // Height 2: Multiply loop: Ragged operand read: partial_1_8
      "tbz x28, #0, 33f\n"
      "ld1 { v31.b }[8], [x27]\n"
      "ld1 { v28.b }[8], [x26]\n"
      "b 33f\n"
      "29:"  // Height 2: Multiply loop: Ragged operand read: partial_4_0
      "tbz x28, #2, 31f\n"
      "ldr s31, [x27], #0x4\n"
      "ldr s28, [x26], #0x4\n"
      "tbz x28, #1, 30f\n"
      "ld1 { v31.h }[2], [x27], #0x2\n"
      "ld1 { v28.h }[2], [x26], #0x2\n"
      "tbz x28, #0, 33f\n"
      "ld1 { v31.b }[6], [x27]\n"
      "ld1 { v28.b }[6], [x26]\n"
      "b 33f\n"
      "30:"  // Height 2: Multiply loop: Ragged operand read: partial_1_4
      "tbz x28, #0, 33f\n"
      "ld1 { v31.b }[4], [x27]\n"
      "ld1 { v28.b }[4], [x26]\n"
      "b 33f\n"
      "31:"  // Height 2: Multiply loop: Ragged operand read: partial_2_0
      "tbz x28, #1, 32f\n"
      "ldr h31, [x27], #0x2\n"
      "ldr h28, [x26], #0x2\n"
      "tbz x28, #0, 33f\n"
      "ld1 { v31.b }[2], [x27]\n"
      "ld1 { v28.b }[2], [x26]\n"
      "b 33f\n"
      "32:"  // Height 2: Multiply loop: Ragged operand read: partial_1_0
      "ldr b31, [x27, #0x0]\n"
      "ldr b28, [x26, #0x0]\n"
      "33:"  // Height 2: Multiply loop: Ragged operand read: Done
      "sadalp v1.8h, v31.16b\n"
      "sadalp v30.8h, v28.16b\n"
      "34:"  // Height 2: Multiply loop: No odd multiplies
      "add x9, x9, #0x1\n"
      "cmp x9, x21\n"
      "bne 19b\n"
      "sadalp v0.4s, v1.8h\n"
      "sadalp v29.4s, v30.8h\n"
      "addp v0.4s, v0.4s, v29.4s\n"
      "addp v0.4s, v0.4s, v0.4s\n"
      "mul v0.4s, v0.4s, v2.4s\n"
      "str d0, [%x[out_ptr]], #0x8\n"
      "b 104f\n"
      "35:"  // Height 3
      "movi v1.8h, #0x0\n"
      "movi v0.4s, #0x0\n"
      "mov x10, #0x0\n"
      "ldr w21, [%x[args_ptr], %[offsetof_num_strings]]\n"
      "movi v30.8h, #0x0\n"
      "movi v29.4s, #0x0\n"
      "mov x9, #0x0\n"
      "movi v27.8h, #0x0\n"
      "movi v26.4s, #0x0\n"
      "36:"  // Height 3: String loop
      "ldr x20, [%x[args_ptr], %[offsetof_string_lengths]]\n"
      "ldr w28, [x20, x9, LSL #0x2]\n"
      "tbz %x[flags], #3, 37f\n"
      "ldr x20, [%x[input_ptr], x9, LSL #0x3]\n"
      "add x20, x20, %x[input_offset], LSL #3\n"
      "ldr x27, [x20, #0x0]\n"
      "ldr x26, [x20, #0x8]\n"
      "ldr x25, [x20, #0x10]\n"
      "cbnz x9, 38f\n"
      "ldr w20, [%x[args_ptr], %[offsetof_input_initial_col]]\n"
      "add x27, x27, x20\n"
      "add x26, x26, x20\n"
      "add x25, x25, x20\n"
      "b 38f\n"
      "37:"  // Height 3: setup direct input
      "mov x27, %x[input_ptr]\n"
      "add x26, x27, %x[input_offset]\n"
      "add x25, x26, %x[input_offset]\n"
      "38:"  // Height 3: input setup done
      "cmp x28, #0x10\n"
      "blt 42f\n"
      "cmp x28, #0x20\n"
      "blt 41f\n"
      "39:"  // Height 3: Multiply loop: Main loop head
      "ldr q31, [x27, #0x0]\n"
      "ldr q28, [x26, #0x0]\n"
      "cmp x10, #0x7e\n"
      "add x27, x27, #0x10\n"
      "ldr q25, [x25, #0x0]\n"
      "add x26, x26, #0x10\n"
      "add x25, x25, #0x10\n"
      "blt 40f\n"
      "sadalp v0.4s, v1.8h\n"
      "movi v1.8h, #0x0\n"
      "mov x10, #0x0\n"
      "sadalp v29.4s, v30.8h\n"
      "movi v30.8h, #0x0\n"
      "sadalp v26.4s, v27.8h\n"
      "movi v27.8h, #0x0\n"
      "40:"  // Height 3: Multiply loop: unique 3: no collapse
      "sub x28, x28, #0x10\n"
      "cmp x28, #0x20\n"
      "sadalp v1.8h, v31.16b\n"
      "sadalp v30.8h, v28.16b\n"
      "sadalp v27.8h, v25.16b\n"
      "add x10, x10, #0x1\n"
      "bge 39b\n"
      "41:"  // Height 3: Multiply loop: Single iteration only
      "ldr q31, [x27, #0x0]\n"
      "ldr q28, [x26, #0x0]\n"
      "sub x28, x28, #0x10\n"
      "sadalp v1.8h, v31.16b\n"
      "ldr q25, [x25, #0x0]\n"
      "sadalp v30.8h, v28.16b\n"
      "sadalp v27.8h, v25.16b\n"
      "add x27, x27, #0x10\n"
      "add x26, x26, #0x10\n"
      "add x25, x25, #0x10\n"
      "42:"  // Height 3: Multiply loop: Main loop skip
      "cbz x28, 51f\n"
      "tbz x28, #3, 46f\n"
      "ldr d31, [x27], #0x8\n"
      "ldr d28, [x26], #0x8\n"
      "ldr d25, [x25], #0x8\n"
      "tbz x28, #2, 44f\n"
      "ld1 { v31.s }[2], [x27], #0x4\n"
      "ld1 { v28.s }[2], [x26], #0x4\n"
      "ld1 { v25.s }[2], [x25], #0x4\n"
      "tbz x28, #1, 43f\n"
      "ld1 { v31.h }[6], [x27], #0x2\n"
      "ld1 { v28.h }[6], [x26], #0x2\n"
      "ld1 { v25.h }[6], [x25], #0x2\n"
      "tbz x28, #0, 50f\n"
      "ld1 { v31.b }[14], [x27]\n"
      "ld1 { v28.b }[14], [x26]\n"
      "ld1 { v25.b }[14], [x25]\n"
      "b 50f\n"
      "43:"  // Height 3: Multiply loop: Ragged operand read: partial_1_12
      "tbz x28, #0, 50f\n"
      "ld1 { v31.b }[12], [x27]\n"
      "ld1 { v28.b }[12], [x26]\n"
      "ld1 { v25.b }[12], [x25]\n"
      "b 50f\n"
      "44:"  // Height 3: Multiply loop: Ragged operand read: partial_2_8
      "tbz x28, #1, 45f\n"
      "ld1 { v31.h }[4], [x27], #0x2\n"
      "ld1 { v28.h }[4], [x26], #0x2\n"
      "ld1 { v25.h }[4], [x25], #0x2\n"
      "tbz x28, #0, 50f\n"
      "ld1 { v31.b }[10], [x27]\n"
      "ld1 { v28.b }[10], [x26]\n"
      "ld1 { v25.b }[10], [x25]\n"
      "b 50f\n"
      "45:"  // Height 3: Multiply loop: Ragged operand read: partial_1_8
      "tbz x28, #0, 50f\n"
      "ld1 { v31.b }[8], [x27]\n"
      "ld1 { v28.b }[8], [x26]\n"
      "ld1 { v25.b }[8], [x25]\n"
      "b 50f\n"
      "46:"  // Height 3: Multiply loop: Ragged operand read: partial_4_0
      "tbz x28, #2, 48f\n"
      "ldr s31, [x27], #0x4\n"
      "ldr s28, [x26], #0x4\n"
      "ldr s25, [x25], #0x4\n"
      "tbz x28, #1, 47f\n"
      "ld1 { v31.h }[2], [x27], #0x2\n"
      "ld1 { v28.h }[2], [x26], #0x2\n"
      "ld1 { v25.h }[2], [x25], #0x2\n"
      "tbz x28, #0, 50f\n"
      "ld1 { v31.b }[6], [x27]\n"
      "ld1 { v28.b }[6], [x26]\n"
      "ld1 { v25.b }[6], [x25]\n"
      "b 50f\n"
      "47:"  // Height 3: Multiply loop: Ragged operand read: partial_1_4
      "tbz x28, #0, 50f\n"
      "ld1 { v31.b }[4], [x27]\n"
      "ld1 { v28.b }[4], [x26]\n"
      "ld1 { v25.b }[4], [x25]\n"
      "b 50f\n"
      "48:"  // Height 3: Multiply loop: Ragged operand read: partial_2_0
      "tbz x28, #1, 49f\n"
      "ldr h31, [x27], #0x2\n"
      "ldr h28, [x26], #0x2\n"
      "ldr h25, [x25], #0x2\n"
      "tbz x28, #0, 50f\n"
      "ld1 { v31.b }[2], [x27]\n"
      "ld1 { v28.b }[2], [x26]\n"
      "ld1 { v25.b }[2], [x25]\n"
      "b 50f\n"
      "49:"  // Height 3: Multiply loop: Ragged operand read: partial_1_0
      "ldr b31, [x27, #0x0]\n"
      "ldr b28, [x26, #0x0]\n"
      "ldr b25, [x25, #0x0]\n"
      "50:"  // Height 3: Multiply loop: Ragged operand read: Done
      "sadalp v1.8h, v31.16b\n"
      "sadalp v30.8h, v28.16b\n"
      "sadalp v27.8h, v25.16b\n"
      "51:"  // Height 3: Multiply loop: No odd multiplies
      "add x9, x9, #0x1\n"
      "cmp x9, x21\n"
      "bne 36b\n"
      "sadalp v0.4s, v1.8h\n"
      "sadalp v29.4s, v30.8h\n"
      "sadalp v26.4s, v27.8h\n"
      "addp v0.4s, v0.4s, v29.4s\n"
      "addp v26.4s, v26.4s, v26.4s\n"
      "addp v0.4s, v0.4s, v0.4s\n"
      "addp v26.4s, v26.4s, v26.4s\n"
      "mul v0.4s, v0.4s, v2.4s\n"
      "str d0, [%x[out_ptr]], #0x8\n"
      "mul v26.4s, v26.4s, v2.4s\n"
      "str s26, [%x[out_ptr]], #0x4\n"
      "b 104f\n"
      "52:"  // Height 4
      "movi v1.8h, #0x0\n"
      "movi v0.4s, #0x0\n"
      "mov x10, #0x0\n"
      "ldr w21, [%x[args_ptr], %[offsetof_num_strings]]\n"
      "movi v30.8h, #0x0\n"
      "movi v29.4s, #0x0\n"
      "mov x9, #0x0\n"
      "movi v27.8h, #0x0\n"
      "movi v26.4s, #0x0\n"
      "movi v24.8h, #0x0\n"
      "movi v23.4s, #0x0\n"
      "53:"  // Height 4: String loop
      "ldr x20, [%x[args_ptr], %[offsetof_string_lengths]]\n"
      "ldr w28, [x20, x9, LSL #0x2]\n"
      "tbz %x[flags], #3, 54f\n"
      "ldr x20, [%x[input_ptr], x9, LSL #0x3]\n"
      "add x20, x20, %x[input_offset], LSL #3\n"
      "ldr x27, [x20, #0x0]\n"
      "ldr x26, [x20, #0x8]\n"
      "ldr x25, [x20, #0x10]\n"
      "ldr x24, [x20, #0x18]\n"
      "cbnz x9, 55f\n"
      "ldr w20, [%x[args_ptr], %[offsetof_input_initial_col]]\n"
      "add x27, x27, x20\n"
      "add x26, x26, x20\n"
      "add x25, x25, x20\n"
      "add x24, x24, x20\n"
      "b 55f\n"
      "54:"  // Height 4: setup direct input
      "mov x27, %x[input_ptr]\n"
      "add x26, x27, %x[input_offset]\n"
      "add x25, x26, %x[input_offset]\n"
      "add x24, x25, %x[input_offset]\n"
      "55:"  // Height 4: input setup done
      "cmp x28, #0x10\n"
      "blt 59f\n"
      "cmp x28, #0x20\n"
      "blt 58f\n"
      "56:"  // Height 4: Multiply loop: Main loop head
      "ldr q31, [x27, #0x0]\n"
      "ldr q28, [x26, #0x0]\n"
      "cmp x10, #0x7e\n"
      "add x27, x27, #0x10\n"
      "ldr q25, [x25, #0x0]\n"
      "ldr q22, [x24, #0x0]\n"
      "add x26, x26, #0x10\n"
      "add x25, x25, #0x10\n"
      "add x24, x24, #0x10\n"
      "blt 57f\n"
      "sadalp v0.4s, v1.8h\n"
      "movi v1.8h, #0x0\n"
      "mov x10, #0x0\n"
      "sadalp v29.4s, v30.8h\n"
      "movi v30.8h, #0x0\n"
      "sadalp v26.4s, v27.8h\n"
      "movi v27.8h, #0x0\n"
      "sadalp v23.4s, v24.8h\n"
      "movi v24.8h, #0x0\n"
      "57:"  // Height 4: Multiply loop: unique 4: no collapse
      "sub x28, x28, #0x10\n"
      "cmp x28, #0x20\n"
      "sadalp v1.8h, v31.16b\n"
      "sadalp v30.8h, v28.16b\n"
      "sadalp v27.8h, v25.16b\n"
      "sadalp v24.8h, v22.16b\n"
      "add x10, x10, #0x1\n"
      "bge 56b\n"
      "58:"  // Height 4: Multiply loop: Single iteration only
      "ldr q31, [x27, #0x0]\n"
      "ldr q28, [x26, #0x0]\n"
      "sub x28, x28, #0x10\n"
      "sadalp v1.8h, v31.16b\n"
      "ldr q25, [x25, #0x0]\n"
      "ldr q22, [x24, #0x0]\n"
      "sadalp v30.8h, v28.16b\n"
      "sadalp v27.8h, v25.16b\n"
      "sadalp v24.8h, v22.16b\n"
      "add x27, x27, #0x10\n"
      "add x26, x26, #0x10\n"
      "add x25, x25, #0x10\n"
      "add x24, x24, #0x10\n"
      "59:"  // Height 4: Multiply loop: Main loop skip
      "cbz x28, 68f\n"
      "tbz x28, #3, 63f\n"
      "ldr d31, [x27], #0x8\n"
      "ldr d28, [x26], #0x8\n"
      "ldr d25, [x25], #0x8\n"
      "ldr d22, [x24], #0x8\n"
      "tbz x28, #2, 61f\n"
      "ld1 { v31.s }[2], [x27], #0x4\n"
      "ld1 { v28.s }[2], [x26], #0x4\n"
      "ld1 { v25.s }[2], [x25], #0x4\n"
      "ld1 { v22.s }[2], [x24], #0x4\n"
      "tbz x28, #1, 60f\n"
      "ld1 { v31.h }[6], [x27], #0x2\n"
      "ld1 { v28.h }[6], [x26], #0x2\n"
      "ld1 { v25.h }[6], [x25], #0x2\n"
      "ld1 { v22.h }[6], [x24], #0x2\n"
      "tbz x28, #0, 67f\n"
      "ld1 { v31.b }[14], [x27]\n"
      "ld1 { v28.b }[14], [x26]\n"
      "ld1 { v25.b }[14], [x25]\n"
      "ld1 { v22.b }[14], [x24]\n"
      "b 67f\n"
      "60:"  // Height 4: Multiply loop: Ragged operand read: partial_1_12
      "tbz x28, #0, 67f\n"
      "ld1 { v31.b }[12], [x27]\n"
      "ld1 { v28.b }[12], [x26]\n"
      "ld1 { v25.b }[12], [x25]\n"
      "ld1 { v22.b }[12], [x24]\n"
      "b 67f\n"
      "61:"  // Height 4: Multiply loop: Ragged operand read: partial_2_8
      "tbz x28, #1, 62f\n"
      "ld1 { v31.h }[4], [x27], #0x2\n"
      "ld1 { v28.h }[4], [x26], #0x2\n"
      "ld1 { v25.h }[4], [x25], #0x2\n"
      "ld1 { v22.h }[4], [x24], #0x2\n"
      "tbz x28, #0, 67f\n"
      "ld1 { v31.b }[10], [x27]\n"
      "ld1 { v28.b }[10], [x26]\n"
      "ld1 { v25.b }[10], [x25]\n"
      "ld1 { v22.b }[10], [x24]\n"
      "b 67f\n"
      "62:"  // Height 4: Multiply loop: Ragged operand read: partial_1_8
      "tbz x28, #0, 67f\n"
      "ld1 { v31.b }[8], [x27]\n"
      "ld1 { v28.b }[8], [x26]\n"
      "ld1 { v25.b }[8], [x25]\n"
      "ld1 { v22.b }[8], [x24]\n"
      "b 67f\n"
      "63:"  // Height 4: Multiply loop: Ragged operand read: partial_4_0
      "tbz x28, #2, 65f\n"
      "ldr s31, [x27], #0x4\n"
      "ldr s28, [x26], #0x4\n"
      "ldr s25, [x25], #0x4\n"
      "ldr s22, [x24], #0x4\n"
      "tbz x28, #1, 64f\n"
      "ld1 { v31.h }[2], [x27], #0x2\n"
      "ld1 { v28.h }[2], [x26], #0x2\n"
      "ld1 { v25.h }[2], [x25], #0x2\n"
      "ld1 { v22.h }[2], [x24], #0x2\n"
      "tbz x28, #0, 67f\n"
      "ld1 { v31.b }[6], [x27]\n"
      "ld1 { v28.b }[6], [x26]\n"
      "ld1 { v25.b }[6], [x25]\n"
      "ld1 { v22.b }[6], [x24]\n"
      "b 67f\n"
      "64:"  // Height 4: Multiply loop: Ragged operand read: partial_1_4
      "tbz x28, #0, 67f\n"
      "ld1 { v31.b }[4], [x27]\n"
      "ld1 { v28.b }[4], [x26]\n"
      "ld1 { v25.b }[4], [x25]\n"
      "ld1 { v22.b }[4], [x24]\n"
      "b 67f\n"
      "65:"  // Height 4: Multiply loop: Ragged operand read: partial_2_0
      "tbz x28, #1, 66f\n"
      "ldr h31, [x27], #0x2\n"
      "ldr h28, [x26], #0x2\n"
      "ldr h25, [x25], #0x2\n"
      "ldr h22, [x24], #0x2\n"
      "tbz x28, #0, 67f\n"
      "ld1 { v31.b }[2], [x27]\n"
      "ld1 { v28.b }[2], [x26]\n"
      "ld1 { v25.b }[2], [x25]\n"
      "ld1 { v22.b }[2], [x24]\n"
      "b 67f\n"
      "66:"  // Height 4: Multiply loop: Ragged operand read: partial_1_0
      "ldr b31, [x27, #0x0]\n"
      "ldr b28, [x26, #0x0]\n"
      "ldr b25, [x25, #0x0]\n"
      "ldr b22, [x24, #0x0]\n"
      "67:"  // Height 4: Multiply loop: Ragged operand read: Done
      "sadalp v1.8h, v31.16b\n"
      "sadalp v30.8h, v28.16b\n"
      "sadalp v27.8h, v25.16b\n"
      "sadalp v24.8h, v22.16b\n"
      "68:"  // Height 4: Multiply loop: No odd multiplies
      "add x9, x9, #0x1\n"
      "cmp x9, x21\n"
      "bne 53b\n"
      "sadalp v0.4s, v1.8h\n"
      "sadalp v29.4s, v30.8h\n"
      "sadalp v26.4s, v27.8h\n"
      "sadalp v23.4s, v24.8h\n"
      "addp v0.4s, v0.4s, v29.4s\n"
      "addp v29.4s, v26.4s, v23.4s\n"
      "addp v0.4s, v0.4s, v29.4s\n"
      "mul v0.4s, v0.4s, v2.4s\n"
      "st1 { v0.4s }, [%x[out_ptr]], #0x10\n"
      "b 104f\n"
      "69:"  // Height 5
      "movi v1.8h, #0x0\n"
      "movi v0.4s, #0x0\n"
      "mov x10, #0x0\n"
      "ldr w21, [%x[args_ptr], %[offsetof_num_strings]]\n"
      "movi v30.8h, #0x0\n"
      "movi v29.4s, #0x0\n"
      "mov x9, #0x0\n"
      "movi v27.8h, #0x0\n"
      "movi v26.4s, #0x0\n"
      "movi v24.8h, #0x0\n"
      "movi v23.4s, #0x0\n"
      "movi v21.8h, #0x0\n"
      "movi v20.4s, #0x0\n"
      "70:"  // Height 5: String loop
      "ldr x20, [%x[args_ptr], %[offsetof_string_lengths]]\n"
      "ldr w28, [x20, x9, LSL #0x2]\n"
      "tbz %x[flags], #3, 71f\n"
      "ldr x20, [%x[input_ptr], x9, LSL #0x3]\n"
      "add x20, x20, %x[input_offset], LSL #3\n"
      "ldr x27, [x20, #0x0]\n"
      "ldr x26, [x20, #0x8]\n"
      "ldr x25, [x20, #0x10]\n"
      "ldr x24, [x20, #0x18]\n"
      "ldr x23, [x20, #0x20]\n"
      "cbnz x9, 72f\n"
      "ldr w20, [%x[args_ptr], %[offsetof_input_initial_col]]\n"
      "add x27, x27, x20\n"
      "add x26, x26, x20\n"
      "add x25, x25, x20\n"
      "add x24, x24, x20\n"
      "add x23, x23, x20\n"
      "b 72f\n"
      "71:"  // Height 5: setup direct input
      "mov x27, %x[input_ptr]\n"
      "add x26, x27, %x[input_offset]\n"
      "add x25, x26, %x[input_offset]\n"
      "add x24, x25, %x[input_offset]\n"
      "add x23, x24, %x[input_offset]\n"
      "72:"  // Height 5: input setup done
      "cmp x28, #0x10\n"
      "blt 76f\n"
      "cmp x28, #0x20\n"
      "blt 75f\n"
      "73:"  // Height 5: Multiply loop: Main loop head
      "ldr q31, [x27, #0x0]\n"
      "ldr q28, [x26, #0x0]\n"
      "cmp x10, #0x7e\n"
      "add x27, x27, #0x10\n"
      "ldr q25, [x25, #0x0]\n"
      "ldr q22, [x24, #0x0]\n"
      "add x26, x26, #0x10\n"
      "add x25, x25, #0x10\n"
      "ldr q19, [x23, #0x0]\n"
      "add x24, x24, #0x10\n"
      "add x23, x23, #0x10\n"
      "blt 74f\n"
      "sadalp v0.4s, v1.8h\n"
      "movi v1.8h, #0x0\n"
      "mov x10, #0x0\n"
      "sadalp v29.4s, v30.8h\n"
      "movi v30.8h, #0x0\n"
      "sadalp v26.4s, v27.8h\n"
      "movi v27.8h, #0x0\n"
      "sadalp v23.4s, v24.8h\n"
      "movi v24.8h, #0x0\n"
      "sadalp v20.4s, v21.8h\n"
      "movi v21.8h, #0x0\n"
      "74:"  // Height 5: Multiply loop: unique 5: no collapse
      "sub x28, x28, #0x10\n"
      "cmp x28, #0x20\n"
      "sadalp v1.8h, v31.16b\n"
      "sadalp v30.8h, v28.16b\n"
      "sadalp v27.8h, v25.16b\n"
      "sadalp v24.8h, v22.16b\n"
      "add x10, x10, #0x1\n"
      "sadalp v21.8h, v19.16b\n"
      "bge 73b\n"
      "75:"  // Height 5: Multiply loop: Single iteration only
      "ldr q31, [x27, #0x0]\n"
      "ldr q28, [x26, #0x0]\n"
      "sub x28, x28, #0x10\n"
      "sadalp v1.8h, v31.16b\n"
      "ldr q25, [x25, #0x0]\n"
      "ldr q22, [x24, #0x0]\n"
      "sadalp v30.8h, v28.16b\n"
      "sadalp v27.8h, v25.16b\n"
      "ldr q19, [x23, #0x0]\n"
      "sadalp v24.8h, v22.16b\n"
      "sadalp v21.8h, v19.16b\n"
      "add x27, x27, #0x10\n"
      "add x26, x26, #0x10\n"
      "add x25, x25, #0x10\n"
      "add x24, x24, #0x10\n"
      "add x23, x23, #0x10\n"
      "76:"  // Height 5: Multiply loop: Main loop skip
      "cbz x28, 85f\n"
      "tbz x28, #3, 80f\n"
      "ldr d31, [x27], #0x8\n"
      "ldr d28, [x26], #0x8\n"
      "ldr d25, [x25], #0x8\n"
      "ldr d22, [x24], #0x8\n"
      "ldr d19, [x23], #0x8\n"
      "tbz x28, #2, 78f\n"
      "ld1 { v31.s }[2], [x27], #0x4\n"
      "ld1 { v28.s }[2], [x26], #0x4\n"
      "ld1 { v25.s }[2], [x25], #0x4\n"
      "ld1 { v22.s }[2], [x24], #0x4\n"
      "ld1 { v19.s }[2], [x23], #0x4\n"
      "tbz x28, #1, 77f\n"
      "ld1 { v31.h }[6], [x27], #0x2\n"
      "ld1 { v28.h }[6], [x26], #0x2\n"
      "ld1 { v25.h }[6], [x25], #0x2\n"
      "ld1 { v22.h }[6], [x24], #0x2\n"
      "ld1 { v19.h }[6], [x23], #0x2\n"
      "tbz x28, #0, 84f\n"
      "ld1 { v31.b }[14], [x27]\n"
      "ld1 { v28.b }[14], [x26]\n"
      "ld1 { v25.b }[14], [x25]\n"
      "ld1 { v22.b }[14], [x24]\n"
      "ld1 { v19.b }[14], [x23]\n"
      "b 84f\n"
      "77:"  // Height 5: Multiply loop: Ragged operand read: partial_1_12
      "tbz x28, #0, 84f\n"
      "ld1 { v31.b }[12], [x27]\n"
      "ld1 { v28.b }[12], [x26]\n"
      "ld1 { v25.b }[12], [x25]\n"
      "ld1 { v22.b }[12], [x24]\n"
      "ld1 { v19.b }[12], [x23]\n"
      "b 84f\n"
      "78:"  // Height 5: Multiply loop: Ragged operand read: partial_2_8
      "tbz x28, #1, 79f\n"
      "ld1 { v31.h }[4], [x27], #0x2\n"
      "ld1 { v28.h }[4], [x26], #0x2\n"
      "ld1 { v25.h }[4], [x25], #0x2\n"
      "ld1 { v22.h }[4], [x24], #0x2\n"
      "ld1 { v19.h }[4], [x23], #0x2\n"
      "tbz x28, #0, 84f\n"
      "ld1 { v31.b }[10], [x27]\n"
      "ld1 { v28.b }[10], [x26]\n"
      "ld1 { v25.b }[10], [x25]\n"
      "ld1 { v22.b }[10], [x24]\n"
      "ld1 { v19.b }[10], [x23]\n"
      "b 84f\n"
      "79:"  // Height 5: Multiply loop: Ragged operand read: partial_1_8
      "tbz x28, #0, 84f\n"
      "ld1 { v31.b }[8], [x27]\n"
      "ld1 { v28.b }[8], [x26]\n"
      "ld1 { v25.b }[8], [x25]\n"
      "ld1 { v22.b }[8], [x24]\n"
      "ld1 { v19.b }[8], [x23]\n"
      "b 84f\n"
      "80:"  // Height 5: Multiply loop: Ragged operand read: partial_4_0
      "tbz x28, #2, 82f\n"
      "ldr s31, [x27], #0x4\n"
      "ldr s28, [x26], #0x4\n"
      "ldr s25, [x25], #0x4\n"
      "ldr s22, [x24], #0x4\n"
      "ldr s19, [x23], #0x4\n"
      "tbz x28, #1, 81f\n"
      "ld1 { v31.h }[2], [x27], #0x2\n"
      "ld1 { v28.h }[2], [x26], #0x2\n"
      "ld1 { v25.h }[2], [x25], #0x2\n"
      "ld1 { v22.h }[2], [x24], #0x2\n"
      "ld1 { v19.h }[2], [x23], #0x2\n"
      "tbz x28, #0, 84f\n"
      "ld1 { v31.b }[6], [x27]\n"
      "ld1 { v28.b }[6], [x26]\n"
      "ld1 { v25.b }[6], [x25]\n"
      "ld1 { v22.b }[6], [x24]\n"
      "ld1 { v19.b }[6], [x23]\n"
      "b 84f\n"
      "81:"  // Height 5: Multiply loop: Ragged operand read: partial_1_4
      "tbz x28, #0, 84f\n"
      "ld1 { v31.b }[4], [x27]\n"
      "ld1 { v28.b }[4], [x26]\n"
      "ld1 { v25.b }[4], [x25]\n"
      "ld1 { v22.b }[4], [x24]\n"
      "ld1 { v19.b }[4], [x23]\n"
      "b 84f\n"
      "82:"  // Height 5: Multiply loop: Ragged operand read: partial_2_0
      "tbz x28, #1, 83f\n"
      "ldr h31, [x27], #0x2\n"
      "ldr h28, [x26], #0x2\n"
      "ldr h25, [x25], #0x2\n"
      "ldr h22, [x24], #0x2\n"
      "ldr h19, [x23], #0x2\n"
      "tbz x28, #0, 84f\n"
      "ld1 { v31.b }[2], [x27]\n"
      "ld1 { v28.b }[2], [x26]\n"
      "ld1 { v25.b }[2], [x25]\n"
      "ld1 { v22.b }[2], [x24]\n"
      "ld1 { v19.b }[2], [x23]\n"
      "b 84f\n"
      "83:"  // Height 5: Multiply loop: Ragged operand read: partial_1_0
      "ldr b31, [x27, #0x0]\n"
      "ldr b28, [x26, #0x0]\n"
      "ldr b25, [x25, #0x0]\n"
      "ldr b22, [x24, #0x0]\n"
      "ldr b19, [x23, #0x0]\n"
      "84:"  // Height 5: Multiply loop: Ragged operand read: Done
      "sadalp v1.8h, v31.16b\n"
      "sadalp v30.8h, v28.16b\n"
      "sadalp v27.8h, v25.16b\n"
      "sadalp v24.8h, v22.16b\n"
      "sadalp v21.8h, v19.16b\n"
      "85:"  // Height 5: Multiply loop: No odd multiplies
      "add x9, x9, #0x1\n"
      "cmp x9, x21\n"
      "bne 70b\n"
      "sadalp v0.4s, v1.8h\n"
      "sadalp v29.4s, v30.8h\n"
      "sadalp v26.4s, v27.8h\n"
      "sadalp v23.4s, v24.8h\n"
      "sadalp v20.4s, v21.8h\n"
      "addp v0.4s, v0.4s, v29.4s\n"
      "addp v29.4s, v26.4s, v23.4s\n"
      "addp v20.4s, v20.4s, v20.4s\n"
      "addp v0.4s, v0.4s, v29.4s\n"
      "addp v20.4s, v20.4s, v20.4s\n"
      "mul v0.4s, v0.4s, v2.4s\n"
      "st1 { v0.4s }, [%x[out_ptr]], #0x10\n"
      "mul v20.4s, v20.4s, v2.4s\n"
      "str s20, [%x[out_ptr]], #0x4\n"
      "b 104f\n"
      "86:"  // Height 6
      "movi v1.8h, #0x0\n"
      "movi v0.4s, #0x0\n"
      "mov x10, #0x0\n"
      "ldr w22, [%x[args_ptr], %[offsetof_num_strings]]\n"
      "movi v30.8h, #0x0\n"
      "movi v29.4s, #0x0\n"
      "mov x9, #0x0\n"
      "movi v27.8h, #0x0\n"
      "movi v26.4s, #0x0\n"
      "movi v24.8h, #0x0\n"
      "movi v23.4s, #0x0\n"
      "movi v21.8h, #0x0\n"
      "movi v20.4s, #0x0\n"
      "movi v18.8h, #0x0\n"
      "movi v17.4s, #0x0\n"
      "87:"  // Height 6: String loop
      "ldr x20, [%x[args_ptr], %[offsetof_string_lengths]]\n"
      "ldr w28, [x20, x9, LSL #0x2]\n"
      "tbz %x[flags], #3, 88f\n"
      "ldr x20, [%x[input_ptr], x9, LSL #0x3]\n"
      "add x20, x20, %x[input_offset], LSL #3\n"
      "ldr x27, [x20, #0x0]\n"
      "ldr x26, [x20, #0x8]\n"
      "ldr x25, [x20, #0x10]\n"
      "ldr x24, [x20, #0x18]\n"
      "ldr x23, [x20, #0x20]\n"
      "ldr x21, [x20, #0x28]\n"
      "cbnz x9, 89f\n"
      "ldr w20, [%x[args_ptr], %[offsetof_input_initial_col]]\n"
      "add x27, x27, x20\n"
      "add x26, x26, x20\n"
      "add x25, x25, x20\n"
      "add x24, x24, x20\n"
      "add x23, x23, x20\n"
      "add x21, x21, x20\n"
      "b 89f\n"
      "88:"  // Height 6: setup direct input
      "mov x27, %x[input_ptr]\n"
      "add x26, x27, %x[input_offset]\n"
      "add x25, x26, %x[input_offset]\n"
      "add x24, x25, %x[input_offset]\n"
      "add x23, x24, %x[input_offset]\n"
      "add x21, x23, %x[input_offset]\n"
      "89:"  // Height 6: input setup done
      "cmp x28, #0x10\n"
      "blt 93f\n"
      "cmp x28, #0x20\n"
      "blt 92f\n"
      "90:"  // Height 6: Multiply loop: Main loop head
      "ldr q31, [x27, #0x0]\n"
      "ldr q28, [x26, #0x0]\n"
      "cmp x10, #0x7e\n"
      "add x27, x27, #0x10\n"
      "ldr q25, [x25, #0x0]\n"
      "ldr q22, [x24, #0x0]\n"
      "add x26, x26, #0x10\n"
      "add x25, x25, #0x10\n"
      "ldr q19, [x23, #0x0]\n"
      "ldr q16, [x21, #0x0]\n"
      "add x24, x24, #0x10\n"
      "add x23, x23, #0x10\n"
      "add x21, x21, #0x10\n"
      "blt 91f\n"
      "sadalp v0.4s, v1.8h\n"
      "movi v1.8h, #0x0\n"
      "mov x10, #0x0\n"
      "sadalp v29.4s, v30.8h\n"
      "movi v30.8h, #0x0\n"
      "sadalp v26.4s, v27.8h\n"
      "movi v27.8h, #0x0\n"
      "sadalp v23.4s, v24.8h\n"
      "movi v24.8h, #0x0\n"
      "sadalp v20.4s, v21.8h\n"
      "movi v21.8h, #0x0\n"
      "sadalp v17.4s, v18.8h\n"
      "movi v18.8h, #0x0\n"
      "91:"  // Height 6: Multiply loop: unique 6: no collapse
      "sub x28, x28, #0x10\n"
      "cmp x28, #0x20\n"
      "sadalp v1.8h, v31.16b\n"
      "sadalp v30.8h, v28.16b\n"
      "sadalp v27.8h, v25.16b\n"
      "sadalp v24.8h, v22.16b\n"
      "add x10, x10, #0x1\n"
      "sadalp v21.8h, v19.16b\n"
      "sadalp v18.8h, v16.16b\n"
      "bge 90b\n"
      "92:"  // Height 6: Multiply loop: Single iteration only
      "ldr q31, [x27, #0x0]\n"
      "ldr q28, [x26, #0x0]\n"
      "sub x28, x28, #0x10\n"
      "sadalp v1.8h, v31.16b\n"
      "ldr q25, [x25, #0x0]\n"
      "ldr q22, [x24, #0x0]\n"
      "sadalp v30.8h, v28.16b\n"
      "sadalp v27.8h, v25.16b\n"
      "ldr q19, [x23, #0x0]\n"
      "ldr q16, [x21, #0x0]\n"
      "sadalp v24.8h, v22.16b\n"
      "sadalp v21.8h, v19.16b\n"
      "sadalp v18.8h, v16.16b\n"
      "add x27, x27, #0x10\n"
      "add x26, x26, #0x10\n"
      "add x25, x25, #0x10\n"
      "add x24, x24, #0x10\n"
      "add x23, x23, #0x10\n"
      "add x21, x21, #0x10\n"
      "93:"  // Height 6: Multiply loop: Main loop skip
      "cbz x28, 102f\n"
      "tbz x28, #3, 97f\n"
      "ldr d31, [x27], #0x8\n"
      "ldr d28, [x26], #0x8\n"
      "ldr d25, [x25], #0x8\n"
      "ldr d22, [x24], #0x8\n"
      "ldr d19, [x23], #0x8\n"
      "ldr d16, [x21], #0x8\n"
      "tbz x28, #2, 95f\n"
      "ld1 { v31.s }[2], [x27], #0x4\n"
      "ld1 { v28.s }[2], [x26], #0x4\n"
      "ld1 { v25.s }[2], [x25], #0x4\n"
      "ld1 { v22.s }[2], [x24], #0x4\n"
      "ld1 { v19.s }[2], [x23], #0x4\n"
      "ld1 { v16.s }[2], [x21], #0x4\n"
      "tbz x28, #1, 94f\n"
      "ld1 { v31.h }[6], [x27], #0x2\n"
      "ld1 { v28.h }[6], [x26], #0x2\n"
      "ld1 { v25.h }[6], [x25], #0x2\n"
      "ld1 { v22.h }[6], [x24], #0x2\n"
      "ld1 { v19.h }[6], [x23], #0x2\n"
      "ld1 { v16.h }[6], [x21], #0x2\n"
      "tbz x28, #0, 101f\n"
      "ld1 { v31.b }[14], [x27]\n"
      "ld1 { v28.b }[14], [x26]\n"
      "ld1 { v25.b }[14], [x25]\n"
      "ld1 { v22.b }[14], [x24]\n"
      "ld1 { v19.b }[14], [x23]\n"
      "ld1 { v16.b }[14], [x21]\n"
      "b 101f\n"
      "94:"  // Height 6: Multiply loop: Ragged operand read: partial_1_12
      "tbz x28, #0, 101f\n"
      "ld1 { v31.b }[12], [x27]\n"
      "ld1 { v28.b }[12], [x26]\n"
      "ld1 { v25.b }[12], [x25]\n"
      "ld1 { v22.b }[12], [x24]\n"
      "ld1 { v19.b }[12], [x23]\n"
      "ld1 { v16.b }[12], [x21]\n"
      "b 101f\n"
      "95:"  // Height 6: Multiply loop: Ragged operand read: partial_2_8
      "tbz x28, #1, 96f\n"
      "ld1 { v31.h }[4], [x27], #0x2\n"
      "ld1 { v28.h }[4], [x26], #0x2\n"
      "ld1 { v25.h }[4], [x25], #0x2\n"
      "ld1 { v22.h }[4], [x24], #0x2\n"
      "ld1 { v19.h }[4], [x23], #0x2\n"
      "ld1 { v16.h }[4], [x21], #0x2\n"
      "tbz x28, #0, 101f\n"
      "ld1 { v31.b }[10], [x27]\n"
      "ld1 { v28.b }[10], [x26]\n"
      "ld1 { v25.b }[10], [x25]\n"
      "ld1 { v22.b }[10], [x24]\n"
      "ld1 { v19.b }[10], [x23]\n"
      "ld1 { v16.b }[10], [x21]\n"
      "b 101f\n"
      "96:"  // Height 6: Multiply loop: Ragged operand read: partial_1_8
      "tbz x28, #0, 101f\n"
      "ld1 { v31.b }[8], [x27]\n"
      "ld1 { v28.b }[8], [x26]\n"
      "ld1 { v25.b }[8], [x25]\n"
      "ld1 { v22.b }[8], [x24]\n"
      "ld1 { v19.b }[8], [x23]\n"
      "ld1 { v16.b }[8], [x21]\n"
      "b 101f\n"
      "97:"  // Height 6: Multiply loop: Ragged operand read: partial_4_0
      "tbz x28, #2, 99f\n"
      "ldr s31, [x27], #0x4\n"
      "ldr s28, [x26], #0x4\n"
      "ldr s25, [x25], #0x4\n"
      "ldr s22, [x24], #0x4\n"
      "ldr s19, [x23], #0x4\n"
      "ldr s16, [x21], #0x4\n"
      "tbz x28, #1, 98f\n"
      "ld1 { v31.h }[2], [x27], #0x2\n"
      "ld1 { v28.h }[2], [x26], #0x2\n"
      "ld1 { v25.h }[2], [x25], #0x2\n"
      "ld1 { v22.h }[2], [x24], #0x2\n"
      "ld1 { v19.h }[2], [x23], #0x2\n"
      "ld1 { v16.h }[2], [x21], #0x2\n"
      "tbz x28, #0, 101f\n"
      "ld1 { v31.b }[6], [x27]\n"
      "ld1 { v28.b }[6], [x26]\n"
      "ld1 { v25.b }[6], [x25]\n"
      "ld1 { v22.b }[6], [x24]\n"
      "ld1 { v19.b }[6], [x23]\n"
      "ld1 { v16.b }[6], [x21]\n"
      "b 101f\n"
      "98:"  // Height 6: Multiply loop: Ragged operand read: partial_1_4
      "tbz x28, #0, 101f\n"
      "ld1 { v31.b }[4], [x27]\n"
      "ld1 { v28.b }[4], [x26]\n"
      "ld1 { v25.b }[4], [x25]\n"
      "ld1 { v22.b }[4], [x24]\n"
      "ld1 { v19.b }[4], [x23]\n"
      "ld1 { v16.b }[4], [x21]\n"
      "b 101f\n"
      "99:"  // Height 6: Multiply loop: Ragged operand read: partial_2_0
      "tbz x28, #1, 100f\n"
      "ldr h31, [x27], #0x2\n"
      "ldr h28, [x26], #0x2\n"
      "ldr h25, [x25], #0x2\n"
      "ldr h22, [x24], #0x2\n"
      "ldr h19, [x23], #0x2\n"
      "ldr h16, [x21], #0x2\n"
      "tbz x28, #0, 101f\n"
      "ld1 { v31.b }[2], [x27]\n"
      "ld1 { v28.b }[2], [x26]\n"
      "ld1 { v25.b }[2], [x25]\n"
      "ld1 { v22.b }[2], [x24]\n"
      "ld1 { v19.b }[2], [x23]\n"
      "ld1 { v16.b }[2], [x21]\n"
      "b 101f\n"
      "100:"  // Height 6: Multiply loop: Ragged operand read: partial_1_0
      "ldr b31, [x27, #0x0]\n"
      "ldr b28, [x26, #0x0]\n"
      "ldr b25, [x25, #0x0]\n"
      "ldr b22, [x24, #0x0]\n"
      "ldr b19, [x23, #0x0]\n"
      "ldr b16, [x21, #0x0]\n"
      "101:"  // Height 6: Multiply loop: Ragged operand read: Done
      "sadalp v1.8h, v31.16b\n"
      "sadalp v30.8h, v28.16b\n"
      "sadalp v27.8h, v25.16b\n"
      "sadalp v24.8h, v22.16b\n"
      "sadalp v21.8h, v19.16b\n"
      "sadalp v18.8h, v16.16b\n"
      "102:"  // Height 6: Multiply loop: No odd multiplies
      "add x9, x9, #0x1\n"
      "cmp x9, x22\n"
      "bne 87b\n"
      "sadalp v0.4s, v1.8h\n"
      "sadalp v29.4s, v30.8h\n"
      "subs %x[M], %x[M], #0x6\n"
      "sadalp v26.4s, v27.8h\n"
      "sadalp v23.4s, v24.8h\n"
      "sadalp v20.4s, v21.8h\n"
      "sadalp v17.4s, v18.8h\n"
      "addp v0.4s, v0.4s, v29.4s\n"
      "addp v29.4s, v26.4s, v23.4s\n"
      "addp v20.4s, v20.4s, v17.4s\n"
      "addp v0.4s, v0.4s, v29.4s\n"
      "addp v20.4s, v20.4s, v20.4s\n"
      "mul v0.4s, v0.4s, v2.4s\n"
      "st1 { v0.4s }, [%x[out_ptr]], #0x10\n"
      "mul v20.4s, v20.4s, v2.4s\n"
      "str d20, [%x[out_ptr]], #0x8\n"
      "beq 104f\n"
      "tbz %x[flags], #3, 103f\n"
      "add %x[input_offset], %x[input_offset], #0x6\n"
      "b 1b\n"
      "103:"  // Update direct input
      "mov x20, #0x6\n"
      "madd %x[input_ptr], x20, %x[input_offset], %x[input_ptr]\n"
      "b 1b\n"
      "104:"  // Exit

      : [M] "+&r" (M), [input_offset] "+&r" (input_offset), [input_ptr] "+&r" (input_ptr), [out_ptr] "+&r" (out_ptr)
      : [args_ptr] "r" (&ka), [b_offset] "I" (offsetof(Requantize32, b_offset)), [flags] "r" (flags), [offsetof_input_initial_col] "I" (offsetof(KernelArgs, input_initial_col)), [offsetof_num_strings] "I" (offsetof(KernelArgs, num_strings)), [offsetof_string_lengths] "I" (offsetof(KernelArgs, string_lengths)), [qp] "r" (qp)
      : "cc", "memory", "v0", "v1", "v2", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31", "x9", "x10", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28"
    );
}

} // namespace arm_gemm

#endif // __aarch64__
