/*
 * Copyright (c) 2018-2021 Arm Limited.
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
#include "arm_compute/runtime/CL/CLTensor.h"
#include "arm_compute/runtime/CL/CLTensorAllocator.h"
#include "arm_compute/runtime/CL/functions/CLElementwiseOperations.h"
#include "tests/CL/CLAccessor.h"
#include "tests/PaddingCalculator.h"
#include "tests/datasets/ConvertPolicyDataset.h"
#include "tests/datasets/ShapeDatasets.h"
#include "tests/framework/Asserts.h"
#include "tests/framework/Macros.h"
#include "tests/framework/datasets/Datasets.h"
#include "tests/validation/Validation.h"
#include "tests/validation/fixtures/ElementwiseOperationsFixture.h"

namespace arm_compute
{
namespace test
{
namespace validation
{
namespace
{
RelativeTolerance<float> tolerance_fp32(0.000001f);
RelativeTolerance<float> tolerance_fp16(0.001f);
AbsoluteTolerance<float> tolerance_qsymm16(1);

/** Input data sets **/
const auto ElementwiseSquaredDiffU8Dataset = combine(combine(framework::dataset::make("DataType", DataType::U8), framework::dataset::make("DataType", DataType::U8)),
                                                     framework::dataset::make("DataType",
                                                                              DataType::U8));
const auto ElementwiseSquaredDiffQASYMM8Dataset = combine(combine(framework::dataset::make("DataType", DataType::QASYMM8), framework::dataset::make("DataType", DataType::QASYMM8)),
                                                          framework::dataset::make("DataType",
                                                                                   DataType::QASYMM8));
const auto ElementwiseSquaredDiffQSYMM16Dataset = combine(combine(framework::dataset::make("DataType", DataType::QSYMM16), framework::dataset::make("DataType", DataType::QSYMM16)),
                                                          framework::dataset::make("DataType",
                                                                                   DataType::QSYMM16));
const auto ElementwiseSquaredDiffS16Dataset = combine(combine(framework::dataset::make("DataType", { DataType::S16 }), framework::dataset::make("DataType", DataType::S16)),
                                                      framework::dataset::make("DataType", DataType::S16));
const auto ElementwiseSquaredDiffFP16Dataset = combine(combine(framework::dataset::make("DataType", DataType::F16), framework::dataset::make("DataType", DataType::F16)),
                                                       framework::dataset::make("DataType", DataType::F16));
const auto ElementwiseSquaredDiffFP32Dataset = combine(combine(framework::dataset::make("DataType", DataType::F32), framework::dataset::make("DataType", DataType::F32)),
                                                       framework::dataset::make("DataType", DataType::F32));
const auto EmptyActivationFunctionsDataset = framework::dataset::make("ActivationInfo",
{ ActivationLayerInfo() });
const auto ActivationFunctionsDataset = framework::dataset::make("ActivationInfo",
{
    ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::BOUNDED_RELU, 0.75f, 0.25f),
    ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::LOGISTIC, 0.75f, 0.25f)
});
const auto InPlaceDataSet    = framework::dataset::make("InPlace", { false, true });
const auto OutOfPlaceDataSet = framework::dataset::make("InPlace", { false });
} // namespace

TEST_SUITE(CL)
TEST_SUITE(ElementwiseSquaredDiff)

