# ARM Compute Library 19.08.01 Release Notes

## Notes
Only NEON as the preferred CPU backend is being validated. CL backend is not supported due to a significant number of issues. In order to leverage GPU/NPU acceleration please use the VSI NPU backend either through [Arm NN](https://source.codeaurora.org/external/imx/armnn-imx) or other frameworks.

## How to compile and execute ACL mannualy
- Clone the [NXP repository](https://source.codeaurora.org/external/imx/arm-computelibrary-imx)
- Install scons on the target using `pip install scons`
- Compile ACL with the following command or similar (see [documentation](https://arm-software.github.io/ComputeLibrary/v19.08.1/) for additional parameters):
```
scons Werror=0 -j6 neon=1 opencl=0 os=linux arch=arm64-v8a build=native examples=1 embed_kernels=1 cppthreads=1 mali=0 gles_compute=0 benchmark=1 benchmark_tests=1
```
- Export the path to ACL libraries (or similar):
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/arm-computelibrary-imx/build
```

## Description
### This release of ACL supports this platform:
- Linux imx8mpevk 5.4.24-2.2.0+g3cc0f1037ce1 #1 SMP PREEMPT Thu Jun 11 18:31:58 UTC 2020 aarch64 aarch64 aarch64 GNU/Linux
- Galcore version 6.4.0.234062

## Results of validation tests
### NEON (CPU) tests
```
Version = arm_compute_version=v19.08.1 Build options: {'embed_kernels': '1', 'gles_compute': '0', 'arch': 'arm64-v8a', 'opencl': '1', 'neon': '1', 'build': 'cross_compile', 'os': 'linux', 'Werror': '0', 'examples': '1'} Git hash=c657c3dbf808121bb5c5e8418a82c7c5a17d4853
CommandLine = ./arm_compute_validation --filter=NEON/* --log-level=ERRORS 
Executed 14231 test(s) (14104 passed, 0 expected failures, 0 failed, 0 crashed, 0 disabled) in 432 second(s)
```

## Changelog
### 19.08.01
- Fixes in Accumulate and PriorBoxLayer tests
- "-fPIC" added automatically to all builds
- Fixed GCC 9.2 compilation errors
