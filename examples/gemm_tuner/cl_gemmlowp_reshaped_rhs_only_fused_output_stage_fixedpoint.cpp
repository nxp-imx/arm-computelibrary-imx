/*
 * Copyright (c) 2020-2021, 2023 Arm Limited.
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
#ifndef ARM_COMPUTE_CL /* Needed by Utils.cpp to handle OpenCL exceptions properly */
#error "This example needs to be built with -DARM_COMPUTE_CL"
#endif /* ARM_COMPUTE_CL */

#include "arm_compute/core/Helpers.h"
#include "arm_compute/core/KernelDescriptors.h"
#include "arm_compute/core/Types.h"
#include "arm_compute/core/utils/misc/ShapeCalculator.h"
#include "arm_compute/core/utils/quantization/AsymmHelpers.h"
#include "arm_compute/runtime/CL/CLScheduler.h"
#include "arm_compute/runtime/CL/CLTuner.h"

#include "src/gpu/cl/kernels/ClGemmLowpMatrixMultiplyReshapedOnlyRhsKernel.h"
#include "src/gpu/cl/kernels/ClGemmLowpReductionKernel.h"
#include "tests/CL/Helper.h"
#include "utils/command_line/CommandLineOptions.h"
#include "utils/command_line/CommandLineParser.h"
#include "utils/Utils.h"

#include "CommonGemmExampleOptions.h"
#include "GemmTunerHelpers.h"
#include <cstdlib>
#include <memory>

using namespace arm_compute;
using namespace utils;
using namespace arm_compute::opencl::kernels;
using namespace arm_compute::misc::shape_calculator;
using namespace gemm_tuner;

namespace
{
/** Structure holding all tunable gemm configs specific to this example/strategy */
struct GemmConfigs
{
    size_t m0{4};                /**< Number of rows processed by the matrix multiplication */
    size_t n0{4};                /**< Number of columns processed by the matrix multiplication */
    size_t k0{4};                /**< Number of partial accumulations performed by the matrix multiplication */
    size_t h0{1};                /**< Number of horizontal blocks of size (k0xn0) stored on the same output row */
    bool   interleave_rhs{true}; /**< Interleave rhs matrix */
    bool   transpose_rhs{true};  /**< Transpose rhs matrix */
};

/** Formatted output of the GemmConfigs type
 *
 * @param[out] os      Output stream.
 * @param[in]  configs Tunable configurations to output
 *
 * @return Modified output stream.
 */
::std::ostream &operator<<(::std::ostream &os, const GemmConfigs &configs)
{
    std::string false_str = std::string("false");
    std::string true_str  = std::string("true");

    os << "m0 : " << configs.m0 << std::endl;
    os << "n0 : " << configs.n0 << std::endl;
    os << "k0 : " << configs.k0 << std::endl;
    os << "h0 : " << configs.h0 << std::endl;
    os << "interleave_rhs : " << (configs.interleave_rhs ? true_str : false_str) << std::endl;
    os << "transpose_rhs : " << (configs.transpose_rhs ? true_str : false_str) << std::endl;
    return os;
}

/** Command line options for gemm configs */
class GemmConfigOptions
{
public:
    /** Constructor
     *
     * @param[in,out] parser A parser on which "parse()" hasn't been called yet.
     */
    GemmConfigOptions(CommandLineParser &parser)
        : m0(parser.add_positional_option<SimpleOption<size_t>>("m0", 4)),
          n0(parser.add_positional_option<SimpleOption<size_t>>("n0", 4)),
          k0(parser.add_positional_option<SimpleOption<size_t>>("k0", 4)),
          h0(parser.add_positional_option<SimpleOption<size_t>>("h0", 1)),
          interleave_rhs(parser.add_positional_option<SimpleOption<size_t>>("interleave_rhs", 1)),
          transpose_rhs(parser.add_positional_option<SimpleOption<size_t>>("transpose_rhs", 1))
    {
        m0->set_help("Number of rows processed by the matrix multiplication");
        n0->set_help("Number of columns processed by the matrix multiplication");
        k0->set_help("Number of partial accumulations performed by the matrix multiplication");
        h0->set_help("Number of horizontal blocks of size (k0xn0) stored on the same output row");
        interleave_rhs->set_help("Interleave rhs matrix (1) / Do not interleave rhs matrix (0)");
        transpose_rhs->set_help("Transpose rhs matrix (1) / Do not transpose rhs matrix (0)");
    }
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    GemmConfigOptions(const GemmConfigOptions &) = delete;
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    GemmConfigOptions &operator=(const GemmConfigOptions &) = delete;
    /** Allow instances of this class to be moved */
    GemmConfigOptions(GemmConfigOptions &&) = default;
    /** Allow instances of this class to be moved */
    GemmConfigOptions &operator=(GemmConfigOptions &&) = default;
    /** Default destructor */
    ~GemmConfigOptions() = default;

