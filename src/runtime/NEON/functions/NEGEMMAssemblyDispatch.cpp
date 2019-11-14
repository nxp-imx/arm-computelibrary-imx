/*
 * Copyright (c) 2018-2020 ARM Limited.
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
#include "arm_compute/runtime/NEON/functions/NEGEMMAssemblyDispatch.h"

#include "arm_compute/core/CPP/Validate.h"
#include "arm_compute/runtime/NEON/NEScheduler.h"
#include "arm_compute/runtime/NEON/functions/NESimpleAssemblyFunction.h"

#include <arm_neon.h>

namespace arm_compute
{
namespace
{
arm_gemm::Activation map_to_arm_gemm_activation(const ActivationLayerInfo &act)
{
    arm_gemm::Activation gemm_act;

    // Early exit in case lower bound is other than 0, as it's not yet supported
    if(act.b() != 0.f)
    {
        return gemm_act;
    }

    switch(act.activation())
    {
        case ActivationLayerInfo::ActivationFunction::RELU:
            gemm_act.type = arm_gemm::Activation::Type::ReLU;
            break;
        case ActivationLayerInfo::ActivationFunction::BOUNDED_RELU:
            gemm_act.type   = arm_gemm::Activation::Type::BoundedReLU;
            gemm_act.param1 = act.a();
            gemm_act.param2 = 0.f;
            break;
        case ActivationLayerInfo::ActivationFunction::LU_BOUNDED_RELU:
            gemm_act.type   = arm_gemm::Activation::Type::BoundedReLU;
            gemm_act.param1 = act.a();
            gemm_act.param2 = act.b();
            break;
        default:
            gemm_act.type = arm_gemm::Activation::Type::None;
    }

    return gemm_act;
}

template <typename TypeInput, typename TypeOutput>
class FallbackTransform : public ITransformWeights
{
public:
    FallbackTransform() noexcept {};
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    FallbackTransform(const FallbackTransform &) = delete;
    /** Default move constructor */
    FallbackTransform(FallbackTransform &&) = default;
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    FallbackTransform &operator=(const FallbackTransform &) = delete;
    /** Default move assignment operator */
    FallbackTransform &operator=(FallbackTransform &&) = default;
    void               run() override
    {
        _output.allocator()->allocate();
        ARM_COMPUTE_ERROR_ON(_output.buffer() == nullptr);
        _gemm_kernel_asm->pretranspose_B_array(_output.buffer(), _in1_ptr, _ldb, _multi_stride_b);
        _reshape_run = true;
    }

    void release() override
    {
        _output.allocator()->free();
    }

    ITensor *get_weights() override
    {
        return &_output;
    }

    uint32_t uid() override
    {
        uint32_t id = (_B_pretranspose_size | 0x80000000);
        return id;
    }

    void configure(size_t B_pretranspose_size, unsigned int alignment)
    {
        _output.allocator()->init(TensorInfo(TensorShape{ (B_pretranspose_size + alignment /* FIXME: remove alignment after COMPMID-1088 */) }, 1, DataType::S8), alignment);
        _B_pretranspose_size = B_pretranspose_size;
    }

    void set_pretranspose(ITensor *tensor)
    {
        if(!_reshape_run)
        {
            _gemm_kernel_asm->set_pretransposed_B_data(tensor->buffer());
        }
    }

    void set_args(const int ldb, const TypeInput *in1_ptr, const int multi_stride_b, std::shared_ptr<arm_gemm::GemmCommon<TypeInput, TypeOutput>> gemm_kernel_asm)
    {
        _ldb             = ldb;
        _in1_ptr         = in1_ptr;
        _multi_stride_b  = multi_stride_b;
        _gemm_kernel_asm = gemm_kernel_asm;
    }

