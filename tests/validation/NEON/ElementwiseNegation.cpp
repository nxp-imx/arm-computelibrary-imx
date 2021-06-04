/*
 * Copyright (c) 2019-2021 Arm Limited.
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
#include "arm_compute/core/Types.h"
#include "arm_compute/runtime/NEON/functions/NEElementwiseUnaryLayer.h"
#include "arm_compute/runtime/Tensor.h"
#include "arm_compute/runtime/TensorAllocator.h"
#include "tests/NEON/Accessor.h"
#include "tests/PaddingCalculator.h"
#include "tests/datasets/ShapeDatasets.h"
#include "tests/framework/Asserts.h"
#include "tests/framework/Macros.h"
#include "tests/framework/datasets/Datasets.h"
#include "tests/validation/Validation.h"
#include "tests/validation/fixtures/ElementwiseUnaryFixture.h"

namespace arm_compute
{
namespace test
{
namespace validation
{
namespace
{
RelativeTolerance<float> tolerance_fp32(0.000001f);
#ifdef __ARM_FEATURE_FP16_VECTOR_ARITHMETIC
RelativeTolerance<float> tolerance_fp16(0.01f);
#endif // __ARM_FEATURE_FP16_VECTOR_ARITHMETIC
} // namespace
TEST_SUITE(NEON)
TEST_SUITE(NegLayer)

template <typename T>
using NENegLayerFixture = NegValidationInPlaceFixture<Tensor, Accessor, NENegLayer, T>;

TEST_SUITE(Float)
#ifdef __ARM_FEATURE_FP16_VECTOR_ARITHMETIC
TEST_SUITE(FP16)
FIXTURE_DATA_TEST_CASE(RunSmall, NENegLayerFixture<half>, framework::DatasetMode::PRECOMMIT, combine(combine(datasets::SmallShapes(),
                                                                                                             framework::dataset::make("DataType", DataType::F16)),
                                                                                                     framework::dataset::make("InPlace", { true, false })))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_fp16);
}
FIXTURE_DATA_TEST_CASE(RunLarge, NENegLayerFixture<half>, framework::DatasetMode::NIGHTLY, combine(combine(datasets::LargeShapes(),
                                                                                                           framework::dataset::make("DataType", DataType::F16)),
                                                                                                   framework::dataset::make("InPlace", { false })))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_fp16);
}

TEST_SUITE_END() // FP16
#endif           // __ARM_FEATURE_FP16_VECTOR_ARITHMETIC

TEST_SUITE(FP32)
FIXTURE_DATA_TEST_CASE(RunSmall, NENegLayerFixture<float>, framework::DatasetMode::ALL, combine(combine(datasets::SmallShapes(),
                                                                                                        framework::dataset::make("DataType", DataType::F32)),
                                                                                                framework::dataset::make("InPlace", { true, false })))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_fp32);
}

FIXTURE_DATA_TEST_CASE(RunLarge, NENegLayerFixture<float>, framework::DatasetMode::NIGHTLY, combine(combine(datasets::LargeShapes(),
                                                                                                            framework::dataset::make("DataType", DataType::F32)),
                                                                                                    framework::dataset::make("InPlace", { false })))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_fp32);
}
TEST_SUITE_END() // FP32
TEST_SUITE_END() // Float

TEST_SUITE(Integer)
TEST_SUITE(S32)
FIXTURE_DATA_TEST_CASE(RunSmall, NENegLayerFixture<int32_t>, framework::DatasetMode::ALL, combine(combine(datasets::SmallShapes(),
                                                                                                          framework::dataset::make("DataType", DataType::S32)),
                                                                                                  framework::dataset::make("InPlace", { true, false })))
{
    // Validate output
    validate(Accessor(_target), _reference);
}

FIXTURE_DATA_TEST_CASE(RunLarge, NENegLayerFixture<int32_t>, framework::DatasetMode::NIGHTLY, combine(combine(datasets::LargeShapes(),
                                                                                                              framework::dataset::make("DataType", DataType::S32)),
                                                                                                      framework::dataset::make("InPlace", { false })))
{
    // Validate output
    validate(Accessor(_target), _reference);
}
TEST_SUITE_END() // S32
TEST_SUITE_END() // Integer

TEST_SUITE_END() // NegLayer
TEST_SUITE_END() // Neon
} // namespace validation
} // namespace test
} // namespace arm_compute