    SimpleOption<size_t> *m0; /**< Number of rows processed by the matrix multiplication option */
    SimpleOption<size_t> *n0; /**< Number of columns processed by the matrix multiplication option */
    SimpleOption<size_t> *k0; /**< Number of partial accumulations performed by the matrix multiplication option */
    SimpleOption<size_t> *h0; /**< Number of horizontal blocks of size (k0xn0) stored on the same output row option */
    SimpleOption<size_t> *interleave_rhs; /**< Interleave rhs matrix option (1 enable; 0 disable) */
    SimpleOption<size_t> *transpose_rhs;  /**< Transpose rhs matrix option (1 enable; 0 disable) */
};

/** Consumes the gemm configuration options and creates a structure containing all information
 *
 * @param[in] options Options to consume
 *
 * @return Structure containing the gemm configurations
 */
GemmConfigs consume_gemm_configs(const GemmConfigOptions &options)
{
    GemmConfigs configs;
    configs.m0             = options.m0->value();
    configs.n0             = options.n0->value();
    configs.k0             = options.k0->value();
    configs.h0             = options.h0->value();
    configs.interleave_rhs = options.interleave_rhs->value() != 0;
    configs.transpose_rhs  = options.transpose_rhs->value() != 0;
    return configs;
}

} // namespace

using ClGemmLowpMatrixMultiplyReshapedOnlyRhs =
    test::CLSynthetizeOperator<ClGemmLowpMatrixMultiplyReshapedOnlyRhsKernel>;
using ClGemmLowpMatrixAReduction = test::CLSynthetizeOperator<ClGemmLowpMatrixAReductionKernel>;

