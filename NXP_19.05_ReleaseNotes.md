# ARM Compute Library 19.05 NXP Beta Release Notes

## Overview
These release notes are for the ARM Compute Library (ACL) beta release 19.05
The purpose of this release is adapting the original ACL 19.05 for NXP HW. Mainly to adapt OpenCL kernels for GC7000 GPU. GC7000 GPU doesn't support half precision therefore only F32 and QASYMM8 kernels were tested for GPU version.


## Features
- Partly tested beta release (100% of validation tests for CPU and NEON passed, 99% of validation tests for F32 data type for GPU passed)
- Coverity and BlackDuck scan passed
- SCR, PLR, EULA, Copyright and License review passed
- Field trial tests passed

## Description
### This release of ACL supports this platform:

- Linux imx8qmmek 4.14.98-imx_4.14.98_2.0.0_ga+g1175b5961153 #1 SMP PREEMPT Fri May 3 13:21:25 UTC 2019 aarch64 aarch64 aarch64 GNU/Linux with OpenCL 1.2
- Galcore version 6.2.4.190076
### It will contain these libraries after the building:
- libarm_compute.so 
- libarm_compute_core.so
- libarm_compute_graph.so

## How to compile ACL mannualy
- Download ACL 19.05 repo from NXP webpage
- Install scons on the target
- Compile ACL with the command
scons Werror=0 -j6 neon=1 opencl=1 os=linux arch=arm64-v8a build=native examples=1 validation_tests=1 embed_kernels=1 cppthreads=1 mali=0 gles_compute=0
- Add the path to ACL libraries to /etc/ld.so.conf and apply the changes with ldconfig command

## Results of validation tests
### CPU tests
- 100% of CPP and NEON validation tests passed: 13313 (passed: 13313, failed: 0)

### GPU tests for F32 data type
- Filter: (CL/(?!.*(DataType=F16|Type=F16|Histogram|EqualizeHistogram)).*(DataType=F32|Type=F32|FP32|DataType=U8|Type=U8|QASYMM8)).*
- Results: 8787 (passed: 8686, failed: 101)

- Failed tests:
CL/Col2Im - total: 20, success: 11, failed: 9
CL/GEMMConvolutionLayer - total: 288, success: 264, failed: 24
CL/DeconvolutionLayer - total: 64, success: 57, failed: 7
CL/GEMMDilatedConvolutionLayer - total: 25, success: 23, failed: 2
CL/FFT1D - total: 31, success: 29, failed: 2
CL/FFT2D - total: 22, success: 20, failed: 2
CL/NonLinearFilter - total: 270, success: 225, failed: 45
CL/PriorBoxLayer - total: 4, success: 2, failed: 2
CL/ReduceMean - total: 96, success: 91, failed: 5
CL/SoftmaxLayer - total: 15, success: 12, failed: 3

- Halted tests:
Histogram and EqualizeHistogram

