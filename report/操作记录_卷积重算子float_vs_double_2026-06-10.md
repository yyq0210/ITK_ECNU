# 卷积类重算子 float vs double — 精度 + 速度实验报告

**日期：** 2026-06-10  
**远端：** `root@202.120.87.20`，96 线程，`taskset -c 0-95`  
**程序：** `precision_conv_bench`  
**原始日志：** `/tmp/precision_conv_2026-06-10_225356.log`

**对比方式（与滤波实验一致）：**

- **full_float：** `Image<float>` 输入 → Filter\<float\>
- **full_double：** `Image<float>` 输入 → Cast→double → Filter\<double\>
- **加速比：** ms_double / ms_float（>1 表示 float 更快）

---

## 1. 测试算子

| 算子 | 类型 | 参数 | 计算特征 |
|------|------|------|---------|
| **BilateralImageFilter** | 非线性邻域 | domainσ=4, rangeσ=50 | 空间+值域双高斯，最重 |
| **DiscreteGaussianImageFilter** | 可分离离散卷积 | σ=4 (variance=16) | 大核高斯 |
| **MeanImageFilter** | 盒式卷积 | radius=15 (31×31) | 纯邻域求和 |
| **ConvolutionImageFilter** | 空间域显式卷积 | 31×31 高斯核 | O(N·K²)，仅 ≤256² 测 |
| **FFTConvolutionImageFilter** | FFT 频域卷积 | 31×31 高斯核 | O(N log N)，大图主路径 |

---

## 2. 结果汇总

### 2.1 214×256（真实 MRI 放大图）

| 算子 | ms_float | ms_double | 加速比 | max_abs | rmse |
|------|----------|-----------|--------|---------|------|
| Bilateral | 65.2 | 62.3 | **0.96×** | 7.6e-6 | 3.6e-6 |
| DiscreteGaussian | 3.6 | 4.7 | **1.33×** | 1.1e-5 | 4.0e-6 |
| Mean(r=15) | 176.4 | 172.0 | **0.97×** | 7.6e-6 | 3.0e-6 |
| ConvolutionSpatial | 81.0 | 77.9 | **0.96×** | 7.6e-6 | 4.0e-6 |
| FFTConvolution | 18.2 | 18.3 | **1.01×** | 7.6e-6 | 4.0e-6 |

### 2.2 1024×1024

| 算子 | ms_float | ms_double | 加速比 | max_abs | rmse |
|------|----------|-----------|--------|---------|------|
| Bilateral | 1108 | 1083 | **0.98×** | 7.6e-6 | 3.6e-6 |
| DiscreteGaussian | 55.5 | 54.6 | **0.98×** | 1.2e-5 | 4.0e-6 |
| Mean(r=15) | 3384 | 3296 | **0.97×** | 7.6e-6 | 4.0e-6 |
| FFTConvolution | 236 | 241 | **1.02×** | 7.6e-6 | 4.0e-6 |

### 2.3 2048×2048

| 算子 | ms_float | ms_double | 加速比 | max_abs | rmse |
|------|----------|-----------|--------|---------|------|
| Bilateral | 3869 | 3781 | **0.98×** | 7.6e-6 | 3.6e-6 |
| DiscreteGaussian | 189 | 184 | **0.97×** | 1.3e-5 | 4.0e-6 |
| Mean(r=15) | 13527 | 13180 | **0.97×** | 7.6e-6 | 4.0e-6 |
| FFTConvolution | 1175 | 1175 | **1.00×** | 7.6e-6 | 4.0e-6 |

---

## 3. 精度结论

所有卷积类算子在 float vs double 之间：

- **max_abs：** 约 **7.6×10⁻⁶ ~ 1.3×10⁻⁵**（相对像素值 0–255 可忽略）
- **max_rel：** ≈ **0**（报告精度内）
- **可视化 / 下游分割：** float 与 double **等价**
- **若需 bit-identical 或科学级复现：** 保留 double

---

## 4. 速度结论

### 4.1 大图（1024²、2048²）— 重计算主场景

| 观察 | 说明 |
|------|------|
| **float 未显著快于 double** | 加速比 **0.97–1.02×**，基本持平或 float 略慢 |
| **Bilateral 最重** | 2048² 单次 ~3.8 s（96 线程），float/double 差距 <3% |
| **Mean 盒式卷积** | 2048² 单次 ~13 s，float 略慢 ~2.6% |
| **FFT 卷积** | 2048² ~1.2 s，float/double **完全相同** |

