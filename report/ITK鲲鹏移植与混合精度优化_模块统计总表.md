# ITK 鲲鹏平台 — 移植与混合精度优化模块统计总表

**数据来源：** 《ITK鲲鹏移植与混合精度优化_最终结项报告》及配套实测  
**测试平台：** `202.120.87.20`，96 逻辑核，GCC 10，`-march=armv8.2-a+crypto -O3`  
**加速比定义：** `ms_double / ms_float`（>1 表示 float 更快）  
**日期：** 2026-09

---

## 1. 总体完成情况

| 范围 | 模块数 | 移植 | 混合精度 | 说明 |
|------|--------|------|----------|------|
| Filtering | **38** | 100% | 100% | 含 4 个 GPU→CPU 等价 |
| Registration | **5**（主线 3 + 等价） | 100% | 100% | Metricsv4 等实测 |
| Segmentation | **12** | 100% | 100% | Otsu 实测，其余代表值 |
| **合计（结项主线）** | **约 55** | **100%** | **100%** | 默认模块编译通过；Example **273** 个 |

---

## 2. 加速比与误差统计（核心表）

| 统计口径 | 模块/样本数 | 最差加速比 | 最好加速比 | 中位数 | 典型误差 (max_abs) |
|----------|-------------|------------|------------|--------|-------------------|
| **Filtering 有加速比条目**（去基类/GPU 重复，32 项） | 32 | **0.96×**（Convolution 小图） | **~2.07×**（Smoothing/RecursiveGaussian） | **1.15×** | 多数 **0**；卷积类 ~1e-5–8e-6 |
| **直接实测算子**（pipeline/conv/diffusion/registration） | 19 | **0.96×** | **2.07×** | **1.07×** | 见第 4 节「需关注」 |
| **配准主线** | 2 实测 + 1 代表 | **1.15×**（PDE 代表） | **1.36×**（Metricsv4） | **1.36×** | Δ平移 **<0.003 mm** |
| **分割** | Otsu 实测 + 其余代表 | **~1.10×** | **1.33×**（Otsu） | **~1.20×** | Otsu **0**；LevelSets 需迭代验收 |
| **全库主线综合**（Filtering+配准+分割代表） | ~37 | **0.96×** | **2.07×** | **1.15×** | 除 CAD 外普遍可接受 |

### 误差分档（Filtering）

| 误差等级 | 典型 max_abs | 模块/算子 |
|----------|--------------|-----------|
| 无损 / 可忽略 | **0** | RecursiveGaussian、Median、Otsu、Bilateral（开关 ON）、多数应用层代表 |
| 数值可忽略 | **~1e-5–8e-6** | Mean、DiscreteGaussian（大图）、Convolution/FFT |
| **需业务验收** | CAD **~20**；GAD **~2.5e-4** | AnisotropicSmoothing / CurvatureFlow 类迭代扩散 |

---

## 3. 按优化收益分档（用户选型）

| 分档 | 加速比区间 | 模块/算子 | 用户建议 |
|------|------------|-----------|----------|
| **高收益** | **1.3–2.1×** | `Smoothing`（RecursiveGaussian **2.07×**）、Median 链 **1.36×**、形态学 **1.36×**、配准 Metricsv4 **1.36×**、Denoising/Otsu **~1.3×** | 优先 `Image<float>`；滤波优先 RecursiveGaussian |
| **中等收益** | **1.1–1.3×** | Thresholding、ImageGradient/Intensity、BiasCorrection、多数「代表值」模块 | 默认 float 即可 |
| **弱加速 / 内存型** | **1.0–1.07×** | Bilateral、Mean/BoxMean、大图 DiscreteGaussian、Resample | 算力收益小，**省一半内存**仍值得 |
| **接近持平或略慢** | **0.96–1.04×** | Convolution 小图、FFT、CAD/GAD | 勿期望算力加速；CAD 要验精度 |

---

## 4. 用户调用需重点关注的模块

| 模块 / 算子 | 原因 | 调用注意 |
|-------------|------|----------|
| **`Smoothing` / `SmoothingRecursiveGaussian`** | 收益最大（**~2×**）、误差 0 | 滤波首选；`Image<float>` |
| **`Registration` / Metricsv4** | **1.36×**，Δ平移 <0.003 mm | **float 图 + double 度量/优化器**，勿整链改 float |
| **`AnisotropicSmoothing`（CAD）** | 加速仅 **~1.04×**，max_abs **~20** | **必须做精度/视觉验收**；线程建议 **16–32**，勿盲目 96 核 |
| **`CurvatureFlow` / 有限差分迭代** | 同扩散类，误差需验收 | 生产前任务级验收 |
| **`ImageFeature` / Bilateral** | 加速 **~1.01×**，误差 0 | 大图主收益是**内存**；多核扫频收益明显（16384² 约 28× 相对单核） |
| **`Convolution` / `FFT`** | 最差约 **0.96×** | 小图可能略慢；大图约 1.0×，别为加速硬改精度 |
| **`Thresholding` / Otsu** | **1.29–1.33×**，误差 0 | 分割阈值类可放心 float |
| **CMake 开关 `ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS`** | 控制 Mean/DiscreteGaussian/Bilateral/扩散等内部累加 | 交付默认 **ON**；对比 upstream 时设 **OFF** 重编 |
| **GPU\*** 模块 | 鲲鹏 CPU 无 OpenCL/CUDA | 请用对应 **CPU 模块**（Smoothing 等） |

---

## 5. 一句话结论

全库主线 **~55 个模块** 移植与混合精度均已完成；综合加速比约 **0.96–2.07×**，中位数约 **1.15×**；多数误差为 0 或 1e-5 量级。调用时优先用 **`Image<float>` + RecursiveGaussian / 配准 float 存 double 算**；对 **CAD/迭代扩散** 单独做精度验收，对 **卷积/FFT/Bilateral** 以内存与稳定性为目标，勿只看算力加速比。

---

*统计口径说明：Filtering「有加速比条目」为去掉 ImageFilterBase / SpatialFunction / GPU 重复后的 32 项；Smoothing 模块级最好值取 RecursiveGaussian 2.07×，中位数计算时 Filtering 模块取各条目代表加速比（区间取中点）。*
