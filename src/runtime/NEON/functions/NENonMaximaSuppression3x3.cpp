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
#include "arm_compute/runtime/NEON/functions/NENonMaximaSuppression3x3.h"

#include "arm_compute/core/Helpers.h"
#include "arm_compute/core/NEON/kernels/NENonMaximaSuppression3x3Kernel.h"

#include <utility>

using namespace arm_compute;

void NENonMaximaSuppression3x3::configure(ITensor *input, ITensor *output, BorderMode border_mode)
{
    auto k = arm_compute::cpp14::make_unique<NENonMaximaSuppression3x3Kernel>();
    k->configure(input, output, border_mode == BorderMode::UNDEFINED);
    _kernel = std::move(k);

    if(border_mode != BorderMode::UNDEFINED)
    {
        _border_handler.configure(input, BorderSize(1), BorderMode::CONSTANT, 0);
    }
    else
    {
        _border_handler.configure(input, BorderSize(1), BorderMode::UNDEFINED, 0);
    }
}
