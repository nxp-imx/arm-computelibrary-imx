/*
 * Copyright (c) 2017-2018 ARM Limited.
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
#include "arm_compute/runtime/CL/functions/CLFastCorners.h"
#include "arm_compute/runtime/Tensor.h"
#include "arm_compute/runtime/TensorAllocator.h"
#include "tests/CL/CLAccessor.h"
#include "tests/CL/CLArrayAccessor.h"
#include "tests/PaddingCalculator.h"
#include "tests/datasets/ShapeDatasets.h"
#include "tests/framework/Asserts.h"
#include "tests/framework/Macros.h"
#include "tests/framework/datasets/Datasets.h"
#include "tests/validation/FastValidation.h"
#include "tests/validation/fixtures/FastCornersFixture.h"

namespace arm_compute
{
namespace test
{
namespace validation
{
namespace
{
/* Radius of the Bresenham circle around the candidate point */
const unsigned int bresenham_radius = 3;
/* Allowed percentage of keypoints missing for target */
const float allowed_missing = 10.f;
/* Allowed percentage of keypoints mismatching between target and reference */
const float allowed_mismatching = 10.f;
/* Tolerance used to compare corner strengths */
const AbsoluteTolerance<float> tolerance(0.5f);
} // namespace

TEST_SUITE(CL)
TEST_SUITE(FastCorners)

DATA_TEST_CASE(Configuration, framework::DatasetMode::ALL, combine(combine(combine(concat(datasets::Small2DShapes(), datasets::Large2DShapes()),
                                                                                   framework::dataset::make("Format", Format::U8)),
                                                                           framework::dataset::make("SuppressNonMax", { false, true })),
                                                                   framework::dataset::make("BorderMode", BorderMode::UNDEFINED)),
               shape, format, suppress_nonmax, border_mode)
{
    std::mt19937                           gen(library->seed());
    std::uniform_int_distribution<uint8_t> int_dist(0, 255);
    std::uniform_real_distribution<float>  real_dist(0, 255);

    const uint8_t constant_border_value = int_dist(gen);
    const float   threshold             = real_dist(gen);

    // Create tensors
    CLTensor src = create_tensor<CLTensor>(shape, data_type_from_format(format));
    src.info()->set_format(format);

    ARM_COMPUTE_EXPECT(src.info()->is_resizable(), framework::LogLevel::ERRORS);

    CLKeyPointArray corners;
    unsigned int    num_corners;

    // Create and configure function
    CLFastCorners fast_corners;
    fast_corners.configure(&src, threshold, suppress_nonmax, &corners, &num_corners, border_mode, constant_border_value);

    // Validate padding
    PaddingCalculator calculator(shape.x(), 1); // elems_processed

    calculator.set_border_size(bresenham_radius);
    calculator.set_access_offset(-bresenham_radius);
    calculator.set_accessed_elements(7); // elems_read

    validate(src.info()->padding(), calculator.required_padding());
}

template <typename T>
using CLFastCornersFixture = FastCornersValidationFixture<CLTensor, CLAccessor, CLKeyPointArray, CLFastCorners, T>;

FIXTURE_DATA_TEST_CASE(RunSmall, CLFastCornersFixture<uint8_t>, framework::DatasetMode::PRECOMMIT, combine(combine(combine(datasets::Small2DShapes(), framework::dataset::make("Format", Format::U8)),
                                                                                                                   framework::dataset::make("SuppressNonMax", { false, true })),
                                                                                                           framework::dataset::make("BorderMode", BorderMode::UNDEFINED)))
{
    // Validate output
    CLArrayAccessor<KeyPoint> array(_target);
    fast_validate_keypoints(array.buffer(), array.buffer() + array.num_values(), _reference.begin(), _reference.end(), tolerance, allowed_missing, allowed_mismatching);
}
FIXTURE_DATA_TEST_CASE(RunLarge, CLFastCornersFixture<uint8_t>, framework::DatasetMode::NIGHTLY, combine(combine(combine(datasets::Large2DShapes(), framework::dataset::make("Format", Format::U8)),
                                                                                                                 framework::dataset::make("SuppressNonMax", { false, true })),
                                                                                                         framework::dataset::make("BorderMode", BorderMode::UNDEFINED)))
{
    // Validate output
    CLArrayAccessor<KeyPoint> array(_target);
    fast_validate_keypoints(array.buffer(), array.buffer() + array.num_values(), _reference.begin(), _reference.end(), tolerance, allowed_missing, allowed_mismatching);
}

TEST_SUITE_END()
TEST_SUITE_END()
} // namespace validation
} // namespace test
} // namespace arm_compute
