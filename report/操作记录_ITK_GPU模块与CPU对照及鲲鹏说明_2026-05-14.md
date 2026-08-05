# ITK 5.4 GPU 相关模块排查：鲲鹏 aarch64、OpenCL/CUDA 与 CPU 对照

**整理日期：2026-05-14**  
**源码与构建参考：** 远端 **`/home/pub/yyq/ITK-5.4.0`（ITK 5.4.0）**、**`build-yyq`** 的 **`CMakeCache.txt`**  

本文档归档对 **ITK 中「GPU 路线」模块** 在 **鲲鹏 Linux aarch64** 上的技术结论：**能否编译运行默认 ITK**、**GPU 与 OpenCL/CUDA 的关系**、**是否存在「对应的 C++ CPU 类」**，以及 **在当前未启用 GPU 栈时视为「尚未在本环境落地」的模块清单**。

**相关文档：** [`操作记录_ITK_NEON向量化安全编译_2026-05-13.md`](./操作记录_ITK_NEON向量化安全编译_2026-05-13.md)（CPU/NEON 编译）、[`操作记录_ITK编译与单节点并行绑核_2026-05-07.md`](./操作记录_ITK编译与单节点并行绑核_2026-05-07.md)（`build-yyq` 全流程）。

---

## 1. ITK 5.4 中「GPU」两条主线 + 一条 FFT 远程线

| 路线 | 在源码中的位置 | 依赖 | 典型说明 |
|------|----------------|------|----------|
| **OpenCL GPU（`ITKGPU*`）** | `Modules/Core/GPUCommon`、`Modules/Core/GPUFiniteDifference`、`Modules/Filtering/GPU*`、`Modules/Registration/GPU*` 等 | **OpenCL**；**`GPUCommon/CMakeLists.txt`** 中在 **`ITK_USE_GPU=ON`** 时走 **`itkOpenCL`**、链接 **`OpenCL_LIBRARIES`** | 类名多为 **`itkGPU*`**，与 **CPU 侧 `itk::*`** 并存，**非 CUDA** |
| **CUDA（Remote）** | **`Modules/Remote/CudaCommon.remote.cmake`**，拉取 **https://github.com/RTKConsortium/ITKCudaCommon.git** | **NVIDIA CUDA Toolkit（`nvcc` 等）** | **`Module_CudaCommon`**；面向 **CUDA 生态**（如 RTK 等扩展），**不是**「某一个滤波器」的单一类 |
| **VkFFT 多后端（Remote）** | **`Modules/Remote/VkFFTBackend.remote.cmake`**，拉取 **ITKVkFFTBackend** | **VkFFT**；说明中含 **Vulkan/CUDA/HIP/OpenCl** 等兼容表述 | **GPU 加速 FFT 后端**；与 CPU 侧其它 FFT 实现可并存，取决于 CMake 是否启用 |

**结论：** 主干里的 **`ITKGPU*`** 主要是 **OpenCL**；**CUDA** 以 **Remote `CudaCommon`** 形式单独存在；勿把 **`itkGPU*`** 与 **CUDA** 混为一谈。

---

## 2. 当前 `build-yyq` 在鲲鹏上的状态（与 GPU 的关系）

- **`ITK_BUILD_DEFAULT_MODULES:BOOL=ON`**：**默认核心与大量常规模块** 可在 **aarch64** 上配置、编译、运行（已在实际环境中验证）。  
- **`Module_ITKGPU*`、`Module_CudaCommon`** 等在本次 **`CMakeCache`** 中为 **OFF / 未启用**：表示 **未选建 GPU/CUDA 路线**，**不是**「ARM 版 ITK 缺文件未移植」。  
- 在 **无 OpenCL 设备/驱动、无 CUDA** 的 **纯鲲鹏 CPU 节点** 上，即使强行打开部分选项，**若没有满足依赖，配置或链接仍会失败**——属于 **环境约束**，而非 **ITK 主线在鲲鹏上不可编**。

---

## 3. GPU 模块是否有「对应的可运行 C++（CPU）版本」？

**有。** CPU 侧 **不是** 再复制一套 `itkGPU*`，而是 **ITK 其它模块里已有的 `itk::*` 滤波器/配准类**。  
`ITKGPU*` 的 **`itk-module.cmake`** 中普遍 **`DEPENDS` / `COMPILE_DEPENDS`** 已挂上 **CPU 模块**（如 **`ITKSmoothing`**、**`ITKPDEDeformableRegistration`**、**`ITKFiniteDifference`**），语义上即 **同一算法族在 CPU 上的实现**。

### 3.1 常见对照表（工程选型用）