private:
    Tensor           _output{};
    int              _ldb{};
    const TypeInput *_in1_ptr{};
    int              _multi_stride_b{};
    size_t           _B_pretranspose_size{};
    std::shared_ptr<arm_gemm::GemmCommon<TypeInput, TypeOutput>> _gemm_kernel_asm{ nullptr };
};

/** Fallback in case ACL doesn't have a function */
template <typename TypeInput, typename TypeOutput, class OutputStage = arm_gemm::Nothing>
class Fallback : public NEGEMMAssemblyDispatch::IFallback
{
public:
    /** Destructor */
    ~Fallback()
    {
        // Release memory if we have allocated the memory ourselves
        if(_pretranspose && !(_weights_manager && _weights_manager->are_weights_managed(_b)))
        {
            delete _pretranspose;
        }
    }

    /** Initialise the functions's input and output.
     *
     * @param[in]  a               Input tensor containing the Matrix A.
     * @param[in]  b               Input tensor containing the Matrix B.
     * @param[in]  c               Input tensor containing the Matrix C.
     * @param[out] d               Output tensor to store the result of matrix multiplication.
     * @param[in]  args            Matrix multiplication information.
     * @param[in]  gemm_info       GEMM meta-data
     * @param[in]  memory_group    Memory group to be used by the function.
     * @param[in]  weights_manager Weights manager to be used by the function.
     * @param[in]  os              Output stage meta-data.
     */
    void configure(const ITensor *a, const ITensor *b, const ITensor *c, ITensor *d,
                   arm_gemm::GemmArgs args, const GEMMInfo &gemm_info,
                   MemoryGroup &memory_group, IWeightsManager *weights_manager, const OutputStage &os = {});

    /** Set requantization shifts to be used
     *
     * @param[in] shifts Requantization shifts
     *
     * @return Pointer to the shift data
     */
    /** Set requantization data to be used
      *
      *
      * @param shifts       Requantization shifts
      * @param multipliers  Requantization multipliers
      *
      * @return A tuple with the pointers to the shift and multiplier data respectively
      */
    std::tuple<const int32_t *, const int32_t *> set_requantize_data(const std::vector<int32_t> &shifts,
                                                                     const std::vector<int32_t> &multipliers);

    // Inherited methods overridden:
    void run() override;
    void prepare() override;
    bool is_configured() const override;

private:
    /** Allocate a workspace tensor.
     *
     * @param[in] workspace_size Size to allocate.
     * @param[in] memory_group   Tensor memory group.
     * @param[in] alignment      Workspace memory alignment.
     */
    void allocate_workspace(size_t workspace_size, MemoryGroup &memory_group, size_t alignment);

    /** Assembly Gemm kernel */
    std::shared_ptr<arm_gemm::GemmCommon<TypeInput, TypeOutput>> _gemm_kernel_asm{ nullptr };
    /** Optimised NEON kernel */
    std::unique_ptr<INEKernel> _optimised_kernel{ nullptr };
    /** Input A */
    const ITensor *_a
    {
        nullptr
    };
    /** Input B */
    const ITensor *_b
    {
        nullptr
    };
    const ITensor *_c
    {
        nullptr
    };
    /** Output */
    ITensor *_d{ nullptr };
    /** GEMM workspace */
    Tensor _workspace{};
    /** Pre-transpose tensor */
    ITensor *_pretranspose{ nullptr };
    /** Prepared flag */
    bool _is_prepared{ false };
    /** GEMM meta-data */
    GEMMInfo _gemm_info{};
    /** Weights manager */
    IWeightsManager *_weights_manager{ nullptr };
    /** Weights transform object */
    FallbackTransform<TypeInput, TypeOutput> _weights_transform{};
    /** GEMM kernel description */
    arm_gemm::KernelDescription _kernel_info{};
    /** Per channel quantization shifts */
    std::vector<int32_t> _shifts{};
    /** Per channel quantization multipliers */
    std::vector<int32_t> _multipliers{};
};

