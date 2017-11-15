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
#include "arm_compute/core/NEON/kernels/NEGEMMInterleaveBlockedKernel.h"
#include "arm_compute/core/NEON/kernels/arm64/NEGEMMLowpAArch64Kernel.h"
#include "arm_compute/core/Types.h"
#include "arm_compute/runtime/NEON/functions/NEGEMMLowpAssemblyMatrixMultiplyCore.h"
#include "arm_compute/runtime/NEON/functions/NEGEMMLowpMatrixMultiplyCore.h"
#include "arm_compute/runtime/NEON/functions/NEGEMMLowpOutputStage.h"
#include "arm_compute/runtime/Tensor.h"
#include "arm_compute/runtime/TensorAllocator.h"
#include "tests/NEON/Accessor.h"
#include "tests/NEON/Helper.h"
#include "tests/PaddingCalculator.h"
#include "tests/datasets/LargeGEMMLowpDataset.h"
#include "tests/datasets/ShapeDatasets.h"
#include "tests/datasets/SmallGEMMLowpDataset.h"
#include "tests/framework/Asserts.h"
#include "tests/framework/Macros.h"
#include "tests/framework/datasets/Datasets.h"
#include "tests/validation/Validation.h"
#include "tests/validation/fixtures/GEMMInterleaveBlockedFixture.h"
#include "tests/validation/fixtures/GEMMLowpAssemblyFixture.h"
#include "tests/validation/fixtures/GEMMLowpFixture.h"

