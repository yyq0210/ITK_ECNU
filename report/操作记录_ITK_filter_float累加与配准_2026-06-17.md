# ITK Filter float 累加补丁 + Metricsv4 配准 — 操作记录

**日期：** 2026-06-17  
**远端：** `root@202.120.87.20`，96 线程  
**ITK：** 5.4.0 `/home/pub/yyq/ITK-5.4.0/build-yyq`

---

## 1. 完成内容

| 任务 | 状态 | 产出 |
|------|------|------|
| **ITK Filter 内部 float 累加（补丁 1）** | 已完成 | 补丁脚本 + ITK 重编 + 卷积 benchmark 复测 |
| **Bilateral 全路径 float（补丁 2）** | 已完成 | 域核/查表/exp/内层阈值 → `OutputPixelRealType` + 复测 |
| **Metricsv4 float 存储 + double 度量** | 已完成 | `precision_registration_bench` + 实测 |

---

## 2. ITK Filter float 累加补丁

### 2.1 修改原理

将邻域/累加类型从 `NumericTraits<T>::RealType`（float 像素时为 **double**）改为 **`FloatType`（float）**：

| 文件 | 原 typedef | 补丁后 |
|------|-----------|--------|
| `itkBilateralImageFilter.h` | `OutputPixelRealType = RealType` | `FloatType` |
| `itkMeanImageFilter.h` | `InputRealType = RealType` | `FloatType` |
| `itkDiscreteGaussianImageFilter.h` | `RealOutputPixelType = RealType` | `FloatType` |

### 2.2 脚本

| 脚本 | 路径 |
|------|------|
| 应用补丁 | [`itk_float_accum_patches/apply_patches.sh`](./itk_float_accum_patches/apply_patches.sh) |
| 还原补丁 | [`itk_float_accum_patches/revert_patches.sh`](./itk_float_accum_patches/revert_patches.sh) |
| 一键：补丁→重编 ITK→benchmark | [`run_float_accum_workflow.cmd`](../run_float_accum_workflow.cmd) |

```bash
# 应用并重编 ITK 模块
bash itk_float_accum_patches/apply_patches.sh
cd /home/pub/yyq/ITK-5.4.0/build-yyq
cmake --build . --target ITKImageFeature ITKSmoothing -j96
```

### 2.3 补丁后源码（节选）

```cpp
using OutputPixelRealType = typename NumericTraits<OutputPixelType>::FloatType; /* yyq: float accum */
```

### 2.4 卷积 benchmark 对比（1024²，96 线程，3 次平均）

| 算子 | 补丁前 speedup | 补丁后 speedup | max_abs（补丁后） |
|------|---------------|---------------|------------------|
| Bilateral | 0.98× | **1.02×** | **0** |
| DiscreteGaussian | 0.98× | **1.03×** | 1.2e-5 |
| Mean(r=15) | 0.97× | **1.04×** | 7.6e-6 |
| FFTConvolution | 1.02× | 1.00× | 7.6e-6 |

**解读：**

- float 累加补丁带来 **约 2–4% 墙钟收益**（1024²），与「RealType→FloatType 启用 float 算路」一致，但 **远非 2×**（Bilateral 仍有 exp 等非线性 double 路径）。
- Bilateral 补丁后 float/double 输出 **逐像素一致（max_abs=0）**，说明 float 累加对该数据足够。

补丁前数据见 [`操作记录_卷积重算子float_vs_double_2026-06-10.md`](./操作记录_卷积重算子float_vs_double_2026-06-10.md)。

---

## 2.5 Bilateral 补丁 2：域核 / 值域查表 / 内层循环 float 化

### 2.5.1 动机

补丁 1 仅改 `OutputPixelRealType`（累加与像素运算已为 float），但以下仍为 **double**：

| 组件 | 补丁 1 | 补丁 2 |
|------|--------|--------|
| 域高斯核 `m_GaussianKernel` | `Neighborhood<double>` | `Neighborhood<OutputPixelRealType>` |
| 域高斯源图 `GaussianImageType` | `Image<double>` | `Image<OutputPixelRealType>` |
| 值域查表 `m_RangeGaussianTable` | `vector<double>` + `std::exp` | `vector<OutputPixelRealType>` + float exp |
| 内层 `rangeDistanceThreshold` / `distanceToTableIndex` | `double` | `OutputPixelRealType` |

**说明：** `std::exp` 仅在 `BeforeThreadedGenerateData()` 建表时调用（每图一次），**不在逐像素内层循环**；内层热路径主要是查表 + 乘加。

### 2.5.2 脚本

| 脚本 | 路径 |
|------|------|
| 应用补丁 2 | [`apply_bilateral_full_float.sh`](./itk_float_accum_patches/apply_bilateral_full_float.sh) |
| 还原补丁 2 | [`revert_bilateral_full_float.sh`](./itk_float_accum_patches/revert_bilateral_full_float.sh) |
| 一键：补丁 2 → 重编 → benchmark | [`run_bilateral_full_float_workflow.cmd`](../run_bilateral_full_float_workflow.cmd) |