template <typename TypeInput, typename TypeOutput, class OutputStage>
std::tuple<const int32_t *, const int32_t *> Fallback<TypeInput, TypeOutput, OutputStage>::set_requantize_data(const std::vector<int32_t> &shifts,
                                                                                                               const std::vector<int32_t> &multipliers)
{
    _multipliers = multipliers;
    _shifts      = shifts;
    std::transform(_shifts.begin(), _shifts.end(), _shifts.begin(),
                   std::bind1st(std::multiplies<int32_t>(), -1));
    return std::make_tuple(_shifts.data(), _multipliers.data());
}

template <typename TypeInput, typename TypeOutput, class OutputStage>
void Fallback<TypeInput, TypeOutput, OutputStage>::configure(const ITensor *a, const ITensor *b, const ITensor *c, ITensor *d,
                                                             arm_gemm::GemmArgs args, const GEMMInfo &gemm_info,
                                                             MemoryGroup &memory_group, IWeightsManager *weights_manager, const OutputStage &os)
{
    arm_gemm::GemmConfig gemm_cfg;
    _kernel_info     = arm_gemm::get_gemm_method<TypeInput, TypeOutput, OutputStage>(args, os);
    _weights_manager = weights_manager;
    if(_kernel_info.method != arm_gemm::GemmMethod::GEMV_BATCHED)
    {
        gemm_cfg.filter = _kernel_info.name;
        args._cfg       = &gemm_cfg;
    }
    _gemm_kernel_asm = arm_gemm::gemm<TypeInput, TypeOutput, OutputStage>(args, os);
    if(_gemm_kernel_asm == nullptr)
    {
        //configuration not supported: Leave function unconfigured:
        return;
    }

    // arm_compute wrapper for the Gemm object (see above)
    std::unique_ptr<NEGEMMAssemblyWrapperKernel<TypeInput, TypeOutput>> acl_gemm_wrapper = support::cpp14::make_unique<NEGEMMAssemblyWrapperKernel<TypeInput, TypeOutput>>();
    ARM_COMPUTE_ERROR_ON(acl_gemm_wrapper == nullptr);
    acl_gemm_wrapper->configure(_gemm_kernel_asm.get(), gemm_cfg.filter);
    const size_t workspace_size = _gemm_kernel_asm->get_working_size();
    if(workspace_size > 0)
    {
        // Allocate workspace
        const unsigned int alignment = 4096;
        allocate_workspace(workspace_size, memory_group, alignment);
    }

    //if we disable this code below in brackets then ConvLayer deadlocks when threads > 1 and
    //the shapes are In=1x1x1024 Weights=1x1x1024x1001 Biases=1001 Out=1x1x1001
    {
        const int window_size = _gemm_kernel_asm->get_window_size();
        if(window_size < args._maxthreads)
        {
            _gemm_kernel_asm->set_nthreads(window_size);
        }
    }

    _optimised_kernel = std::move(acl_gemm_wrapper);
    _a                = a;
    _b                = b;
    _c                = c;
    _d                = d;
    _gemm_info        = gemm_info;
    // Check for pre-transposed support
    if(_gemm_kernel_asm->B_pretranspose_required())
    {
        // Forcing 128-byte alignment (required by 32-bit kernels)
        const unsigned int alignment           = 128;
        const size_t       B_pretranspose_size = _gemm_kernel_asm->get_B_pretransposed_array_size();
        if(weights_manager && _weights_manager->are_weights_managed(b))
        {
            _weights_transform.configure(B_pretranspose_size, alignment);
            _pretranspose = _weights_manager->acquire(b, &_weights_transform);
        }
        else
        {
            _pretranspose = new Tensor();
            static_cast<Tensor *>(_pretranspose)->allocator()->init(TensorInfo(TensorShape{ (B_pretranspose_size + alignment /* FIXME: remove alignment after COMPMID-1088 */) }, 1, DataType::S8), alignment);
        }
    }
}

