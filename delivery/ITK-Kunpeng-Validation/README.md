# ITK-Kunpeng-Validation

华为鲲鹏 ITK 5.4 已测函数的验收 bench。完整复测步骤见仓库  
[`docs/已测函数复测指南.md`](../../docs/已测函数复测指南.md)。

## 编译

在鲲鹏上优先用 `report/itk_hybrid_precision_demo/CMakeLists.txt` 同步到  
`/home/pub/yyq/itk_hybrid_precision_demo`，然后：

```bash
bash compile_all_measured.sh
```

本目录 `CMakeLists.txt` 列出全部已测 bench，也可对已安装的 ITK：

```bash
cmake -S . -B build -DITK_DIR=/path/to/ITK/build-yyq
cmake --build build -j
```

## 已测 bench

| 程序 | 复跑脚本 |
|------|----------|
| `precision_pipeline_bench` | `run_1024_remeasure.sh` |
| `precision_bilateral_bench` / `precision_conv_bench` / `precision_diffusion_bench` / `precision_registration_bench` | 同上 |
| `precision_fill_missing_bench` | `run_fill_missing.sh` `run_construct*_1024.sh` |
| `precision_path_mesh_bench` | `run_path_mesh_1024.sh` |
| `precision_levelset_bench` | `run_levelset_1024.sh` |
| `precision_metric_bench` | `run_metric_1024.sh` |
| `precision_remain_bench` | `run_remain_1024.sh` |
| `precision_extra35_bench` | `run_extra35_1024.sh` |
| `precision_cat3_bench` | `run_cat3_1024.sh` |

```bash
./precision_xxx_bench --list
taskset -c 0-95 ./precision_xxx_bench /tmp/BrainProtonDensity1024.png 3 OperatorName
```

## 结果

墙钟 CSV 在 `docs/*_bound.csv`、`docs/*_unbound.csv`。
