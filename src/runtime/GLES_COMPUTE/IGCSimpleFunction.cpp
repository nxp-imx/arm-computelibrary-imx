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
#include "arm_compute/runtime/GLES_COMPUTE/IGCSimpleFunction.h"

#include "arm_compute/core/Error.h"
#include "arm_compute/runtime/GLES_COMPUTE/GCScheduler.h"

using namespace arm_compute;

IGCSimpleFunction::IGCSimpleFunction() //NOLINT
    : _kernel(),
      _border_handler()
{
}

void IGCSimpleFunction::run()
{
    ARM_COMPUTE_ERROR_ON_MSG(!_kernel, "The child class didn't set the GLES kernel or function isn't configured");

    // FIXME(APPBROWSER-300): We may need to rename "enqueue" to "dispatch" and "sync" to "memory_barrier".
    GCScheduler::get().enqueue(_border_handler, false);
    GCScheduler::get().sync();
    GCScheduler::get().enqueue(*_kernel);
}
