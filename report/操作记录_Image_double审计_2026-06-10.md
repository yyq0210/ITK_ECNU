# Image<double> 使用审计 — 2026-06-10

**范围：** 本仓库示例代码、ITK 5.4.0 官方 Example（远端 `/home/pub/yyq/ITK-5.4.0`）、常见业务 pipeline 模式  
**结论摘要：** 本仓库 **唯一** 使用 `Image<double>` 的地方是 **hybrid cast 链对照实验**；ITK 官方滤波 Example **默认 float**；**配准/优化** 才是 double 的主战场。

---

## 1. 本仓库代码审计

| 文件 | `Image<double>` 用途 | 可否 float 化 | 建议 |
|------|---------------------|--------------|------|
| `itk_hybrid_precision_demo/hybrid_precision_demo.cxx` | hybrid 支路：cast→double 高斯→cast | **整条支路可删** | 改为仅 float vs double 对照；**默认 pipeline 用 float** |
| `itk_hybrid_precision_demo/hybrid_precision_bench.cxx` | hybrid / full_double 基准 | hybrid 可删 | 保留 float vs double 对比 |
| `itk_hybrid_precision_demo/hybrid_precision_real_bench.cxx` | hybrid / full_double 基准 | hybrid 可删 | 同上 |
| `mpi_hybrid_precision_demo/mpi_hybrid_precision_demo.cxx` | hybrid cast 链 + double 参考 | hybrid 可删 | MPI 体数据 **全程 float**；double 仅作精度参考 |
| `混合精度.md` | 文档描述 | — | 已明确放弃 cast 链 hybrid |

**本仓库无生产业务 pipeline**；`Image<double>` 均为 **实验对照** 引入，非业务必需。

---

## 2. ITK 5.4.0 官方 Example 审计（滤波/分割类）

以下 Example 为本仓库基准脚本实际调用的程序（见 `bench_itk_larger_examples_sweep.sh`）：

| Example | 典型像素类型 | 是否可 float 化 | 说明 |
|---------|-------------|----------------|------|
| `BilateralImageFilter` | **`float`**（`Image<float,2>`） | 已是 float | ITK 官方默认；无需改 |
| `SmoothingRecursiveGaussianImageFilter` | **`float`** | 已是 float | 小图 ~25 ms，降 double 无意义 |
| `DiscreteGaussianImageFilter` | **`float`** | 已是 float | 同上 |
| `ResampleImageFilter` | **`float`** / `short` | 已是 float 或整型 | 重采样用 float 足够 |
| `CurvatureAnisotropicDiffusionImageFilter` | **`float`** | 已是 float | 迭代 PDE；度量内部 RealType 可能 double |
| `GradientAnisotropicDiffusionImageFilter` | **`float`** | 已是 float | 同上 |
| `MedianImageFilter` | **`float`** | 已是 float | — |
| `OtsuThresholdImageFilter` | 输入常为 **`short`/`unsigned char`** | IO 整型，计算可 cast float | 分割阈值对 float 通常足够 |

**结论：** 本仓库压测涉及的 ITK 官方 Example **均已使用 float 或整型存储**，不存在「官方 Example 误用 double 存像素」的问题。

---

## 3. ITK 中 double 的合理使用场景（不应 float 化）

| 模块 | 典型 double 用途 | 是否可 float 化 |
|------|-----------------|----------------|
| **Registration / Metricsv4** | 度量值、梯度、变换参数 | **否** — 优化收敛敏感 |
| **Transform** | 变换矩阵、B-Spline 系数 | **否** — 几何精度 |
| **Image 空间元数据** | origin / spacing / direction | 可选（`ITK_USE_FLOAT_SPACE_PRECISION`） |
| **Statistics** | 协方差、PCA | 视误差预算，通常保留 double |
| **Level-set / 迭代优化** | 水平集函数演化 | **谨慎** — 多步累积误差 |

本仓库 **未包含配准 Example**；若后续加入 `ImageRegistration`，应 **float 图像 + double 度量**。

---

## 4. 分类决策表

| 阶段 | 推荐类型 | 理由 |
|------|---------|------|
| 读入 CT/MRI | `int16` / `uint16`（或 IO 原生类型） | 无损存储 |
| 预处理、滤波、形态学 | **`Image<float, Dim>`** | 实测 1.05–1.73× 加速；RecursiveGaussian 精度无差异 |
| 分割（阈值、区域生长） | **`float` 计算**（或整型标签输出） | 阈值对 float 足够 |
| 配准 | **`float` 存储 + `double` 度量/变换** | 速度 + 收敛 |
| hybrid cast 链 | **废弃** | 实测慢于 full_float（0.8–0.97× vs double） |

---

## 5. 行动项

- [x] 完成审计（本文档）
- [x] 示例代码移除 hybrid cast 链，默认推荐 full_float
- [x] 滤波/分割 float vs double 基准 → [`操作记录_混合精度float化实验报告_2026-06-10.md`](./操作记录_混合精度float化实验报告_2026-06-10.md)
- [x] Bilateral float vs double 基准 → 同上