template <typename TypeInput, typename TypeOutput, class OutputStage>
void Fallback<TypeInput, TypeOutput, OutputStage>::prepare()
{
    if(!_is_prepared)
    {
        // Setup up matrix bias in the assembly kernel, it's just a pointer to matrix C.
        if(_c && _c->info()->data_type() == DataType::S32)
        {
            _gemm_kernel_asm->set_quantized_bias(reinterpret_cast<const int32_t *>(_c->buffer() + _c->info()->offset_first_element_in_bytes()), 0);
        }

        // Pretranspose B if required
        if(_gemm_kernel_asm->B_pretranspose_required())
        {
            const int  ldb            = _b->info()->strides_in_bytes().y() / sizeof(TypeInput);
            const auto in1_ptr        = reinterpret_cast<const TypeInput *>(_b->buffer() + _b->info()->offset_first_element_in_bytes());
            const int  multi_stride_b = _b->info()->strides_in_bytes().z() / sizeof(TypeInput);

            if(_weights_manager && _weights_manager->are_weights_managed(_b))
            {
                _weights_transform.set_args(ldb, in1_ptr, multi_stride_b, _gemm_kernel_asm);
                _weights_manager->run(_b, &_weights_transform);

                // If we didn't run the reshape function, set the pretransposed buffer
                if(!_weights_transform.is_reshape_run())
                {
                    _weights_transform.set_pretranspose(_pretranspose);
                }
            }
            else
            {
                static_cast<Tensor *>(_pretranspose)->allocator()->allocate();
                ARM_COMPUTE_ERROR_ON(_pretranspose->buffer() == nullptr);
                _gemm_kernel_asm->pretranspose_B_array(_pretranspose->buffer(), in1_ptr, ldb, multi_stride_b);
                _b->mark_as_unused();
            }
        }

        _is_prepared = true;
    }
}

template <typename TypeInput, typename TypeOutput, class OutputStage>
void Fallback<TypeInput, TypeOutput, OutputStage>::allocate_workspace(size_t workspace_size, MemoryGroup &memory_group, size_t alignment)
{
    ARM_COMPUTE_ERROR_ON_MSG(workspace_size == 0, "size cannot be 0");
    _workspace.allocator()->init(TensorInfo(TensorShape{ (workspace_size + alignment /* FIXME: remove alignment after COMPMID-1088 */) }, 1, DataType::S8), alignment);
    memory_group.manage(&_workspace);
    _workspace.allocator()->allocate();
}

template <typename TypeInput, typename TypeOutput, class OutputStage>
bool Fallback<TypeInput, TypeOutput, OutputStage>::is_configured() const
{
    return _optimised_kernel != nullptr;
}

