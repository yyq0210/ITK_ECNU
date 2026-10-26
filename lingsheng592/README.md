# 灵昇服务器：592 核混合精度测试

本目录是 **灵昇 HPC 上的最终测试入口**。编译 ITK、跑混精计时都必须在灵昇完成；登录节点只负责准备环境与 `dsub` 提交。计时以计算节点 **592 核** 作业为准。

- 分支：[`lingsheng-592-qualified`](https://github.com/yyq0210/ITK_ECNU/tree/lingsheng-592-qualified)
- 改源码 ITK 5.4.0：[`lingsheng-itk-5.4-592`](https://github.com/yyq0210/ITK_ECNU/tree/lingsheng-itk-5.4-592)
- **当前送测主口径：343 个函数**（`qualified_mix100_343.tsv` / `qualified_mix100_343.md`）

不要用鲲鹏脚本当本分支验收流程。不要在登录节点直接跑计算节点脚本。

## 合格标准与当前送测名单

两条须**同时**满足：

1. 可判定的 `max_abs < 1e-5`（套件里只打到 4 位小数的 `0.0000` 不算）
2. `speedup = double_ms / float_ms > 1`（图像 float 存储 vs 图像 double 存储）

| 口径 | 数量 | 说明 |
| --- | ---: | --- |
| **当前送测（主）** | **343** | 作业 1960127：已测混精可跑接口 **一轮 × 每函数 100 次**内层计时后，双标合格集合 |
| 名单文件 | — | `qualified_mix100_343.tsv`（跑测用）、`qualified_mix100_343.md`（按模块表） |
| 重测入口 | — | `bash submit_mix100_343.sh` |

全表 584 行见 `mixed_precision_lingsheng592.csv`（471 已测混精，113 不能做）。盘点说明见 `interface_status_584.md`。

历史多轮浮动结果（作业 1859213，五轮、内层约 1～3 次）见文末「历史口径」；**新验收以 343 为准**。

## 其他使用者：推荐测试流程

目标：在灵昇上只验证当前送测的 **343** 个函数。

### 0. 前置

- 账号能登录灵昇，并已加载 Donau 客户端（`dsub` / `djob`）
- 队列：`q_hpcapp`；单作业申请 `cpu=592`
- 不要把 SSH 密码写进仓库或脚本

### 1. 取代码

```bash
cd /home/share/nsls_yyq/yyq/ITK_ECNU   # 或你的克隆路径，并改 env.sh 里的 YYQ/ROOT
git fetch origin
git checkout lingsheng-592-qualified
git pull --ff-only origin lingsheng-592-qualified
cd lingsheng592
```

若目录不是 git 仓库，从 GitHub 同步本目录文件即可，重点保证存在：

- `qualified_mix100_343.tsv`
- `run_mix100_343.sh` / `submit_mix100_343.sh`
- `env.sh`（按本机路径改 `YYQ` / `ROOT`）

### 2. 确认数据与安装树

默认主图：

```text
$ROOT/test/data/BrainProtonDensity1024.png
```

缺图时在仓库根目录：

```bash
python3 test/scripts/prepare_data.py
```

同目录另有 `BrainProtonDensity1024_fixed.png` / `_moving.png`（配准套件单独跑时用）。Hough / 细化等用 bench 内 64×64 合成图；网格、路径等运行时造数。

确认 ITK 安装与 tarball（`compile_itk.sh` 在现成安装树匹配时会直接退出）：

- `$YYQ/ITK-5.4.0-static/install`
- `$YYQ/InsightToolkit-5.4.0.tar.gz`（需要重编时）

### 3. 登录节点准备（只做一次或库/脚本变更后）

```bash
cd .../lingsheng592
bash build_pinpreload.sh    # 生成 $YYQ/pinpreload.so
bash compile_itk.sh         # 已安装则很快退出
bash compile_benches.sh     # 编译 precision_*_bench，套件支持 ITK_BENCH_ONLY_FILE
```

约束：

- CMake 只用 `/usr/bin/cmake`
- 链接用 `$YYQ/ld-gold`，不要乱加 `-static-libstdc++`
- 线程默认 592；绑核列表在 `env.sh` 的 `COMPUTE_CPUS`

### 4. 提交：只测 343（正式送测复测）

```bash
bash submit_mix100_343.sh          # 默认 RUNS=1
# 若要加内层次数：RUNS=3 bash submit_mix100_343.sh
```

- 作业名：`itk-mix343`
- 结果：`lingsheng592/results/mix100_343_<时间戳>/`
  - `measurements.csv`：每个函数的 float/double 时间、加速比、max_abs
  - `summary.txt`：`qualify_err_and_faster=...`
  - `logs/`：各 bench 日志
- 提交日志：`results/mix100_343.out` / `mix100_343.err`

### 5. 看作业与结果

```bash
djob                 # 或 djob -J <作业号>
# 成功后：
ls results/mix100_343_*/
cat results/mix100_343_*/summary.txt
head results/mix100_343_*/measurements.csv
```

判断是否仍双标合格：与当初相同——`max_abs < 1e-5` 且 `speedup > 1`（套件不可判定的 `0.0000` 不计入）。

### 6. 不要做的事

- 不要在登录节点跑 `run_mix100_343.sh` / `run_qualified.sh`（会检查 `nproc --all >= 600`）
- 不要用 `dsub -I` 做长任务（CLI 断开作业就死）
- 不要对 `/opt`、`/home/share` 做全盘 `find`
- 不要堆多个 592 核作业；排除节点 `cn22976`、`cn23018` 已写在 submit 脚本里

## 灵昇环境速查

| 项 | 值 |
| --- | --- |
| 参考代码路径 | `/home/share/nsls_yyq/yyq/ITK_ECNU` |
| 本目录 | `.../ITK_ECNU/lingsheng592` |
| 调度 | Donau：`dsub` / `djob` / `dqueue` / `dkill`（不是 Slurm） |
| 队列 | `q_hpcapp`，`cpu=592` |
| CMake | `/usr/bin/cmake`（3.22） |
| 绑核 | `pinpreload` + `COMPUTE_CPUS`（避开每 NUMA 管家核） |

计算核（592 = 16×37）：`0-36,38-74,76-112,114-150,152-188,190-226,228-264,266-302,304-340,342-378,380-416,418-454,456-492,494-530,532-568,570-606`。

## 可选：复现 / 扩测（非日常送测）

全量已测混精可跑名单（468）再跑一轮×100 次（耗时长，墙钟可到数十小时）：

```bash
bash submit_mixable_100.sh
```

旧版「误差已 &lt;1e-5」多轮浮动统计：

```bash
bash submit_err_ok_repeat.sh    # 默认 REPEATS=5
```

旧版上界 300 名单复测（已被 343 主口径替代，仅兼容）：

```bash
bash submit_592.sh              # 读 qualified_functions.tsv
```

## 历史口径（备查，非当前送测）

作业 **1859213**（五轮，内层约 1～3 次）曾报：

| 项 | 值 |
| --- | --- |
| 各轮合格数 | 300, 280, 286, 284, 281 |
| 浮动区间 | [280, 300] |
| 每轮都合格 | 217（`stable_every_round_217.md`） |
| 当时上界送测 | 300（`qualified_functions.tsv`） |

当前库打开 float 像素累加。Bilateral 等误差不够或 float 不快的不在 343 里。`CurvatureRegistrationFilter` 仍缺 FFTW。

## 脚本一览

| 文件 | 作用 |
| --- | --- |
| `env.sh` | 路径与线程；换机器只改这里 |
| `build_pinpreload.sh` / `pinpreload.c` | 每线程绑到 592 计算核 |
| `compile_itk.sh` / `apply_float_accum.py` | 编译安装当前混精库 |
| `compile_benches.sh` / `apply_bench_filter.py` | 编译 bench；套件可读 `ITK_BENCH_ONLY_FILE` |
| **`qualified_mix100_343.tsv` / `.md`** | **当前送测 343** |
| **`run_mix100_343.sh` / `submit_mix100_343.sh`** | **只测 343（推荐）** |
| `mixable_candidates.tsv` | 已测混精可跑 468 |
| `run_mixable_100.sh` / `submit_mixable_100.sh` | 全量 468，一轮×100 |
| `qualified_functions.tsv` | 历史上界 300 |
| `run_qualified.sh` / `submit_592.sh` | 历史 300 复测 |
| `stable_every_round_217.*` | 历史五轮交集 217 |
| `mixed_precision_lingsheng592.csv` / `interface_status_584.md` | 584 接口盘点 |

343 名单各 bench 数量：`fill_missing` 200，`remain` 65，`extra35` 26，`metric` 14，`levelset` 13，`cat3` 9，`path_mesh` 7，`remainder_instantiable` 4，`fail_retry` 2，`four_retry` 2，`pipeline` 1。