class CLGEMMLowpMatrixMultiplyReshapedOnlyRHSFusedOutputStageFixedpointExample : public Example
{
public:
    bool do_setup(int argc, char **argv) override
    {
        // Default parameters
        CommonGemmExampleParams params;
        GemmConfigs             configs;

        // Parse command line options
        CommandLineParser        parser;
        CommonGemmExampleOptions param_options(parser, DataType::QASYMM8);
        GemmConfigOptions        config_options(parser);

        parser.parse(argc, argv);
        if (param_options.help->is_set() && param_options.help->value())
        {
            parser.print_help(argv[0]);
            return false;
        }
        if (!parser.validate())
        {
            // Invalid arguments. Use default parameters and configs
            std::cerr << "Invalid arguments." << std::endl;
            parser.print_help(argv[0]);
            std::cerr << "Falling back to default parameters and configs" << std::endl;
        }
        else
        {
            params  = consume_common_gemm_example_parameters(param_options);
            configs = consume_gemm_configs(config_options);
        }

        std::cout << "Gemm parameters:" << std::endl;
        std::cout << params << std::endl;
        std::cout << "Gemm configurations:" << std::endl;
        std::cout << configs << std::endl;

        tuner.set_tuner_mode(params.tuner_mode);

        CLScheduler::get().default_init(&tuner);

        lhs.allocator()->init(TensorInfo(TensorShape(params.K, params.M, params.B), 1, params.data_type));
        rhs.allocator()->init(TensorInfo(TensorShape(params.N, params.K, params.B), 1, params.data_type));
        bias.allocator()->init(TensorInfo(TensorShape(params.N), 1, DataType::S32));
        dst.allocator()->init(TensorInfo(TensorShape(params.N, params.M, params.B), 1, params.data_type));

        // Set arbitrary quantization information (non-zero offset to ensure offset contribution stage is included)
        // Could be extended in the future to include a user-controlled option for offset == 0
        const QuantizationInfo q_info{0.012, 3};
        lhs.info()->set_quantization_info(q_info);
        rhs.info()->set_quantization_info(q_info);
        bias.info()->set_quantization_info(q_info);
        dst.info()->set_quantization_info(q_info);

        GEMMLHSMatrixInfo lhs_info;
        lhs_info.m0 = configs.m0;
        lhs_info.k0 = configs.k0;

        GEMMRHSMatrixInfo rhs_info;
        rhs_info.n0                 = configs.n0;
        rhs_info.k0                 = configs.k0;
        rhs_info.h0                 = configs.h0;
        rhs_info.interleave         = configs.interleave_rhs;
        rhs_info.transpose          = configs.transpose_rhs;
        rhs_info.export_to_cl_image = false; // CL image not supported for quantized cases yet

        if (rhs_info.h0 == 0)
        {
            rhs_info.h0 = std::max(static_cast<unsigned int>(params.N) / rhs_info.n0, 1U);
        }

        rhs_reshaped.allocator()->init(
            TensorInfo(compute_rhs_reshaped_shape(*rhs.info(), rhs_info), 1, params.data_type));
        rhs_reshaped.info()->set_quantization_info(q_info);
        if (rhs_info.export_to_cl_image)
        {
            if (!examples::gemm_tuner_helpers::update_padding_for_cl_image(rhs_reshaped.info()))
            {
                std::cerr << "cl_image is not supported on the device, disable export_to_cl_image" << std::endl;
                return false;
            }
        }

        // Configure output stage for quantized case
        GEMMLowpOutputStageInfo gemmlowp_output_stage;
        gemmlowp_output_stage.type             = GEMMLowpOutputStageType::QUANTIZE_DOWN_FIXEDPOINT;
        gemmlowp_output_stage.output_data_type = dst.info()->data_type();
        gemmlowp_output_stage.gemmlowp_offset  = 0;
        {
            gemmlowp_output_stage.is_quantized_per_channel = false;
            // Num_filters is 1 unless quantized type is of per_channel type. Could be extended in the future to support per-channel quantization.
            const unsigned int num_filters = 1;

            dst_multipliers.allocator()->init(TensorInfo(TensorShape(num_filters), 1, DataType::S32));
            dst_shifts.allocator()->init(TensorInfo(TensorShape(num_filters), 1, DataType::S32));

            gemmlowp_output_stage.gemmlowp_multipliers.resize(num_filters);
            gemmlowp_output_stage.gemmlowp_shifts.resize(num_filters);
            quantization::compute_quantized_multipliers_and_shifts(lhs.info(), rhs.info(), dst.info(),
                                                                   gemmlowp_output_stage.gemmlowp_multipliers.data(),
                                                                   gemmlowp_output_stage.gemmlowp_shifts.data());
            gemmlowp_output_stage.gemmlowp_multiplier = gemmlowp_output_stage.gemmlowp_multipliers[0];
            gemmlowp_output_stage.gemmlowp_shift      = gemmlowp_output_stage.gemmlowp_shifts[0];

            // No fused activation
            PixelValue min_val{};
            PixelValue max_val{};
            std::tie(min_val, max_val) = get_min_max(dst.info()->data_type());

            auto min_activation = min_val.get<int32_t>();
            auto max_activation = max_val.get<int32_t>();

            // Set the GEMMLowp output stage info
            gemmlowp_output_stage.gemmlowp_offset    = dst.info()->quantization_info().uniform().offset;
            gemmlowp_output_stage.gemmlowp_min_bound = min_activation;
            gemmlowp_output_stage.gemmlowp_max_bound = max_activation;
        }

        GEMMKernelInfo gemm_info;
        gemm_info.m                       = params.M;
        gemm_info.n                       = params.N;
        gemm_info.k                       = params.K;
        gemm_info.depth_output_gemm3d     = 0;
        gemm_info.reinterpret_input_as_3d = false;
        gemm_info.broadcast_bias          = true;
        gemm_info.fp_mixed_precision      = false;
        gemm_info.has_pad_y               = false;
        gemm_info.mult_transpose1xW_width = configs.h0;
        gemm_info.lhs_info                = lhs_info;
        gemm_info.rhs_info                = rhs_info;
        gemm_info.a_offset                = lhs.info()->quantization_info().uniform().offset;
        gemm_info.b_offset                = rhs.info()->quantization_info().uniform().offset;
        gemm_info.output_stage            = gemmlowp_output_stage;

        // Initialize Matrix A reduction kernel only if _b_offset is not equal to 0
        if (gemm_info.b_offset != 0)
        {
            const TensorInfo info_vector_sum_row(compute_reductionB_shape(*lhs.info()), 1, DataType::S32);
            vector_sum_row.allocator()->init(info_vector_sum_row);

            mtx_a_reduction = std::make_unique<ClGemmLowpMatrixAReduction>();

            if (!mtx_a_reduction->validate(lhs.info(), vector_sum_row.info(), GEMMLowpReductionKernelInfo{}))
            {
                std::cerr << "Invalid arguments for CLGEMMLowpMatrixAReductionKernel." << std::endl;
                return false;
            }

            mtx_a_reduction->configure(lhs.info(), vector_sum_row.info(), GEMMLowpReductionKernelInfo{});
        }
        // Initialize matrix B reduction kernel only if _a_offset is not equal to 0
        if (gemm_info.a_offset != 0)
        {
            const TensorInfo info_vector_sum_col(compute_reductionA_shape(*rhs.info()), 1, DataType::S32);
            vector_sum_col.allocator()->init(info_vector_sum_col);
            // There's no need for a Matrix B reduction kernel as this is assumed to be run only once in the prepare stage
        }

        // Validate argments
        if (!gemm.validate(lhs.info(), rhs_reshaped.info(), dst.info(), gemm_info,
                           gemm_info.a_offset == 0 ? nullptr : vector_sum_col.info(),
                           gemm_info.b_offset == 0 ? nullptr : vector_sum_row.info(), bias.info(),
                           dst_multipliers.info(), dst_shifts.info()))
        {
            std::cerr << "Invalid arguments for ClGemmLowpMatrixMultiplyReshapedOnlyRhsKernel." << std::endl;
            return false;
        }

        // Configure function
        gemm.configure(lhs.info(), rhs_reshaped.info(), dst.info(), gemm_info,
                       gemm_info.a_offset == 0 ? nullptr : vector_sum_col.info(),
                       gemm_info.b_offset == 0 ? nullptr : vector_sum_row.info(), bias.info(), dst_multipliers.info(),
                       dst_shifts.info());

        // Allocate tensors
        lhs.allocator()->allocate();
        rhs.allocator()->allocate();
        rhs_reshaped.allocator()->allocate();
        bias.allocator()->allocate();
        dst.allocator()->allocate();
        vector_sum_col.allocator()->allocate();
        vector_sum_row.allocator()->allocate();
        dst_multipliers.allocator()->allocate();
        dst_shifts.allocator()->allocate();

        return true;
    }
    void do_run() override
    {
        if (mtx_a_reduction != nullptr)
        {
            ITensorPack red_pack({{ACL_SRC, &lhs}, {ACL_DST, &dst}});
            mtx_a_reduction->run(red_pack);
        }

        ITensorPack gemm_pack({{ACL_SRC_0, &lhs},
                               {ACL_SRC_1, &rhs},
                               {ACL_BIAS, &bias},
                               {ACL_VEC_COL_SUM, &vector_sum_col},
                               {ACL_VEC_ROW_SUM, &vector_sum_row},
                               {ACL_SHIFTS, &dst_shifts},
                               {ACL_MULTIPLIERS, &dst_multipliers},
                               {ACL_DST, &dst}});
        gemm.run(gemm_pack);

        // Make sure all the OpenCL jobs are done executing:
        CLScheduler::get().sync();
    }

    void do_teardown() override
    {
    }

private:
    CLTensor                                    lhs{};
    CLTensor                                    rhs{};
    CLTensor                                    rhs_reshaped{};
    CLTensor                                    bias{};
    CLTensor                                    dst{};
    CLTensor                                    vector_sum_col{};
    CLTensor                                    vector_sum_row{};
    CLTensor                                    dst_multipliers{};
    CLTensor                                    dst_shifts{};
    CLTuner                                     tuner{};
    ClGemmLowpMatrixMultiplyReshapedOnlyRhs     gemm{};
    std::unique_ptr<ClGemmLowpMatrixAReduction> mtx_a_reduction{nullptr};
};

/** Main test program for gemmlowp reshaped rhs only with fused output stage fixedpoint
 *
 * @param[in] argc Number of arguments
 * @param[in] argv Arguments ( [optional] M, [optional] N, [optional] K, [optional] B, [optional] m0, [optional] n0, [optional] k0, [optional] h0, [optional] interleave_rhs, [optional] transpose_rhs )
 */
int main(int argc, char **argv)
{
    return run_example<CLGEMMLowpMatrixMultiplyReshapedOnlyRHSFusedOutputStageFixedpointExample>(argc, argv);
}