依赖：先应用补丁 1（`apply_patches.sh`）。

### 2.5.3 补丁后 benchmark（2026-06-17，96 线程）

| 场景 | 补丁 1 speedup | 补丁 2 speedup | max_abs |
|------|---------------|---------------|---------|
| Bilateral 1024²（conv_bench, 5 runs） | ~1.02× | **1.006×** | 0 |
| Bilateral 1024²（bilateral_bench, 5 runs） | — | **1.012×** | 0 |
| Bilateral 2048²（bilateral_bench, 3 runs） | — | **1.015×** | 0 |

日志：`/tmp/bilateral_full_float_2026-06-17_093259.log`

**解读：**

- 补丁 2 **未带来明显额外加速**（与补丁 1 同属 ~1–2% 量级，在测量噪声内）。
- 原因：内层乘加在补丁 1 后已是 float；补丁 2 消除的是 **核权重/查表项的 double 存储与乘法**，但 Bilateral 仍 **内存带宽主导**（大邻域、随机访存），且 exp 本就不在内层。
- **精度：** 1024²/2048² 相对 double 路径仍为 **max_abs=0**。

---

## 3. Metricsv4：float 存储 + double 度量

### 3.1 实现模式（与 ImageRegistration1 一致）

```cpp
using PixelType = float;
using FixedImageType = itk::Image<PixelType, 2>;
using TransformType = itk::TranslationTransform<double, 2>;
using OptimizerType = itk::RegularStepGradientDescentOptimizerv4<double>;
using MetricType = itk::MeanSquaresImageToImageMetricv4<FixedImageType, FixedImageType>;
// 插值: LinearInterpolateImageFunction<Image, double>
```

- **像素存储：** `float`
- **变换 / 度量 / 优化器 / 插值坐标：** `double`

### 3.2 程序

[`itk_hybrid_precision_demo/precision_registration_bench.cxx`](./itk_hybrid_precision_demo/precision_registration_bench.cxx)

| 模式 | 说明 |
|------|------|
| **A（推荐）** | `Image<float>` 直接配准 |
| **B（对照）** | 读入后 Cast→`Image<double>` 再配准 |

参数：平移配准，MeanSquares v4，maxIter=50，learningRate=4（同 ImageRegistration1）。

### 3.3 实测（Border20 + Shifted13x17y，96 线程，3 次平均）

| 指标 | float 存储 (A) | double 存储 (B) |
|------|---------------|----------------|
| 墙钟 ms | **273.4** | 371.4 |
| 加速比 (B/A) | — | **1.36×** |
| final_metric | 0.000585 | 0.001297 |
| tx / ty (mm) | 12.999 / 17.000 | 13.002 / 17.000 |
| Δtx / Δty | — | 0.0028 / 0.0003 |

**结论：**

1. **生产配准应使用模式 A**（ITK 官方 Example 默认即如此）。
2. float 存储 **更快**（~36%），因省内存带宽且避免 Cast。
3. 收敛参数 **基本一致**（平移差 <0.003 mm）；度量值有差异（double 存储略高），属数值路径差异，可视任务决定是否用 float。

### 3.4 复现

```bash
cd /home/pub/yyq/itk_hybrid_precision_demo/build
export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=96
taskset -c 0-95 ./precision_registration_bench \
  /home/pub/yyq/ITK-5.4.0/Examples/Data/BrainProtonDensitySliceBorder20.png \
  /home/pub/yyq/ITK-5.4.0/Examples/Data/BrainProtonDensitySliceShifted13x17y.png \
  50 3
```

---

## 4. 注意事项

1. **补丁修改 ITK 源码**，升级 ITK 版本或 `git checkout` 后需重新应用；生产环境建议维护独立 patch 分支。
2. **补丁 1** 改 3 个 Filter 头文件 typedef；**补丁 2** 另改 `itkBilateralImageFilter.h/.hxx`（需补丁 1 先行）。
3. 全库 `NumericTraits<float>::RealType` 仍为 double，其它 Filter 行为不变。
4. **配准** 未改 ITK 源码；Metricsv4 本身已支持 float 图像 + double 度量。
5. 还原：先 `revert_bilateral_full_float.sh`，再 `revert_patches.sh`，重编 ITKImageFeature（+ ITKSmoothing 若还原补丁 1）。

---

## 5. 相关文档

- [`混合精度.md`](./混合精度.md)
- [`操作记录_卷积重算子float_vs_double_2026-06-10.md`](./操作记录_卷积重算子float_vs_double_2026-06-10.md) §8（补丁前 RealType 机理）
- ITK 官方 [`ImageRegistration1`](https://github.com/InsightSoftwareConsortium/ITK/blob/v5.4.0/Examples/RegistrationITKv4/ImageRegistration1.cxx)

---

*2026-06-17 在 202.120.87.20 实测。*
