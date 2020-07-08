/*
 * Copyright (c) 2016-2020 Arm Limited.
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
#include "arm_compute/runtime/NEON/functions/NEMagnitude.h"

#include "arm_compute/core/NEON/kernels/NEMagnitudePhaseKernel.h"
#include "arm_compute/core/Types.h"
#include "support/MemorySupport.h"

#include <utility>

using namespace arm_compute;

void NEMagnitude::configure(const ITensor *input1, const ITensor *input2, ITensor *output, MagnitudeType mag_type)
{
    if(mag_type == MagnitudeType::L1NORM)
    {
        auto k = arm_compute::support::cpp14::make_unique<NEMagnitudePhaseKernel<MagnitudeType::L1NORM, PhaseType::SIGNED>>();
        k->configure(input1, input2, output, nullptr);
        _kernel = std::move(k);
    }
    else
    {
        auto k = arm_compute::support::cpp14::make_unique<NEMagnitudePhaseKernel<MagnitudeType::L2NORM, PhaseType::SIGNED>>();
        k->configure(input1, input2, output, nullptr);
        _kernel = std::move(k);
    }
}