**原因分析（概要）：** 见 **§8 向量化与 RealType 验证**——ITK 已开 NEON，但多数 Filter 在 `Image<float>` 下仍用 **double RealType** 做核心运算，float 主要省存储而非算力。

### 4.2 小图（214×256）

- **DiscreteGaussian：** float **1.33×** 更快（唯一明显收益）
- 其余算子：float **0.96–1.01×**，基本持平

### 4.3 与 RecursiveGaussian 对比

[`操作记录_混合精度float化实验报告_2026-06-10.md`](./操作记录_混合精度float化实验报告_2026-06-10.md) 中 RecursiveGaussian 在小图有 **~2×** 加速，是因为 double 路径 **Cast 占比高** 且递归高斯实现路径不同。**卷积类重算子在大图上不呈现相同模式。**

---

## 5. 实践建议

| 目标 | 建议 |
|------|------|
| **省内存 / 带宽** | 卷积 pipeline 用 **`Image<float>`**（内存减半） |
| **追求大图加速** | **不要指望** float 替代 double 带来显著提速；优先 **线程数 / 算法选择**（如 FFT vs 空间卷积） |
| **Bilateral 压测** | 官方 Example 已是 float；保持即可 |
| **大核卷积** | 1024²+ 用 **FFTConvolution**；2048² 上 FFT ~1.2 s vs Mean ~13 s |
| **精度** | float **足够**；差异在 10⁻⁵ 量级 |

---

## 8. 向量化与 RealType 验证（为何 float 未更快）

**问题：** float 像素更小，理论上 DRAM/L3 吞吐量应更高；为何卷积类重算子 float 与 double 速度几乎相同？

**结论先行：** **NEON 向量化在 ITK 库上是开启的**；float 未显著更快，主因是 **ITK 在 `Image<float>` 下仍用 `NumericTraits::RealType = double` 做邻域/累加运算**，并未走「全程 float SIMD（4×f32）」路径。

### 8.1 编译与向量化：ITK 已开 NEON，benchmark 程序未加 `-march`

| 组件 | `CMAKE_CXX_FLAGS` / 实际标志 | 说明 |
|------|------------------------------|------|
| **ITK `build-yyq`** | `-march=armv8.2-a+crypto` + `-O3` | 鲲鹏安全方案，见 [`操作记录_ITK_NEON向量化安全编译_2026-05-13.md`](./操作记录_ITK_NEON向量化安全编译_2026-05-13.md) |
| **`precision_conv_bench`** | 空（仅 Release 默认 `-O3`） | 热路径在 **链接的 ITK 静态库** 内，对 Bilateral/卷积耗时影响可忽略 |
| **`-march=native` / `kunpeng-simd.sh`** | **未用于 ITK** | 旧 `as` 2.27 无法汇编 `dotprod+fp16fml` |
| **VNL `VNL_CONFIG_ENABLE_SSE2_ROUNDING`** | **OFF** | x86 SSE2 选项；ARM 上无意义 |
| **ITK GPU/OpenCL** | **未启用** | — |

**ITK ImageFeature 模块 `flags.make`（节选）：**

```text
CXX_FLAGS = -march=armv8.2-a+crypto ... -O3 -DNDEBUG -std=c++17 -fPIC
```

**反汇编验证（Bilateral Example 目标文件）：**

```bash
objdump -d .../BilateralImageFilter.cxx.o | grep fmla | head -5
```

**实测输出（节选）：**

```text
  24:  4e60cce5  fmla  v5.2d, v7.2d, v0.2d
  28:  4fc61004  fmla  v4.2d, v0.2d, v6.d[0]
 300:  4e70cf73  fmla  v19.2d, v27.2d, v16.2d
```

可见生成的是 **`fmla v*.2d`（double NEON，128-bit 内 2 路）**，而非 **`fmla v*.4s`（float NEON，4 路）**。向量化 **已开启**，但宽度是 **double 级**。

### 8.2 ITK 内部精度：`Image<float>` ≠ 全程 float 计算

`itkNumericTraits.h` 中 `float` 特化：

```cpp
using FloatType = float;
using RealType = double;        // 内部「精确计算」类型
using AccumulateType = double;
```

**Bilateral**（`itkBilateralImageFilter.h`）：

```cpp
using OutputPixelRealType = NumericTraits<OutputPixelType>::RealType;
```