template <typename TypeInput, typename TypeOutput, class OutputStage>
void Fallback<TypeInput, TypeOutput, OutputStage>::run()
{
    const int lda = _a->info()->strides_in_bytes().y() / sizeof(TypeInput);
    int       ldb = 0;
    const int ldd = _d->info()->strides_in_bytes().y() / sizeof(TypeOutput);

    const size_t a_batch_idx = _gemm_info.reinterpret_input_as_3d() != 0 ? 3 : 2;
    const size_t a_multi_idx = a_batch_idx + 1;
    const size_t d_batch_idx = _gemm_info.depth_output_gemm3d() != 0 ? 3 : 2;
    const size_t d_multi_idx = d_batch_idx + 1;

    const int batch_stride_a = _a->info()->strides_in_bytes()[a_batch_idx] / sizeof(TypeInput);
    const int batch_stride_d = _d->info()->strides_in_bytes()[d_batch_idx] / sizeof(TypeOutput);

    const int multi_stride_a = _a->info()->strides_in_bytes()[a_multi_idx] / sizeof(TypeInput);
    int       multi_stride_b = 0;
    const int multi_stride_d = _d->info()->strides_in_bytes()[d_multi_idx] / sizeof(TypeOutput);

    const auto       in0_ptr = reinterpret_cast<const TypeInput *>(_a->buffer() + _a->info()->offset_first_element_in_bytes());
    const TypeInput *in1_ptr = nullptr;
    auto             out_ptr = reinterpret_cast<TypeOutput *>(_d->buffer() + _d->info()->offset_first_element_in_bytes());

    // Check if B is pre-tranposed and de-reference if not
    if(!_gemm_kernel_asm->B_is_pretransposed())
    {
        ldb            = _b->info()->strides_in_bytes().y() / sizeof(TypeInput);
        multi_stride_b = _b->info()->strides_in_bytes().z() / sizeof(TypeInput);
        in1_ptr        = reinterpret_cast<const TypeInput *>(_b->buffer() + _b->info()->offset_first_element_in_bytes());
    }

    // Set workspace if needed and reset number of threads as buffer manager gets re-created with max_threads
    if(_workspace.buffer() != nullptr)
    {
        _gemm_kernel_asm->set_working_space(reinterpret_cast<void *>(_workspace.buffer()));
        const unsigned int window_size = _gemm_kernel_asm->get_window_size();
        unsigned int       num_threads = NEScheduler::get().num_threads();
        if(window_size < num_threads)
        {
            num_threads = window_size;
            _gemm_kernel_asm->set_nthreads(num_threads);
        }
    }

    // Prepare assembly kernel
    prepare();

    TypeOutput *bias = nullptr;
    // Setup up matrix bias in the assembly kernel, it's just a pointer to matrix C.
    if(_c && _c->info()->data_type() != DataType::S32)
    {
        bias = reinterpret_cast<TypeOutput *>(_c->buffer() + _c->info()->offset_first_element_in_bytes());
    }
    // Set gemm parameters
    _gemm_kernel_asm->set_arrays(in0_ptr, lda, batch_stride_a, multi_stride_a,
                                 in1_ptr, ldb, multi_stride_b,
                                 out_ptr, ldd, batch_stride_d, multi_stride_d,
                                 bias, 0);

    // Schedule assembly kernel
    IScheduler::Hints scheduling_hint = IScheduler::Hints(Window::DimX);
    if(_kernel_info.method == arm_gemm::GemmMethod::GEMM_INTERLEAVED && _d->info()->data_type() == DataType::F32)
    {
        const int granule_threshold = 200;
        scheduling_hint             = IScheduler::Hints(Window::DimX, IScheduler::StrategyHint::DYNAMIC, granule_threshold);
    }
    NEScheduler::get().schedule(_optimised_kernel.get(), scheduling_hint);
}

template <typename TypeInput, typename TypeOutput>
void create_arm_gemm(std::unique_ptr<NEGEMMAssemblyDispatch::IFallback> &arm_gemm, MemoryGroup &memory_group,
                     const ITensor *a, const ITensor *b, const ITensor *c, ITensor *d, arm_gemm::Activation activation, const GEMMInfo &gemm_info,
                     IWeightsManager *weights_manager)
{
    INEGEMMWrapperKernel::Params p           = INEGEMMWrapperKernel::extract_parameters(a, b, d, gemm_info);
    const CPUInfo               &ci          = NEScheduler::get().cpu_info();
    unsigned int                 num_threads = NEScheduler::get().num_threads();

    arm_gemm::GemmArgs args(&ci, p.M, p.N, p.K, p.batches, p.multis, false, false, activation, num_threads, gemm_info.pretranpose_B());

    // Create arm_gemm fallback
    auto fallback = support::cpp14::make_unique<Fallback<TypeInput, TypeOutput>>();
    fallback->configure(a, b, c, d, args, gemm_info, memory_group, weights_manager);
    arm_gemm = std::move(fallback);
}

