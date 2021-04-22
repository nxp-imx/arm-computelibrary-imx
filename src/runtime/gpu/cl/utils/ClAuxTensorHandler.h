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
#ifndef ARM_COMPUTE_CL_UTILS_CL_AUX_TENSOR_HANDLER_H
#define ARM_COMPUTE_CL_UTILS_CL_AUX_TENSOR_HANDLER_H

#include "arm_compute/core/ITensorPack.h"
#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/runtime/CL/CLTensor.h"

#include "support/Cast.h"

namespace arm_compute
{
namespace opencl
{
/* Tensor handler to wrap and handle tensor allocations on workspace buffers */
class CLAuxTensorHandler
{
public:
    CLAuxTensorHandler(int slot_id, TensorInfo &info, ITensorPack &pack, bool pack_inject = false)
        : _tensor()
    {
        _tensor.allocator()->soft_init(info);

        ICLTensor *packed_tensor = utils::cast::polymorphic_downcast<ICLTensor *>(pack.get_tensor(slot_id));
        if((packed_tensor == nullptr) || (info.total_size() > packed_tensor->info()->total_size()))
        {
            _tensor.allocator()->allocate();
            if(pack_inject)
            {
                pack.add_tensor(slot_id, &_tensor);
            }
        }
        else
        {
            _tensor.allocator()->import_memory(packed_tensor->cl_buffer());
        }
    }

    CLAuxTensorHandler(TensorInfo &info, ICLTensor &tensor)
        : _tensor()
    {
        _tensor.allocator()->soft_init(info);
        if(info.total_size() <= tensor.info()->total_size())
        {
            _tensor.allocator()->import_memory(tensor.cl_buffer());
        }
    }

    ICLTensor *get()
    {
        return &_tensor;
    }

    ICLTensor *operator()()
    {
        return &_tensor;
    }

private:
    CLTensor _tensor{};
};
} // namespace opencl
} // namespace arm_compute
#endif /* ARM_COMPUTE_CL_UTILS_CL_AUX_TENSOR_HANDLER_H */