| GPU 模块（CMake 名） | 典型 GPU 类（`itkGPU*`） | 常见 CPU 侧对应（算法族） |
|----------------------|--------------------------|---------------------------|
| **ITKGPUSmoothing** | `itkGPUDiscreteGaussianImageFilter`、`itkGPUMeanImageFilter` | `itk::DiscreteGaussianImageFilter`、`itk::SmoothingRecursiveGaussianImageFilter`、`itk::MeanImageFilter` 等（**`ITKSmoothing`**） |
| **ITKGPUThresholding** | `itkGPUBinaryThresholdImageFilter` | `itk::BinaryThresholdImageFilter`（**`ITKThresholding`**） |
| **ITKGPUAnisotropicSmoothing** | `itkGPUGradientAnisotropicDiffusionImageFilter` 等 | `itk::GradientAnisotropicDiffusionImageFilter` 等（**`ITKAnisotropicSmoothing`**） |
| **ITKGPUFiniteDifference** | `itkGPUDenseFiniteDifferenceImageFilter` 等 | `itk::DenseFiniteDifferenceImageFilter` 等（**`ITKFiniteDifference`**） |
| **ITKGPUImageFilterBase** | `itkGPUCastImageFilter`、`itkGPUBoxImageFilter`、`itkGPUNeighborhoodOperatorImageFilter` | `itk::CastImageFilter`、盒式/邻域类滤波器等 |
| **ITKGPURegistrationCommon** | GPU 配准公共组件 | 与 **CPU 配准框架**共用设计；具体类分布在 **Registration** 各 CPU 模块 |
| **ITKGPUPDEDeformableRegistration** | `itkGPUDemonsRegistrationFilter`、`itkGPUPDEDeformableRegistrationFilter` | `itk::DemonsRegistrationFilter`、`itk::PDEDeformableRegistrationFilter` 等（**`ITKPDEDeformableRegistration`** 等） |
| **ITKGPUCommon** | `itkGPUImage`、OpenCL 上下文与 kernel 管理 | **无单一「GPUCommon 的 CPU 同名类」**；CPU 路径直接使用 **`itk::Image`** + **`itk::ImageToImageFilter`** 等常规管线 |

### 3.2 `CudaCommon` 与 `VkFFTBackend`

- **`CudaCommon`（Remote）**：**CUDA 基础设施**，供 **CUDA 系扩展** 使用；**不是**「某个滤波器的 GPU 版」的 1:1 对象。**CPU 图像处理不依赖该 Remote。**  
- **`VkFFTBackend`**：**FFT 的 GPU/多后端加速**；CPU 侧 ITK 仍有 **不依赖 VkFFT 的 FFT 相关路径**（具体取决于 **`ITKFFT`** 等模块是否打开），属于 **后端可选**，不是「没有 C++」。

---

## 4. 「尚未在本环境落地」的 GPU 相关清单（默认 OFF / 未 fetch）

以下含义：**在你们当前「未启用 OpenCL GPU、未启用 CUDA」的鲲鹏构建语义下，尚未作为 GPU 实现参与构建与运行**；其中 **1～8 的算法在 CPU 上已有 §3.1 所列替代类**。

**OpenCL 路线（主干源码，默认未编）：**

1. `ITKGPUCommon`  
2. `ITKGPUFiniteDifference`  
3. `ITKGPUImageFilterBase`  
4. `ITKGPUSmoothing`  
5. `ITKGPUThresholding`  
6. `ITKGPUAnisotropicSmoothing`  
7. `ITKGPURegistrationCommon`  
8. `ITKGPUPDEDeformableRegistration`  

**CUDA 路线（Remote，未 fetch）：**

9. **`CudaCommon`**（对应 **`ITKCudaCommon`** 仓库）

**多后端 FFT 加速（Remote）：**

10. **`VkFFTBackend`**（对应 **`ITKVkFFTBackend`** 仓库）

**说明：** 在 **`Modules/Remote/*.remote.cmake`** 中检索 **`cuda`** 关键字，与 **CUDA 直接相关** 的 fetch 描述主要为 **`CudaCommon`** 与 **`VkFFTBackend`** 的说明文字；**主干 `ITKGPU*` 源码树内对 `cuda` 字符串的依赖可忽略为「无」**（以 **OpenCL** 为主）。

---

## 5. 若要在鲲鹏上「跑起来」GPU 路线，需要什么

| 目标 | 前置条件（摘要） |
|------|------------------|
| **跑 `ITKGPU*`** | **OpenCL 运行时** + 能暴露 OpenCL 的 **驱动/GPU**（或 CPU OpenCL 实现，性能另论）；CMake 中 **`ITK_USE_GPU=ON`** 并打开对应 **`Module_ITKGPU*`** |
| **跑 `CudaCommon` / CUDA 滤波器** | **NVIDIA CUDA 工具链** + **GPU**；**纯鲲鹏 CPU 无卡** 场景通常 **不适用** |
| **跑 `VkFFTBackend`** | 按 Remote 说明准备 **Vulkan/CUDA/HIP/OpenCL** 等之一及 **VkFFT** 依赖 |

---

## 6. 与「移植」表述的关系

- **ITK 官方**：维护 **跨平台 C++ 核心** + 可选 **OpenCL（`ITKGPU*`）**、可选 **Remote（CUDA / VkFFT）**。  
- **在鲲鹏 CPU 上跑同一算法**：多数情况应 **直接使用 §3.1 的 CPU 类**，而不是等待「`itkGPU*` 的 ARM 移植版」。  
- **若业务强依赖 CUDA 源码路径**：属于 **换执行后端（重写或换硬件）** 的工程，**不等价于**「ITK ARM 包缺模块」。

---

*本文档根据对远端 ITK 5.4.0 源码目录与 `build-yyq/CMakeCache.txt` 的排查结果整理，便于与 NEON/编译主文档交叉查阅。*
