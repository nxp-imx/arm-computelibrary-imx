/*
 * Copyright (c) 2017-2020 Arm Limited.
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
#include "arm_compute/runtime/SingleThreadScheduler.h"

#include "arm_compute/core/CPP/ICPPKernel.h"
#include "arm_compute/core/Error.h"
#include "arm_compute/core/Utils.h"

namespace arm_compute
{
void SingleThreadScheduler::set_num_threads(unsigned int num_threads)
{
    ARM_COMPUTE_UNUSED(num_threads);
    ARM_COMPUTE_ERROR_ON(num_threads != 1);
}

void SingleThreadScheduler::schedule(ICPPKernel *kernel, const Hints &hints)
{
    const Window      &max_window     = kernel->window();
    const unsigned int num_iterations = max_window.num_iterations(hints.split_dimension());
    if(num_iterations < 1)
    {
        return;
    }

    ThreadInfo info;
    info.cpu_info = &_cpu_info;
    kernel->run(kernel->window(), info);
}

void SingleThreadScheduler::schedule_op(ICPPKernel *kernel, const Hints &hints, const InputTensorMap &inputs, const OutputTensorMap &outputs)
{
    ARM_COMPUTE_UNUSED(hints);
    ThreadInfo info;
    info.cpu_info = &_cpu_info;
    kernel->run_op(inputs, outputs, kernel->window(), info);
}

void SingleThreadScheduler::run_workloads(std::vector<Workload> &workloads)
{
    ThreadInfo info;
    info.cpu_info = &_cpu_info;
    for(auto &wl : workloads)
    {
        wl(info);
    }
}
unsigned int SingleThreadScheduler::num_threads() const
{
    return 1;
}
} // namespace arm_compute
