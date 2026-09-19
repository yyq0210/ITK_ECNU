# ITK_ECNU

华东师大 / 华为鲲鹏上的 ITK 5.4 移植与混合精度验收。

- **远程代码仓库：** https://github.com/yyq0210/ITK_ECNU
- **鲲鹏运行目录：** `root@202.120.87.86:/home/pub/yyq/送测` 或 `/home/pub/yyq/itk_songce`  
  （服务器上没有 git，以本仓库为准）

## 送测（数据集 + 测试脚本）

统一入口：**[送测/README.md](送测/README.md)**

Excel 里 **有绑核时间的 435 个函数** 都能测。官方三步：

```bash
cd 送测
bash scripts/run_test.sh
```

等价于：编译 → `run_port_double.sh`（移植，只双精度）→ `run_mixed.sh`（混合精度）→ `summarize_mixed_error.sh`。

| 目录 | 内容 |
|------|------|
| `送测/data/` | 官方脑切片 + 已生成的 1024 图（直接能用，不用编译数据集） |
| `送测/src/` | 全部 bench 源码（`ITK_BENCH_PRECISION` 控制 double / mixed） |
| `送测/scripts/` | `run_test.sh` / `compile.sh` / `run_port_double.sh` / `run_mixed.sh` / `summarize_mixed_error.sh` |
| `送测/results/` | 跑完后的 CSV（仓库里为空目录） |

网格、路径、标签等没有独立文件，由 bench 运行时生成。

## 其它目录

| 目录 | 内容 |
|------|------|
| `delivery/ITK-Kunpeng-Validation/` | 与送测同源的 bench / 分批 `run_*_1024.sh` |
| `testdata/` | 官方图副本；`testdata/images/` 含 1024 与配准图 |
| `testdata/scripts/` | 分批跑数脚本（路径仍指向鲲鹏 `/home/pub/yyq/...`） |
| `report/itk_hybrid_precision_demo/` | 鲲鹏上使用的 CMake 工程 |
| `docs/*_bound.csv` `docs/*_unbound.csv` | 已测墙钟 |
| `docs/已测函数复测指南.md` | 复测口径 |
| `release/` | 交付用 ITK 源码包说明与 Validation 快照 |
