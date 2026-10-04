# 灵昇 592 核：当前 ITK 编译与合格函数测试

在灵昇计算节点上编译当前 ITK 5.4（静态库、`ITK_DEFAULT_MAX_THREADS=592`、float 像素累加开关打开、gold 链接器），并且只重测已经满足下面两条的函数：

- 可判定的 `max_abs < 1e-5`（流水线里只打到 4 位小数的 `0.0000` 不算）
- `speedup = double_ms / float_ms > 1`

名单共 **299** 个，写在 `qualified_functions.tsv`。基准是计算节点作业 1799867 的 592 核全量结果。当前库打开累加开关之后，作业 1800229 复测过的 Bilateral 误差升到约 4e-4～5e-4，已从名单去掉。`Mean(radius=15)` 和 `BoxMean(radius=15)` 复测后仍满足两条，保留。CAD / GAD 误差下来了，但 float 不比 double 快，不在名单里。配准不是逐像素 `max_abs`，也不在名单里。pipeline、diffusion、bilateral 这三个套件里没有同时满足两条的用例，测试脚本不会启动它们。

各程序数量：

- `precision_cat3_bench`：5
- `precision_conv_bench`：3
- `precision_extra35_bench`：10
- `precision_fill_missing_bench`：197
- `precision_levelset_bench`：14
- `precision_metric_bench`：13
- `precision_path_mesh_bench`：8
- `precision_remain_bench`：49

## 脚本

| 文件 | 作用 |
| --- | --- |
| `compile_itk.sh` | 安装已经是当前库时直接退出。否则用 `/usr/bin/cmake` 和 `ld.gold` 编译安装。已有构建目录时不再重新 configure。 |
| `apply_float_accum.py` | 把 float 累加开关打进 ITK 源码。`ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS` 默认 ON。 |
| `compile_benches.sh` | 给 pipeline / conv / diffusion 加上按名单过滤，并编译 bench。链接动态 libstdc++。 |
| `apply_bench_filter.py` | 套件 bench 读取 `ITK_BENCH_ONLY_FILE`，名字不在文件里的用例直接跳过。 |
| `build_pinpreload.sh` | 编译 `pinpreload.so`。每个新线程绑到 592 个计算核之一，避开每 NUMA 最后一个管家核。 |
| `run_qualified.sh` | 只跑名单里的函数。必须在计算节点上执行（`nproc --all` 为 608）。 |
| `submit_592.sh` | `dsub` 提交到 `q_hpcapp`，`cpu=592`，排除 `cn22976` 和 `cn23018`。 |
| `pinpreload.c` | 绑核预加载库源码。 |

列表类 bench 本来就接受单个算子名。套件类 bench 一次会跑内部全部用例，所以测试脚本先写成 `ITK_BENCH_ONLY_FILE`，再启动对应程序。

## 在灵昇上执行

```bash
cd /home/share/nsls_yyq/yyq/ITK_ECNU/lingsheng592
bash build_pinpreload.sh
bash compile_itk.sh
bash compile_benches.sh
bash submit_592.sh
```

结果写到 `lingsheng592/results/<时间戳>/`，不覆盖已有的登录节点、16 核、592 核全量和累加开关复测结果。

计算核列表（592 = 16×37）：`0-36,38-74,76-112,114-150,152-188,190-226,228-264,266-302,304-340,342-378,380-416,418-454,456-492,494-530,532-568,570-606`。

不要把 SSH 密码写进本目录。