namespace arm_compute
{
namespace test
{
namespace validation
{
namespace
{
const auto data_int_blk         = framework::dataset::make("M", 8, 12) * framework::dataset::make("N", 8, 12) * framework::dataset::make("by", 8, 13) * framework::dataset::make("block", 4, 9);
const auto data_int_blk_tr      = framework::dataset::make("M", 8, 17) * framework::dataset::make("N", 8, 14) * framework::dataset::make("by", 12) * framework::dataset::make("block", 4);
const auto data_matrix_multiply = framework::dataset::make("M", 12, 20) * framework::dataset::make("N", 12, 20) * framework::dataset::make("K", 16);
} // namespace

TEST_SUITE(NEON)
TEST_SUITE(ASSEMBLY_MATRIX_MULTIPLY)
using NEGEMMAssemblyFixture = GEMMLowpAssemblyFixture<Tensor, Accessor, NEGEMMLowpAssemblyMatrixMultiplyCore>;
FIXTURE_DATA_TEST_CASE(RunSmall, NEGEMMAssemblyFixture, framework::DatasetMode::PRECOMMIT, data_matrix_multiply)
{
    // Validate output
    validate(Accessor(_target), _reference);
}
TEST_SUITE_END()

TEST_SUITE(GEMMLowp)

TEST_SUITE(INTERLEAVE_BLOCKED)

using NEInterleaveBlocked            = NESynthetizeFunction<NEGEMMInterleaveBlockedKernel>;
using NEGEMMInterleaveBlockedFixture = GEMMInterleaveBlockedValidationFixture<Tensor, Accessor, NEInterleaveBlocked>;
FIXTURE_DATA_TEST_CASE(RunSmall, NEGEMMInterleaveBlockedFixture, framework::DatasetMode::PRECOMMIT, data_int_blk)
{
    // Validate output
    validate(Accessor(_target), _reference);
}
TEST_SUITE_END()

TEST_SUITE(INTERLEAVE_BLOCKED_TRANSPOSED)
using NEInterleaveBlockedTransposed            = NESynthetizeFunction<NEGEMMInterleaveBlockedKernel>;
using NEGEMMInterleaveBlockedTransposedFixture = GEMMInterleaveBlockedValidationFixture<Tensor, Accessor, NEInterleaveBlockedTransposed, true>;
FIXTURE_DATA_TEST_CASE(RunSmall, NEGEMMInterleaveBlockedTransposedFixture, framework::DatasetMode::PRECOMMIT, data_int_blk_tr)
{
    // Validate output
    validate(Accessor(_target), _reference);
}

TEST_SUITE_END()

TEST_SUITE(MatrixMultiplyCore)
using NEGEMMLowpMatrixMultiplyCoreFixture = GEMMLowpMatrixMultiplyCoreValidationFixture<Tensor, Accessor, NEGEMMLowpMatrixMultiplyCore>;

DATA_TEST_CASE(Configuration, framework::DatasetMode::ALL, framework::dataset::concat(datasets::SmallGEMMLowpDataset(), datasets::LargeGEMMLowpDataset()),
               shape_a, shape_b, shape_c, a_offset, b_offset)
{
    // Create tensors
    Tensor a = create_tensor<Tensor>(shape_a, DataType::QASYMM8);
    Tensor b = create_tensor<Tensor>(shape_b, DataType::QASYMM8);
    Tensor c = create_tensor<Tensor>(shape_c, DataType::S32);

    a.info()->set_quantization_info(QuantizationInfo(1.0f / 255, a_offset));
    b.info()->set_quantization_info(QuantizationInfo(1.0f / 255, b_offset));

    ARM_COMPUTE_EXPECT(a.info()->is_resizable(), framework::LogLevel::ERRORS);
    ARM_COMPUTE_EXPECT(b.info()->is_resizable(), framework::LogLevel::ERRORS);
    ARM_COMPUTE_EXPECT(c.info()->is_resizable(), framework::LogLevel::ERRORS);

    // Create and configure function
    NEGEMMLowpMatrixMultiplyCore gemmlowp_mm;
    gemmlowp_mm.configure(&a, &b, &c);
}

FIXTURE_DATA_TEST_CASE(RunSmall, NEGEMMLowpMatrixMultiplyCoreFixture, framework::DatasetMode::ALL, datasets::SmallGEMMLowpDataset())
{
    // Validate output
    validate(Accessor(_target), _reference);
}

FIXTURE_DATA_TEST_CASE(RunLarge, NEGEMMLowpMatrixMultiplyCoreFixture, framework::DatasetMode::NIGHTLY, datasets::LargeGEMMLowpDataset())
{
    // Validate output
    validate(Accessor(_target), _reference);
}

TEST_SUITE_END() // MatrixMultiplyCore

TEST_SUITE(OutputStage)

TEST_SUITE(QuantizeDownInt32ToUint8Scale)

using NEGEMMLowpQuantizeDownInt32ToUint8ScaleFixture = GEMMLowpQuantizeDownInt32ToUint8ScaleValidationFixture<Tensor, Accessor, NEGEMMLowpQuantizeDownInt32ToUint8Scale>;

const auto quantize_down_int32_to_uint8_scale_cases = framework::dataset::make("result_offset", -4, 4) * framework::dataset::make("result_mult_int", 1, 3) * framework::dataset::make("result_shift", 2,
                                                      4);

DATA_TEST_CASE(Configuration, framework::DatasetMode::ALL, combine(framework::dataset::concat(datasets::SmallShapes(), datasets::LargeShapes()), quantize_down_int32_to_uint8_scale_cases),
               shape, result_offset, result_mult_int, result_shift)
{
    // Create tensors
    Tensor in  = create_tensor<Tensor>(shape, DataType::S32);
    Tensor out = create_tensor<Tensor>(shape, DataType::QASYMM8);

    ARM_COMPUTE_EXPECT(in.info()->is_resizable(), framework::LogLevel::ERRORS);
    ARM_COMPUTE_EXPECT(out.info()->is_resizable(), framework::LogLevel::ERRORS);

    // Create and configure function
    NEGEMMLowpQuantizeDownInt32ToUint8Scale output_stage;
    output_stage.configure(&in, &out, result_offset, result_mult_int, result_shift);

    // Validate valid region
    const ValidRegion valid_region = shape_to_valid_region(shape);
    validate(in.info()->valid_region(), valid_region);
    validate(out.info()->valid_region(), valid_region);

    // Validate padding
    const PaddingSize padding = PaddingCalculator(shape.x(), 16).required_padding();
    validate(in.info()->padding(), padding);
    validate(out.info()->padding(), padding);
}

FIXTURE_DATA_TEST_CASE(RunSmall, NEGEMMLowpQuantizeDownInt32ToUint8ScaleFixture, framework::DatasetMode::ALL, combine(datasets::SmallShapes(), quantize_down_int32_to_uint8_scale_cases))
{
    // Validate output
    validate(Accessor(_target), _reference);
}

FIXTURE_DATA_TEST_CASE(RunLarge, NEGEMMLowpQuantizeDownInt32ToUint8ScaleFixture, framework::DatasetMode::NIGHTLY, combine(datasets::LargeShapes(), quantize_down_int32_to_uint8_scale_cases))
{
    // Validate output
    validate(Accessor(_target), _reference);
}

TEST_SUITE_END() // QuantizeDownInt32ToUint8Scale

TEST_SUITE_END() // OutputStage

TEST_SUITE_END() // GEMMLowp
TEST_SUITE_END() // NEON
} // namespace validation
} // namespace test
} // namespace arm_compute
