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
 * FITCLSS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONCLCTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "arm_compute/core/TensorShape.h"
#include "arm_compute/core/Types.h"
#include "arm_compute/runtime/CL/CLTensor.h"
#include "arm_compute/runtime/CL/CLTensorAllocator.h"
#include "arm_compute/runtime/CL/functions/CLFullyConnectedLayer.h"
#include "framework/Macros.h"
#include "framework/datasets/Datasets.h"
#include "tests/CL/CLAccessor.h"
#include "tests/TypePrinter.h"
#include "tests/datasets_new/system_tests/alexnet/AlexNetFullyConnectedLayerDataset.h"
#include "tests/datasets_new/system_tests/googlenet/inceptionv1/GoogLeNetInceptionV1FullyConnectedLayerDataset.h"
#include "tests/datasets_new/system_tests/lenet5/LeNet5FullyConnectedLayerDataset.h"
#include "tests/fixtures_new/FullyConnectedLayerFixture.h"

namespace arm_compute
{
namespace test
{
namespace
{
const auto data_types = framework::dataset::make("DataType", { DataType::F16, DataType::F32, DataType::QS8, DataType::QS16 });
} // namespace

using CLFullyConnectedLayerFixture = FullyConnectedLayerFixture<CLTensor, CLFullyConnectedLayer, CLAccessor>;

TEST_SUITE(CL)

REGISTER_FIXTURE_DATA_TEST_CASE(AlexNetFullyConnectedLayer, CLFullyConnectedLayerFixture, framework::DatasetMode::ALL,
                                framework::dataset::combine(framework::dataset::combine(datasets::AlexNetFullyConnectedLayerDataset(),
                                                                                        data_types),
                                                            framework::dataset::make("Batches", { 1, 4 })));

REGISTER_FIXTURE_DATA_TEST_CASE(LeNet5FullyConnectedLayer, CLFullyConnectedLayerFixture, framework::DatasetMode::ALL,
                                framework::dataset::combine(framework::dataset::combine(datasets::LeNet5FullyConnectedLayerDataset(),
                                                                                        data_types),
                                                            framework::dataset::make("Batches", { 1, 4 })));

REGISTER_FIXTURE_DATA_TEST_CASE(GoogLeNetInceptionV1FullyConnectedLayer, CLFullyConnectedLayerFixture, framework::DatasetMode::ALL,
                                framework::dataset::combine(framework::dataset::combine(datasets::GoogLeNetInceptionV1FullyConnectedLayerDataset(),
                                                                                        data_types),
                                                            framework::dataset::make("Batches", { 1, 4 })));

TEST_SUITE(NIGHTLY)
REGISTER_FIXTURE_DATA_TEST_CASE(AlexNetFullyConnectedLayer, CLFullyConnectedLayerFixture, framework::DatasetMode::NIGHTLY,
                                framework::dataset::combine(framework::dataset::combine(datasets::AlexNetFullyConnectedLayerDataset(),
                                                                                        data_types),
                                                            framework::dataset::make("Batches", 8)));

REGISTER_FIXTURE_DATA_TEST_CASE(LeNet5FullyConnectedLayer, CLFullyConnectedLayerFixture, framework::DatasetMode::NIGHTLY,
                                framework::dataset::combine(framework::dataset::combine(datasets::LeNet5FullyConnectedLayerDataset(),
                                                                                        data_types),
                                                            framework::dataset::make("Batches", 8)));

REGISTER_FIXTURE_DATA_TEST_CASE(GoogLeNetInceptionV1FullyConnectedLayer, CLFullyConnectedLayerFixture, framework::DatasetMode::NIGHTLY,
                                framework::dataset::combine(framework::dataset::combine(datasets::GoogLeNetInceptionV1FullyConnectedLayerDataset(),
                                                                                        data_types),
                                                            framework::dataset::make("Batches", 8)));
TEST_SUITE_END()
TEST_SUITE_END()
} // namespace test
} // namespace arm_compute
