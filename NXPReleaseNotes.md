# ARM Compute Library NXP Release Notes

## Overview
The latest release notes are for the ARM Compute Library (ACL) release 20.08. The purpose of this release is to adapt the [original ACL 20.08](https://github.com/ARM-software/ComputeLibrary) and optimize it for the NXP i.MX8 series/Yocto Linux. Only NEON/CPU backend is supported, OpenCL backend is not tested, but it can be used experimentally. To take advantage of GPU and NPU acceleration on NXP i.MX8, we strongly suggest using [Arm NN with the VSI NPU backend](https://source.codeaurora.org/external/imx/armnn-imx).

## How to cross-compile for Yocto Linux and execute ACL mannualy
- Clone the [NXP repository](https://source.codeaurora.org/external/imx/arm-computelibrary-imx)
```
git clone https://source.codeaurora.org/external/imx/arm-computelibrary-imx -b imx_20.08
```

- Install scons on the target using `pip install scons`
- Setup the cross-compile enviornment. Navigate to the ACL directory and compile it with the following command or similar (see [documentation](https://arm-software.github.io/ComputeLibrary/v20.08/) for additional parameters):
```
source <path_to_yocto_sdk>/environment-setup-aarch64-poky-linux
cd <path_to_acl_repo>
scons os=linux neon=1 opencl=0 embed_kernels=1 gles_compute=0 arch=arm64-v8a build=cross_compile toolchain_prefix=' ' extra_cxx_flags='-fPIC' -j32
```
- To build all the tests for testing use the following command instead:
```
scons os=linux neon=1 opencl=0 embed_kernels=1 gles_compute=0 arch=arm64-v8a build=cross_compile toolchain_prefix=' ' extra_cxx_flags='-fPIC' benchmark_examples=1 validate_examples=1 validation_tests=1 benchmark_tests=1 -j32
```
- Optionally you may also use a custom build directory using `build_dir=<build_directory>`
- Copy the build directory to your i.MX8 series board
- Add the build libraries to `LD_LIBRARY_PATH` to link dynamically:
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<path_to_the_build>
```
- Now you may run your program or some of the examples and it will link the dynamic libraries. You may also use the static libraries instead for a statically linked application.

### 20.08 (Not yet released!)
- Fixed O3 build break. Switched back from O1 to O3 optimizations.
- Removed forced "-fPIC" in builds. Should be added using extra_cxx_flags='-fPIC'.
- For a complete list of changes see documentation for [20.08](https://arm-software.github.io/ComputeLibrary/v20.08/) and [20.05](https://arm-software.github.io/ComputeLibrary/v20.05/).

###### Validation tests on Linux imx8mpevk 5.4.47-2.2.0+Galcore version 6.4.3.p0.286725
```
./arm_compute_validation --filter='NEON/*' --log-level=ERRORS
Executed 19415 test(s) (19288 passed, 0 expected failures, 0 failed, 0 crashed, 0 disabled) in 611 second(s)
./arm_compute_validation --filter='CPP/*' --log-level=ERRORS
Executed 784 test(s) (784 passed, 0 expected failures, 0 failed, 0 crashed, 0 disabled) in 16 second(s)
```

### 20.02.01
- For a complete list of changes see documentation for [20.02.01](https://arm-software.github.io/ComputeLibrary/v20.02.1/) and [20.02](https://arm-software.github.io/ComputeLibrary/v20.02/)
- Removed "imx8" scons compile flag
- Switched from O3 to O1 optimization due to compile break

##### Validation tests on Linux imx8mpevk 5.4.47-2.2.0+Galcore version 6.4.3.p0.283965
```
./arm_compute_validation --filter='NEON/*' --log-level=ERRORS
Executed 16998 test(s) (16871 passed, 0 expected failures, 0 failed, 0 crashed, 0 disabled) in 786 second(s)
./arm_compute_validation --filter='CPP/*' --log-level=ERRORS
Executed 710 test(s) (710 passed, 0 expected failures, 0 failed, 0 crashed, 0 disabled) in 16 second(s)
```

### 19.08.01
- Fixes in Accumulate and PriorBoxLayer tests
- "-fPIC" added automatically to all builds
- Fixed GCC 9.2 compilation errors
- For a complete list of changes see documentation for [19.08.01](https://arm-software.github.io/ComputeLibrary/v19.08.1/) and [19.08](https://arm-software.github.io/ComputeLibrary/v19.08/)

##### Validation tests on Linux imx8mpevk 5.4.24-2.2.0+Galcore version 6.4.0.234062
```
./arm_compute_validation --filter='NEON/*' --log-level=ERRORS
Executed 14231 test(s) (14104 passed, 0 expected failures, 0 failed, 0 crashed, 0 disabled) in 432 second(s)
```

### 19.05
- For a complete list of changes see documentation for [19.05](https://arm-software.github.io/ComputeLibrary/v19.05/)

##### Validation tests on Linux imx8qmmek 4.14.98_2.0.0+Galcore version 6.2.4.190076
```
./arm_compute_validation --filter='NEON/*' --log-level=ERRORS
Executed 13313 test(s) (13313 passed, 0 expected failures, 0 failed, 0 crashed, 0 disabled) in 432 second(s)
./arm_compute_validation --filter='(CL/(?!.*(DataType=F16|Type=F16|Histogram|EqualizeHistogram)).*(DataType=F32|Type=F32|FP32|DataType=U8|Type=U8|QASYMM8)).*' --log-level=ERRORS
Executed 8787 test(s) (8686 passed, 0 expected failures, 101 failed, 0 crashed, 0 disabled) in 432 second(s)
```