template <typename TypeInput, typename TypeOutput>
void create_arm_gemm_quant(std::unique_ptr<NEGEMMAssemblyDispatch::IFallback> &arm_gemm, MemoryGroup &memory_group,
                           const ITensor *a, const ITensor *b, const ITensor *c, ITensor *d, arm_gemm::Activation activation, const GEMMInfo &gemm_info,
                           IWeightsManager *weights_manager)
{
    INEGEMMWrapperKernel::Params p           = INEGEMMWrapperKernel::extract_parameters(a, b, d, gemm_info);
    const CPUInfo               &ci          = NEScheduler::get().cpu_info();
    unsigned int                 num_threads = NEScheduler::get().num_threads();

    arm_gemm::GemmArgs args(&ci, p.M, p.N, p.K, p.batches, p.multis, false, false, activation, num_threads, gemm_info.pretranpose_B());

    // Create arm_gemm fallback
    auto fallback = support::cpp14::make_unique<Fallback<TypeInput, TypeOutput, arm_gemm::Requantize32>>();

    // Configure requantization info
    const int32_t                 a_offset = -a->info()->quantization_info().uniform().offset;
    const int32_t                 b_offset = -b->info()->quantization_info().uniform().offset;
    const GEMMLowpOutputStageInfo os_info  = gemm_info.gemmlowp_output_stage();

    arm_gemm::Requantize32 gemm_requant_info{};
    if(os_info.gemmlowp_shifts.size() > 1)
    {
        const auto requantize_data = fallback->set_requantize_data(os_info.gemmlowp_shifts, os_info.gemmlowp_multipliers);
        gemm_requant_info          = arm_gemm::Requantize32(nullptr, 0,
                                                            a_offset, b_offset, os_info.gemmlowp_offset,
                                                            std::get<0>(requantize_data), std::get<1>(requantize_data),
                                                            os_info.gemmlowp_min_bound, os_info.gemmlowp_max_bound);
    }
    else
    {
        gemm_requant_info = arm_gemm::Requantize32(nullptr, 0,
                                                   a_offset, b_offset, os_info.gemmlowp_offset,
                                                   -os_info.gemmlowp_shift, os_info.gemmlowp_multiplier,
                                                   os_info.gemmlowp_min_bound, os_info.gemmlowp_max_bound);
    }

    // Configure fallback
    fallback->configure(a, b, c, d, args, gemm_info, memory_group, weights_manager, gemm_requant_info);
    arm_gemm = std::move(fallback);
}

} //namespace

NEGEMMAssemblyDispatch::NEGEMMAssemblyDispatch(std::shared_ptr<IMemoryManager> memory_manager, IWeightsManager *weights_manager)
    : _arm_gemm(nullptr), _memory_group(std::move(memory_manager)), _weights_manager(weights_manager)
{
}

Status NEGEMMAssemblyDispatch::validate(const ITensorInfo *a, const ITensorInfo *b, const ITensorInfo *c, const ITensorInfo *d, const GEMMInfo &gemm_info)
{
    ARM_COMPUTE_UNUSED(gemm_info);
    ARM_COMPUTE_UNUSED(c);
    ARM_COMPUTE_RETURN_ERROR_ON_NULLPTR(a, b, d);
    ARM_COMPUTE_RETURN_ERROR_ON_CPU_F16_UNSUPPORTED(a);
#ifndef __aarch64__
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(a->element_size() == 1, "8bit integer types only supported for aarch64");
#endif /* __aarch64__ */
    ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(a, 1, DataType::U8, DataType::QASYMM8, DataType::QASYMM8_SIGNED, DataType::S8,
                                                         DataType::F16, DataType::F32);
    ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(b, 1, DataType::U8, DataType::QASYMM8, DataType::QASYMM8_SIGNED, DataType::QSYMM8_PER_CHANNEL, DataType::S8,
                                                         DataType::F16, DataType::F32);
    if(is_data_type_quantized_per_channel(b->data_type()))
    {
        ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(a, 1, DataType::QASYMM8_SIGNED, DataType::S8);
    }
    else
    {
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(a, b);
    }
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(a->data_type() == DataType::F32 && d->data_type() != DataType::F32, "Only F32 output supported for F32 input");
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(a->data_type() == DataType::F16 && d->data_type() != DataType::F16, "Only F16 output supported for F16 input");
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(a->data_type() == DataType::U8 && d->data_type() != DataType::U32, "Only U32 output supported for U8 input");
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(a->data_type() == DataType::S8 && d->data_type() != DataType::S32, "Only S32 output supported for S8 input");
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(a->data_type() == DataType::QASYMM8 && d->data_type() != DataType::QASYMM8, "Only QASYMM8 output supported for QASYMM8 input");
    return Status{};
}

