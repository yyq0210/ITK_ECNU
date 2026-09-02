# CAD/GAD 内部 double 迭代 — 操作记录

**日期：** 2026-09-02  
**远端：** `root@202.120.87.86`（鲲鹏新 IP，原 `202.120.87.20`）  
**ITK：** `/home/pub/yyq/ITK-5.4.0`  
**日志：** `/tmp/cad_gad_double_fd_2026-09-02_145628.log`（仅 PixelRealType 还原）；内部 double 迭代见本次 bench 输出

---

## 1. 连接

新地址 **`202.120.87.86`** 可 SSH。ITK 树与 `itk_hybrid_precision_demo` 仍在 `/home/pub/yyq/`。

---

## 2. 第一次尝试：只把 `PixelRealType` 改回 double

CAD/GAD 的 `ComputeUpdate` 局部变量本来就是 `double`。误差来自 **50 步解仍存在 `Image<float>` 里**。

| 算子 | max_abs | rmse | 加速比 |
|------|---------|------|--------|
| CAD | **19.62** | 0.327 | 0.95× |
| GAD | **2.46e-4** | 7e-6 | 0.99× |

与改 `PixelRealType` 之前几乎相同 → **不够**。

---

## 3. 有效改法：float I/O + 内部 double 迭代

在 `itkCurvatureAnisotropicDiffusionImageFilter.h` / `itkGradientAnisotropicDiffusionImageFilter.h` 的 `GenerateData` 中：

1. `Cast` 输入 → `Image<double>`
2. 用同名 Filter 的 double 实例跑完所有 iteration
3. `Cast` 回 `Image<float>` 作为输出

脚本：`delivery/apply_cad_gad_double_iterate.py`（交付烘焙也会调用）。

滤波 float 累加（Mean/Bilateral/BoxMean）未改。

---

## 4. 实测（BrainProtonDensitySlice 181×217，50 iter，timeStep=0.125，conductance=3，96 线程，5 次平均）

| 算子 | 改前 max_abs | 改后 max_abs | rmse | 加速比 |
|------|-------------|--------------|------|--------|
| **CAD** | 19.62 | **8e-6** | 4e-6 | **1.14×** |
| **GAD** | 2.46e-4 | **8e-6** | 4e-6 | **1.00×** |

`8e-6` 为 float 相对 double 参考的约 **1 ULP**（强度约 64–128），达到滤波类底噪。

---

## 5. 复现

```bash
python3 delivery/apply_cad_gad_double_iterate.py /home/pub/yyq/ITK-5.4.0
cd /home/pub/yyq/itk_hybrid_precision_demo/build
cmake --build . --target precision_diffusion_bench -j8
taskset -c 0-95 ./precision_diffusion_bench \
  /home/pub/yyq/ITK-5.4.0/Examples/Data/BrainProtonDensitySlice.png \
  50 0.125 3 5
```
