# 送测包：数据集 + 测试脚本

对应测试表：`docs/算法性能测试结果-并行加速_含未绑核.xlsx`  
机器：鲲鹏 `202.120.87.86`（Kunpeng 920 5250，96 核）

```text
送测/
  data/       测试图（官方切片 + 已生成的 1024）
  src/        全部 bench 源码
  scripts/    编译 / 造 1024 图 / 跑数
  results/    跑完后的 CSV / 日志（空目录，测试时写入）
  measured_operators.tsv   已测函数名单
```

网格、路径、标签、水平集等**没有独立文件**，由 bench 按 ITK 官方测试协议在程序里生成。

---

## 测什么

每个能跑的函数：同一套输入，**双精度一遍、单精度一遍**，各 3 次取平均。

| 表列 | 怎么得到 |
|------|----------|
| H 未绑核 | 不指定 CPU，96 线程，`ms_double` |
| I 绑核 | `taskset -c 0-95`，`ms_double` |
| K 混合精度时间 | 绑核，`ms_float` |
| N 误差 | `max_abs`（float 图相对 double 图） |
| J / M | `H/I`、`I/K` |
| DCU 列 | 不填 |

串行函数、空壳接口、GPU、不能混精的（编号/0-1/彩色/频谱等）：时间列按表规留空。

---

## 步骤

在鲲鹏上（ITK 已编好，`ITK_ENABLE_MIXED_PRECISION=ON`）：

```bash
# 1. 拷到机器，例如
#    scp -r 送测 root@202.120.87.86:/home/pub/yyq/送测

cd /home/pub/yyq/送测

# 2. 指定 ITK 构建目录（默认已是 87.86 上的路径）
export ITK_DIR=/home/pub/yyq/ITK-5.4.0/build-yyq

# 3. 编译测试程序
bash scripts/compile.sh

# 4. 全量跑（先绑核，再未绑核）
bash scripts/run_all.sh

# 5. 结果在 results/
ls results/
```

单函数复测：

```bash
BIND=1 bash scripts/run_one.sh precision_fill_missing_bench MeanImageFilter
```

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

缺 1024 时：`python3 scripts/prepare_data.py`（需 Pillow）。本包已带上 1024 图，一般不用再生成。

---

## 环境变量（可选）

| 变量 | 默认 |
|------|------|
| `ITK_DIR` | `/home/pub/yyq/ITK-5.4.0/build-yyq` |
| `ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS` | 96 |
| `BIND_CPUS` | `0-95` |
| `RUNS` | 3 |
