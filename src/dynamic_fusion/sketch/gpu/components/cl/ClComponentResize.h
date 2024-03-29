/*
 * Copyright (c) 2022-2023 Arm Limited.
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

#ifndef SRC_DYNAMIC_FUSION_SKETCH_GPU_COMPONENTS_CL_CLCOMPONENTRESIZE
#define SRC_DYNAMIC_FUSION_SKETCH_GPU_COMPONENTS_CL_CLCOMPONENTRESIZE

#include "arm_compute/dynamic_fusion/sketch/attributes/ResizeAttributes.h"

#include "src/dynamic_fusion/sketch/gpu/components/IGpuKernelComponent.h"

namespace arm_compute
{
/** Forward declaration */
class ITensorInfo;
namespace experimental
{
namespace dynamic_fusion
{
/** Forward declaration */
template <typename T>
class ArgumentPack;

/** Forward declaration */
#ifndef ACL_INTERNAL_TEST_CKW_IN_DF
class ClTemplateResize;
#else  // ACL_INTERNAL_TEST_CKW_IN_DF
class GpuCkwResize;
#endif // ACL_INTERNAL_TEST_CKW_IN_DF

class ClComponentResize final : public IGpuKernelComponent
{
public:
    /** Attributes are a set of backend-agnostic parameters that define what a component does */
    using Attributes = ResizeAttributes;

    /** Validate the component
     *
     * @param[in]     properties Component properties @ref Properties
     * @param[in,out] tensors    Tensor arguments to the component
     * @param[in]     attributes Component attributes @ref Attributes
     *
     * @return Status        Validation results
     *
     * Tensor argument names:
     * - ACL_SRC_0: Input
     * - ACL_DST_0: Output
     *
     * Tensor argument constness:
     * - ACL_SRC_0: Const
     * - ACL_DST_0: Const
     *
     * Valid data layouts:
     * - NHWC
     *
     ** Valid data type configurations:
     * |ACL_SRC_0      |ACL_DST_0      |
     * |:--------------|:--------------|
     * |QASYMM8        |QASYMM8        |
     * |QASYMM8_SIGNED |QASYMM8_SIGNED |
     * |F16            |F16            |
     * |F32            |F32            |
     * |U8             |U8             |
     * |S16            |S16            |
     */
    static Status
    validate(const Properties &properties, const ArgumentPack<ITensorInfo> &tensors, const Attributes &attributes);

    /** Constructor
     *
     * Similar to @ref ClComponentResize::validate()
     */
    ClComponentResize(ComponentId                      id,
                      const Properties                &properties,
                      const ArgumentPack<ITensorInfo> &tensors,
                      const Attributes                &attributes);

    /** Destructor */
    ~ClComponentResize() override;

    /** Prevent instances of this class from being copy constructed */
    ClComponentResize(const ClComponentResize &component) = delete;

    /** Prevent instances of this class from being copied */
    ClComponentResize &operator=(const ClComponentResize &component) = delete;

    /** Allow instances of this class to be move constructed */
    ClComponentResize(ClComponentResize &&component) = default;

    /** Allow instances of this class to be moved */
    ClComponentResize &operator=(ClComponentResize &&component) = default;

    /** Get writer for the component */
#ifndef ACL_INTERNAL_TEST_CKW_IN_DF
    const IGpuTemplateComponentWriter *template_writer() const override;
#else  // ACL_INTERNAL_TEST_CKW_IN_DF
    const IGpuCkwComponentDriver *ckw_component_driver() const override;
#endif // ACL_INTERNAL_TEST_CKW_IN_DF

    /** Get component type */
    GpuComponentType type() const override
    {
        return GpuComponentType::Complex;
    }

private:
#ifndef ACL_INTERNAL_TEST_CKW_IN_DF
    std::unique_ptr<ClTemplateResize> _component_writer;
#else  // ACL_INTERNAL_TEST_CKW_IN_DF
    std::unique_ptr<GpuCkwResize> _component_writer;
#endif // ACL_INTERNAL_TEST_CKW_IN_DF
};

} // namespace dynamic_fusion
} // namespace experimental
} // namespace arm_compute

#endif /* SRC_DYNAMIC_FUSION_SKETCH_GPU_COMPONENTS_CL_CLCOMPONENTRESIZE */