// *INDENT-OFF*
// clang-format off
DATA_TEST_CASE(Validate, framework::DatasetMode::ALL, zip(zip(zip(
               framework::dataset::make("Input1Info", { TensorInfo(TensorShape(32U, 13U, 2U), 1, DataType::U8),
                                                        TensorInfo(TensorShape(32U, 13U, 2U), 1, DataType::U8),      // Invalid data type combination
                                                        TensorInfo(TensorShape(32U, 13U, 2U), 1, DataType::F32),     // Mismatching shapes
                                                      }),
               framework::dataset::make("Input2Info",{ TensorInfo(TensorShape(32U, 13U, 2U), 1, DataType::U8),
                                                       TensorInfo(TensorShape(32U, 13U, 2U), 1, DataType::S16),
                                                       TensorInfo(TensorShape(48U, 11U, 2U), 1, DataType::F32),
                                                     })),
               framework::dataset::make("OutputInfo",{ TensorInfo(TensorShape(32U, 13U, 2U), 1, DataType::U8),
                                                       TensorInfo(TensorShape(32U, 13U, 2U), 1, DataType::U8),
                                                       TensorInfo(TensorShape(48U, 11U, 2U), 1, DataType::F32),
                                                     })),
               framework::dataset::make("Expected", { true, false, false})),
               input1_info, input2_info, output_info, expected)
{
    ARM_COMPUTE_EXPECT(bool(CLElementwiseSquaredDiff::validate(&input1_info.clone()->set_is_resizable(false), &input2_info.clone()->set_is_resizable(false), &output_info.clone()->set_is_resizable(false))) == expected, framework::LogLevel::ERRORS);
}
// clang-format on
// *INDENT-ON*

template <typename T>
using CLElementwiseSquaredDiffFixture = ElementwiseSquaredDiffValidationFixture<CLTensor, CLAccessor, CLElementwiseSquaredDiff, T>;

TEST_SUITE(Integer)
TEST_SUITE(U8)
FIXTURE_DATA_TEST_CASE(RunSmall, CLElementwiseSquaredDiffFixture<uint8_t>, framework::DatasetMode::PRECOMMIT, combine(combine(datasets::SmallShapes(), ElementwiseSquaredDiffU8Dataset),
                                                                                                                      OutOfPlaceDataSet))
{
    // Validate output
    validate(CLAccessor(_target), _reference);
}
TEST_SUITE_END()

TEST_SUITE(S16)
FIXTURE_DATA_TEST_CASE(RunSmall, CLElementwiseSquaredDiffFixture<int16_t>, framework::DatasetMode::ALL, combine(combine(datasets::SmallShapes(), ElementwiseSquaredDiffS16Dataset),
                                                                                                                OutOfPlaceDataSet))
{
    // Validate output
    validate(CLAccessor(_target), _reference);
}
TEST_SUITE_END()
TEST_SUITE_END()

template <typename T>
using CLElementwiseSquaredDiffQuantizedFixture = ElementwiseSquaredDiffValidationQuantizedFixture<CLTensor, CLAccessor, CLElementwiseSquaredDiff, T>;

TEST_SUITE(Quantized)
TEST_SUITE(QASYMM8)
FIXTURE_DATA_TEST_CASE(RunSmall, CLElementwiseSquaredDiffQuantizedFixture<uint8_t>, framework::DatasetMode::PRECOMMIT, combine(combine(combine(combine(combine(datasets::SmallShapes(),
                       ElementwiseSquaredDiffQASYMM8Dataset),
                       framework::dataset::make("Src0QInfo", { QuantizationInfo(5.f / 255.f, 20) })),
                       framework::dataset::make("Src1QInfo", { QuantizationInfo(2.f / 255.f, 10) })),
                       framework::dataset::make("OutQInfo", { QuantizationInfo(1.f / 255.f, 5) })),
                       OutOfPlaceDataSet))
{
    // Validate output
    validate(CLAccessor(_target), _reference, tolerance_fp32, 0.01);
}
TEST_SUITE_END()
TEST_SUITE(QSYMM16)
FIXTURE_DATA_TEST_CASE(RunSmall, CLElementwiseSquaredDiffQuantizedFixture<int16_t>, framework::DatasetMode::PRECOMMIT, combine(combine(combine(combine(combine(datasets::SmallShapes(),
                       ElementwiseSquaredDiffQSYMM16Dataset),
                       framework::dataset::make("Src0QInfo", { QuantizationInfo(1.f / 32768.f, 0), QuantizationInfo(5.f / 32768.f, 0) })),
                       framework::dataset::make("Src1QInfo", { QuantizationInfo(2.f / 32768.f, 0), QuantizationInfo(5.f / 32768.f, 0) })),
                       framework::dataset::make("OutQInfo", { QuantizationInfo(5.f / 32768.f, 0) })),
                       OutOfPlaceDataSet))
{
    // Validate output
    validate(CLAccessor(_target), _reference, tolerance_qsymm16);
}
TEST_SUITE_END()
TEST_SUITE_END()