bool NEGEMMAssemblyDispatch::is_activation_supported(const ActivationLayerInfo &activation)
{
    arm_gemm::Activation act = map_to_arm_gemm_activation(activation);
    return act.type != arm_gemm::Activation::Type::None;
}

void NEGEMMAssemblyDispatch::configure(const ITensor *a, const ITensor *b, const ITensor *c, ITensor *d, const GEMMInfo &gemm_info)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(a, b, d);
    arm_gemm::Activation act = map_to_arm_gemm_activation(gemm_info.activation_info());

    //If we don't support a combination of data types, silently return: it is the caller's responsibility to check if configure() was successful via is_configured()
    if(!NEGEMMAssemblyDispatch::validate(a->info(), b->info(), c != nullptr ? c->info() : nullptr, d->info(), gemm_info))
    {
        return;
    }

    switch(a->info()->data_type())
    {
        case DataType::F32:
            create_arm_gemm<float, float>(_arm_gemm, _memory_group, a, b, c, d, act, gemm_info, _weights_manager);
            break;
#ifdef __aarch64__
        case DataType::U8:
        case DataType::QASYMM8:
            if(d->info()->data_type() == DataType::S32)
            {
                create_arm_gemm<uint8_t, uint32_t>(_arm_gemm, _memory_group, a, b, c, d, act, gemm_info, _weights_manager);
            }
            else
            {
                create_arm_gemm_quant<uint8_t, uint8_t>(_arm_gemm, _memory_group, a, b, c, d, act, gemm_info, _weights_manager);
            }
            break;
        case DataType::S8:
        case DataType::QASYMM8_SIGNED:
            if(d->info()->data_type() == DataType::S32)
            {
                create_arm_gemm<int8_t, int32_t>(_arm_gemm, _memory_group, a, b, c, d, act, gemm_info, _weights_manager);
            }
            else
            {
                create_arm_gemm_quant<int8_t, int8_t>(_arm_gemm, _memory_group, a, b, c, d, act, gemm_info, _weights_manager);
            }
            break;
#endif /* __aarch64__ */
#ifdef __ARM_FEATURE_FP16_VECTOR_ARITHMETIC
        case DataType::F16:
            create_arm_gemm<float16_t, float16_t>(_arm_gemm, _memory_group, a, b, c, d, act, gemm_info, _weights_manager);
            break;
#endif /* __ARM_FEATURE_FP16_VECTOR_ARITHMETIC */
        default:
            break;
    }
}

void NEGEMMAssemblyDispatch::prepare()
{
    ARM_COMPUTE_ERROR_ON(_arm_gemm == nullptr);
    _arm_gemm->prepare();
}

bool NEGEMMAssemblyDispatch::is_configured() const
{
    return _arm_gemm != nullptr && _arm_gemm->is_configured();
}

void NEGEMMAssemblyDispatch::run()
{
    MemoryGroupResourceScope scope_mg(_memory_group);

    ARM_COMPUTE_ERROR_ON(_arm_gemm == nullptr);
    _arm_gemm->run();
}
} //namespace arm_compute
