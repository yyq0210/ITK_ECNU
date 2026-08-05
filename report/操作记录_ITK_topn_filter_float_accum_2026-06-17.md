# 扫频 TopN 算子 float accum 扩展（补丁 3）— 操作记录

**日期：** 2026-06-17  
**远端：** `root@202.120.87.20`，96 线程  
**ITK：** 5.4.0 `/home/pub/yyq/ITK-5.4.0/build-yyq`  
**日志：** `/tmp/topn_float_accum_2026-06-17_095252.log`（补丁应用）；conv+diffusion 见下方实测

---

## 1. 扫频 TopN 与 RealType 审计

依据 [`操作记录_ITK绑核更大Example扫频_2026-05-17.md`](./操作记录_ITK绑核更大Example扫频_2026-05-17.md) 单线程墙钟排序：

| 排名 | Workload | 单线程 (s) | ITK 内部类型 | 本次动作 |
|------|----------|------------|--------------|----------|
| 1–4 | Bilateral 4096²–16384² / 2048² | 12–716 | `OutputPixelRealType` | **补丁 1+2 已完成** |
| 5 | **CAD 50iter** | ~0.60 | `FiniteDifferenceFunction::PixelRealType = double`（硬编码） | **补丁 3** |
| 6 | **GAD 50iter** | ~0.50 | 同上 | **补丁 3** |
| 7 | Bilateral 1024² | ~3.2 | 已完成 | 跳过 |
| — | SmoothGauss | ~0.03 | `InternalRealType = FloatType` **ITK 已内置** | 跳过 |
| — | DiscreteGaussian | ~0.03 | `RealOutputPixelType` | 补丁 1 已完成 |
| — | Resample | ~0.03 | 插值，非 RealType 累加热点 | 跳过 |
| 附加 | **BoxMean** | — | `itkBoxUtilities.h` `AccPixType = RealType` | **补丁 3** |

---

## 2. 补丁 3 修改内容

| 文件 | 改动 |
|------|------|
| `itkFiniteDifferenceFunction.h` | `PixelRealType = double` → `NumericTraits<PixelType>::FloatType` |
| `itkFiniteDifferenceImageFilter.hxx` | `double coeffs[]` → `FiniteDifferenceFunctionType::PixelRealType coeffs[]`（配套 spacing 赋值） |
| `itkBoxUtilities.h` | `AccPixType = RealType` → `FloatType`（2 处） |

**脚本：**

| 操作 | 路径 |
|------|------|
| 应用 | [`apply_patches_topn.sh`](./itk_float_accum_patches/apply_patches_topn.sh) |
| 还原 | [`revert_patches_topn.sh`](./itk_float_accum_patches/revert_patches_topn.sh) |
| 一键 workflow | [`run_topn_float_accum_workflow.cmd`](../run_topn_float_accum_workflow.cmd) |

**依赖：** 补丁 1（`apply_patches.sh`）+ 可选补丁 2（Bilateral 全路径）。

**重编目标：**

```bash
cmake --build build-yyq --target ITKCommon ITKSmoothing ITKAnisotropicSmoothing-all ITKImageFeature -j96
```

**注意：** 修改 `PixelRealType` 后必须同步改 `itkFiniteDifferenceImageFilter.hxx` 中 `coeffs` 数组类型，否则模板实例化编译失败。

---

## 3. Benchmark 程序

| 程序 | 覆盖算子 |
|------|----------|
| `precision_conv_bench` | Bilateral / DiscreteGaussian / Mean / **BoxMean** / FFTConvolution |
| `precision_diffusion_bench`（新增） | **CAD / GAD**（参数与扫频 Example 一致：`iter=50 timeStep=0.125 conductance=3`） |

---

## 4. 补丁 3 后实测（96 线程）

### 4.1 卷积类（1024²，5 runs）

| 算子 | speedup | max_abs |
|------|---------|---------|
| Bilateral | 1.005× | 0 |
| DiscreteGaussian | 1.074× | ~1.2e-5 |
| Mean | 1.019× | ~8e-6 |
| **BoxMean**（补丁 3 新增测项） | **1.017×** | **0** |
| FFTConvolution | 0.975× | ~8e-6 |

### 4.2 扩散类（181×217 slice，50 iter，5 runs）

| 算子 | speedup | max_abs | rmse |
|------|---------|---------|------|
| **CAD** | **1.035×** | **19.6** | 0.34 |
| **GAD** | **0.979×** | **2.5e-4** | 7e-6 |

**解读：**

- **BoxMean / 卷积类：** 与补丁 1 同类，~**1–7%** 加速，精度几乎无损。
- **CAD：** 有 **~3.5%** 墙钟收益，但 **50 次迭代** 后 float/double 路径 **max_abs≈20**（曲率扩散对累加精度更敏感）；若业务使用 CAD，需单独验收敛/视觉效果。
- **GAD：** 补丁 3 **几乎无加速**（在噪声内），精度 **max_abs≈0**。
- **影响面：** `PixelRealType` 在基类 `FiniteDifferenceFunction` 修改，**所有有限差分/Level-set 类 Filter** 在 `Image<float>` 下内部计算均变为 float（含未单独 benchmark 的算子）。

---

## 5. 结论与建议

| 场景 | 建议 |
|------|------|
| Bilateral / Mean / BoxMean / DiscreteGaussian | 补丁 1+3 可保留；收益 ~1–7%，精度 OK |
| GAD | 补丁 3 可选；**速度无显著收益**，精度 OK |
| CAD | 补丁 3 **有小幅加速**但 **迭代误差增大**；生产需实测，或保留 double 内部 |
| RecursiveGaussian | **无需补丁**（ITK 已 `InternalRealType=FloatType`） |

---

## 6. 复现

```bash
bash /home/pub/yyq/itk_float_accum_patches/apply_patches_topn.sh
cd /home/pub/yyq/itk_hybrid_precision_demo/build
taskset -c 0-95 ./precision_conv_bench /tmp/BrainProtonDensity1024.png 5
taskset -c 0-95 ./precision_diffusion_bench \
  /home/pub/yyq/ITK-5.4.0/Examples/Data/BrainProtonDensitySlice.png 50 0.125 3 5
```

Windows：`cmd /c D:\ECNU_HPC\ITK_huawei\run_topn_float_accum_workflow.cmd`

---

*2026-06-17 在 202.120.87.20 实测。*