template <typename T>
using CLElementwiseSquaredDiffFloatFixture = ElementwiseSquaredDiffValidationFloatFixture<CLTensor, CLAccessor, CLElementwiseSquaredDiff, T>;

TEST_SUITE(Float)
TEST_SUITE(FP16)
FIXTURE_DATA_TEST_CASE(RunSmall, CLElementwiseSquaredDiffFloatFixture<half>, framework::DatasetMode::ALL, combine(combine(combine(datasets::SmallShapes(), ElementwiseSquaredDiffFP16Dataset),
                                                                                                                  EmptyActivationFunctionsDataset),
                                                                                                                  OutOfPlaceDataSet))
{
    // Validate output
    validate(CLAccessor(_target), _reference, tolerance_fp16, 0.01);
}
FIXTURE_DATA_TEST_CASE(RunWithActivation, CLElementwiseSquaredDiffFloatFixture<half>, framework::DatasetMode::ALL, combine(combine(combine(datasets::TinyShapes(), ElementwiseSquaredDiffFP16Dataset),
                       ActivationFunctionsDataset),
                       OutOfPlaceDataSet))
{
    // Validate output
    validate(CLAccessor(_target), _reference, tolerance_fp16, 0.01);
}
TEST_SUITE_END()

TEST_SUITE(FP32)
FIXTURE_DATA_TEST_CASE(RunSmall, CLElementwiseSquaredDiffFloatFixture<float>, framework::DatasetMode::ALL, combine(combine(combine(datasets::SmallShapes(), ElementwiseSquaredDiffFP32Dataset),
                                                                                                                   EmptyActivationFunctionsDataset),
                                                                                                                   OutOfPlaceDataSet))
{
    // Validate output
    validate(CLAccessor(_target), _reference, tolerance_fp32);
}
FIXTURE_DATA_TEST_CASE(RunWithActivation, CLElementwiseSquaredDiffFloatFixture<float>, framework::DatasetMode::ALL, combine(combine(combine(datasets::TinyShapes(), ElementwiseSquaredDiffFP32Dataset),
                       ActivationFunctionsDataset),
                       OutOfPlaceDataSet))
{
    // Validate output
    validate(CLAccessor(_target), _reference, tolerance_fp32);
}
template <typename T>
using CLElementwiseSquaredDiffBroadcastFloatFixture = ElementwiseSquaredDiffBroadcastValidationFloatFixture<CLTensor, CLAccessor, CLElementwiseSquaredDiff, T>;

FIXTURE_DATA_TEST_CASE(RunSmallBroadcast, CLElementwiseSquaredDiffBroadcastFloatFixture<float>, framework::DatasetMode::ALL, combine(combine(combine(datasets::SmallShapesBroadcast(),
                       ElementwiseSquaredDiffFP32Dataset),
                       EmptyActivationFunctionsDataset),
                       OutOfPlaceDataSet))
{
    // Validate output
    validate(CLAccessor(_target), _reference, tolerance_fp32);
}
FIXTURE_DATA_TEST_CASE(RunWithActivationBroadcast, CLElementwiseSquaredDiffBroadcastFloatFixture<float>, framework::DatasetMode::ALL, combine(combine(combine(datasets::TinyShapesBroadcast(),
                       ElementwiseSquaredDiffFP32Dataset),
                       ActivationFunctionsDataset),
                       OutOfPlaceDataSet))
{
    // Validate output
    validate(CLAccessor(_target), _reference, tolerance_fp32);
}
TEST_SUITE_END()
TEST_SUITE_END()

TEST_SUITE_END() // ElementwiseSquaredDiff
TEST_SUITE_END() // CL
} // namespace validation
} // namespace test
} // namespace arm_compute