邻域循环内变量（`itkBilateralImageFilter.hxx`）使用 `OutputPixelRealType`，即对 **float 像素** 仍 **提升到 double 再算**。

| 模板实例 | 像素存储 | 核心邻域/exp 运算 |
|---------|---------|------------------|
| `BilateralImageFilter<float,...>` | float | **double（RealType）** |
| `BilateralImageFilter<double,...>` | double | double |

因此：**float 版省一半像素内存，但算力路径仍是 double NEON**；还多一次 float↔double 转换，大图上与 double 实例 **打平或略慢** 符合预期。

### 8.3 float 吞吐优势何时才能体现？

需 **同时** 满足：

1. 算子内部 **真正用 float 累加**（而非 `RealType=double`）  
2. 瓶颈在 **DRAM/L3 带宽**（而非 exp/sqrt、分支、96 线程同步）  
3. 编译器生成 **4×f32** 向量代码（`v*.4s`），而非 **2×f64**（`v*.2d`）

本仓库卷积类实测：**(1) 不满足**（ITK 默认 RealType）；**(2) 部分满足**（Bilateral 非纯访存 bound）；**(3) 不满足**（objdump 见 `fmla.2d`）。

### 8.4 与 RecursiveGaussian 小图 ~2× 的差异

小图 RecursiveGaussian 的 double 路径含 **Cast float→double**，Cast 占比高 → float 直跑显得快很多。  
大图卷积 **计算主导**，Cast 占比下降，且 ITK 内部仍用 double RealType → **float 带宽优势被抵消**。

### 8.5 复现上述验证

```bash
# 1) ITK 编译标志
grep CXX_FLAGS /home/pub/yyq/ITK-5.4.0/build-yyq/CMakeCache.txt | head -3
grep CXX_FLAGS /home/pub/yyq/ITK-5.4.0/build-yyq/Modules/Filtering/ImageFeature/src/CMakeFiles/ITKImageFeature.dir/flags.make

# 2) benchmark 程序编译标志（应为空）
grep CMAKE_CXX_FLAGS /home/pub/yyq/itk_hybrid_precision_demo/build/CMakeCache.txt | head -1

# 3) Bilateral 目标文件中的 NEON 指令
objdump -d /home/pub/yyq/ITK-5.4.0/build-yyq/Examples/Filtering/CMakeFiles/BilateralImageFilter.dir/BilateralImageFilter.cxx.o | grep fmla | head -8

# 4) NumericTraits / Bilateral RealType 源码
grep -E 'RealType|AccumulateType' /home/pub/yyq/ITK-5.4.0/Modules/Core/Common/include/itkNumericTraits.h | head -6
grep OutputPixelRealType /home/pub/yyq/ITK-5.4.0/Modules/Filtering/ImageFeature/include/itkBilateralImageFilter.h
```

### 8.6 实践含义

| 目标 | 建议 |
|------|------|
| **省内存** | 用 `Image<float>` — 有效 |
| **靠改像素类型加速卷积** | **无效**（在 ITK 默认 RealType 下） |
| **真正 float 算力加速** | 需 fork Filter、累加改为 `FloatType`，或换底层库 |
| **算法选型** | 优先 FFT 卷积 vs 空间/盒式卷积（收益远大于 float/double 切换） |

**一句话：** float 没更快，不是因为「没开 SIMD」，而是因为 **ITK 在 float 像素下仍用 double 做核心运算**；NEON 已开，但主要是 **double 宽度** 的 `fmla.2d`。

---

## 9. 复现

```bat
cmd /c D:\ECNU_HPC\ITK_huawei\run_precision_conv_bench.cmd
```

```bash
cd /home/pub/yyq/itk_hybrid_precision_demo/build
export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=96
taskset -c 0-95 ./precision_conv_bench /tmp/BrainProtonDensity2048.png 5
```

---

## 10. 相关文件

| 文件 | 说明 |
|------|------|
| `itk_hybrid_precision_demo/precision_conv_bench.cxx` | 卷积类基准源码 |
| `run_precision_conv_bench.sh` / `.cmd` | 一键跑 256²/1024²/2048² |
| [`操作记录_混合精度float化实验报告_2026-06-10.md`](./操作记录_混合精度float化实验报告_2026-06-10.md) | 滤波/分割 float 化实验 |

---

*2026-06-10 在 202.120.87.20 实测，墙钟 5 次平均。*
