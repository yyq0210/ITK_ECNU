# 送测包：数据集 + 测试脚本

对应测试表：`docs/算法性能测试结果-并行加速_含未绑核.xlsx`  
机器：鲲鹏 `202.120.87.86`（Kunpeng 920 5250，96 核）  
表里 **有绑核时间（I 列）的 435 个函数** 都能测；26 个串行、149 个无时间数据的按表规不跑。

```text
送测/
  data/       测试图（官方切片 + 已生成的 1024）
  src/        全部 bench 源码（含 ITK_BENCH_PRECISION 开关）
  scripts/    编译 / 造图 / 跑数 / 汇总
  results/    跑完后的 CSV / 日志（测试时写入）
  measured_operators.tsv   已测函数名单
```

网格、路径、标签、水平集等**没有独立文件**，由 bench 按 ITK 官方测试协议在程序里生成。  
`data/` 下 PNG **直接能用**，不用编译数据集。缺 1024 图时才跑 `prepare_data.py`。

---

## 官方流程（三步）

绑核一律 `taskset -c 0-95`，96 线程。

| 顺序 | 脚本 | 干什么 |
|------|------|--------|
| 1 | `scripts/run_port_double.sh` | **移植**：绑核、全部可测函数、**只跑双精度** |
| 2 | `scripts/run_mixed.sh` | **混合精度**：绑核、全部可测函数、**float + double**，并算 `max_abs` |
| 3 | `scripts/summarize_mixed_error.sh` | 两次跑完后，汇总误差到 `results/mixed_error_summary.csv` |

一条命令跑完上面三步（含编译）：

```bash
bash scripts/run_test.sh
```

| 表列 | 怎么得到 |
|------|----------|
| I 绑核双精度 | `run_port_double.sh` 的 `ms_double` |
| K 混合精度时间 | `run_mixed.sh` 的 `ms_float` |
| N 误差 | `run_mixed.sh` 的 `max_abs`（float 图相对 double 图） |
| M | `I/K`，汇总脚本会写加速比 |
| H 未绑核 | 可选：`scripts/run_all.sh` 的未绑核段 |
| DCU 列 | 不填 |

串行（Fast Marching、ByReconstruction、HMaxima、细化、填洞、RegionGrowing 等）脚本会跳过，表上时间列留空。

---

## 步骤

在鲲鹏上（ITK 已编好，`ITK_ENABLE_MIXED_PRECISION=ON`）：

```bash
# 1. 拷到机器
#    scp -r 送测 root@202.120.87.86:/home/pub/yyq/送测
#    机器上也可放在 /home/pub/yyq/itk_songce（与 送测 同内容）

cd /home/pub/yyq/送测

# 2. ITK 构建目录（默认已是 87.86 上的路径）
export ITK_DIR=/home/pub/yyq/ITK-5.4.0/build-yyq

# 3. 官方三步（编译 + 移植双精度 + 混合精度 + 汇总）
bash scripts/run_test.sh
```

分步跑也可以：

```bash
bash scripts/compile.sh
bash scripts/run_port_double.sh
bash scripts/run_mixed.sh
bash scripts/summarize_mixed_error.sh
```

结果：

```text
results/suite_double_bound.txt
results/*_double_bound.csv
results/suite_double_bound 对应 fill/path/levelset/metric/remain/extra35/cat3
results/suite_mixed_bound.txt
results/*_mixed_bound.csv
results/mixed_error_summary.csv
results/mixed_error_summary.txt
```

单函数复测（绑核；精度默认 mixed = float+double）：

```bash
BIND=1 bash scripts/run_one.sh precision_fill_missing_bench MeanImageFilter
BIND=1 ITK_BENCH_PRECISION=double bash scripts/run_one.sh precision_fill_missing_bench ConvolutionImageFilter
```

只要 **H 未绑核** 时，再跑（默认 mixed，先绑核再未绑核）：

```bash
bash scripts/run_all.sh
```

不要用 `run_all.sh` 代替官方三步。`run_float.sh` 会转到 `run_mixed.sh`，不再只跑单精度。

---

## 脚本一览

| 脚本 | 作用 |
|------|------|
| `compile.sh` | 编译全部 bench。鲲鹏必须 `env -i`，不要 `-march=native` |
| `run_test.sh` | 官方全流程：编译 → 移植双精度 → 混合精度 → 汇总 |
| `run_port_double.sh` | 绑核、全部函数、只双精度（`ITK_BENCH_PRECISION=double`） |
| `run_mixed.sh` | 绑核、全部函数、float+double+误差（`ITK_BENCH_PRECISION=both`） |
| `summarize_mixed_error.sh` | 读两次结果，写误差表 |
| `run_one.sh` | 单个 bench / 单个函数 |
| `run_all.sh` | 可选：绑核 + 未绑核（填 H 列） |
| `run_float.sh` | 兼容入口，实际执行 `run_mixed.sh` |
| `run_bound_mode.sh` | 上面两个入口共用的绑核实现，不要直接当正式入口 |
| `prepare_data.py` | 缺 1024 图时从官方切片生成 |
| `env.sh` | 公共路径与线程数 |

精度开关（C++ 读环境变量 `ITK_BENCH_PRECISION`）：

| 值 | 行为 |
|----|------|
| `double` | 只跑 `Image<double>`，`ms_float=0` |
| `both` / `mixed` | float 与 double 都跑，并算 `max_abs` |
| `float` | 只跑 `Image<float>`（一般不用） |
| 未设置 | 与 `both` 相同 |

---

## 数据

| 文件 | 用途 |
|------|------|
| `data/BrainProtonDensitySlice.png` | 官方脑切片（ITK Examples/Data） |
| `data/BrainProtonDensitySliceBorder20.png` | 配准固定图原图 |
| `data/BrainProtonDensitySliceShifted13x17y.png` | 配准移动图原图 |
| `data/BrainProtonDensity1024.png` | 上图拉成 1024×1024，主测墙钟 |
| `data/BrainProtonDensity1024_fixed.png` | 配准固定图 1024 |
| `data/BrainProtonDensity1024_moving.png` | 配准移动图 1024 |

缺 1024 时：`python3 scripts/prepare_data.py`（需 Pillow）。本包已带 1024 图时不用再生成。  
汇总脚本按 **Python 3.6** 写（鲲鹏系统 python3 是 3.6.8）。

---

## 环境变量（可选）

| 变量 | 默认 |
|------|------|
| `ITK_DIR` | `/home/pub/yyq/ITK-5.4.0/build-yyq` |
| `ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS` | 96 |
| `BIND_CPUS` | `0-95` |
| `RUNS` | 3 |
| `ITK_BENCH_PRECISION` | `double` / `both` / `float` |
| `MIXED_ABS_TOL` | 汇总时判定误差超标的阈值，默认 `1e-5` |
| `SKIP_COMPILE` | `run_test.sh` 设为 `1` 时跳过编译 |