- Passed tests:
CL/AbsoluteDifference - total: 20, success: 20, failed: 0
CL/Accumulate - total: 10, success: 10, failed: 0
CL/AccumulateWeighted - total: 10, success: 10, failed: 0
CL/AccumulateSquared - total: 10, success: 10, failed: 0
CL/ActivationLayer - total: 315, success: 315, failed: 0
CL/ArgMinMax - total: 41, success: 41, failed: 0
CL/ArithmeticAddition - total: 77, success: 77, failed: 0
CL/ArithmeticDivision - total: 20, success: 20, failed: 0
CL/ArithmeticSubtraction - total: 77, success: 77, failed: 0
CL/BatchNormalizationLayer - total: 101, success: 101, failed: 0
CL/BatchNormalizationLayerFusion - total: 98, success: 98, failed: 0
CL/BatchToSpaceLayer - total: 23, success: 23, failed: 0
CL/BitwiseAnd - total: 10, success: 10, failed: 0
CL/BitwiseNot - total: 10, success: 10, failed: 0
CL/BitwiseOr - total: 10, success: 10, failed: 0
CL/BitwiseXor - total: 10, success: 10, failed: 0
CL/BBoxTransform - total: 46, success: 46, failed: 0
CL/Box3x3 - total: 30, success: 30, failed: 0
CL/Cast - total: 220, success: 220, failed: 0
CL/ChannelShuffle - total: 30, success: 30, failed: 0
CL/ColorConvert - total: 6, success: 6, failed: 0
CL/Comparison - total: 75, success: 75, failed: 0
CL/ConvertFullyConnectedWeights - total: 27, success: 27, failed: 0
CL/ConvolutionLayer - total: 7, success: 7, failed: 0
CL/GroupedGEMMConvolutionLayer - total: 90, success: 90, failed: 0
CL/Copy - total: 14, success: 14, failed: 0
CL/CropResize - total: 23, success: 23, failed: 0
CL/DepthConcatenateLayer - total: 15, success: 15, failed: 0
CL/DepthConvertLayer - total: 155, success: 155, failed: 0
CL/DepthwiseConvolutionLayer - total: 526, success: 526, failed: 0
CL/DequantizationLayer - total: 15, success: 15, failed: 0
CL/Dilate - total: 30, success: 30, failed: 0
CL/DilatedConvolutionLayer - total: 4, success: 4, failed: 0
CL/DirectConvolutionLayer - total: 20, success: 20, failed: 0
CL/ElementwiseMax - total: 46, success: 46, failed: 0
CL/ElementwiseMin - total: 46, success: 46, failed: 0
CL/ElementwiseSquaredDiff - total: 46, success: 46, failed: 0
CL/Erode - total: 30, success: 30, failed: 0
CL/ExpLayer - total: 5, success: 5, failed: 0
CL/FFTConvolutionLayer - total: 4, success: 4, failed: 0
CL/FillBorder - total: 270, success: 270, failed: 0
CL/FlattenLayer - total: 8, success: 8, failed: 0
CL/Floor - total: 9, success: 9, failed: 0
CL/FullyConnectedLayer - total: 148, success: 148, failed: 0
CL/GEMM - total: 120, success: 120, failed: 0
CL/GEMMMatrixMultiplyReshaped - total: 576, success: 576, failed: 0
CL/GEMMMatrixMultiplyReshapedOnlyRHS - total: 288, success: 288, failed: 0
CL/GEMMReshapeLHSMatrix - total: 576, success: 576, failed: 0
CL/GEMMReshapeRHSMatrix - total: 1320, success: 1320, failed: 0
CL/Gather - total: 51, success: 51, failed: 0
CL/Gaussian3x3 - total: 30, success: 30, failed: 0
CL/Gaussian5x5 - total: 30, success: 30, failed: 0
CL/GenerateProposals - total: 16, success: 16, failed: 0
CL/GlobalPoolingLayer - total: 30, success: 30, failed: 0
CL/HeightConcatenateLayer - total: 10, success: 10, failed: 0
CL/Im2Col - total: 66, success: 66, failed: 0
CL/IntegralImage - total: 10, success: 10, failed: 0
CL/L2NormalizeLayer - total: 30, success: 30, failed: 0
CL/LSTMLayer - total: 20, success: 20, failed: 0
CL/LocallyConnected - total: 20, success: 20, failed: 0
CL/MeanStdDev - total: 12, success: 12, failed: 0
CL/Median3x3 - total: 30, success: 30, failed: 0
CL/MinMaxLocation - total: 12, success: 12, failed: 0
CL/NormalizationLayer - total: 545, success: 545, failed: 0
CL/NormalizePlanarYUVLayer - total: 26, success: 26, failed: 0
CL/PadLayer - total: 125, success: 125, failed: 0
CL/Permute - total: 299, success: 299, failed: 0
CL/PixelWiseMultiplication - total: 16, success: 16, failed: 0
CL/PoolingLayer - total: 290, success: 290, failed: 0
CL/QuantizationLayer - total: 20, success: 20, failed: 0
CL/RNNLayer - total: 8, success: 8, failed: 0
CL/RoiAlign - total: 37, success: 37, failed: 0
CL/Range - total: 56, success: 56, failed: 0
CL/ReductionOperation - total: 44, success: 44, failed: 0
CL/Remap - total: 40, success: 40, failed: 0
CL/ReorgLayer - total: 23, success: 23, failed: 0
CL/ReshapeLayer - total: 24, success: 24, failed: 0
CL/Reverse - total: 26, success: 26, failed: 0
CL/RsqrtLayer - total: 7, success: 7, failed: 0
CL/Scale - total: 268, success: 268, failed: 0
CL/Select - total: 46, success: 46, failed: 0
CL/Slice - total: 18, success: 18, failed: 0