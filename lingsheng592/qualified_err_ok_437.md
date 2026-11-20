# 误差达标复测名单（437）

来源：`mixable100` 一轮×100（作业目录 `mixable100_20261025_182701/run_1`）中，
可判定且 `max_abs < 1e-5` 的全部函数；并并入当前送测 355 中若有遗漏项。

本名单**只要求误差达标**，不要求加速比 >1。用于把「误差已过、速度未过」一并纳入复测。

| 项 | 数量 |
| --- | ---: |
| 误差达标合计 | **437** |
| 其中已在 355 双标名单 | 355 |
| 误差达标但原不在 355 | 82 |

## 按 bench

| bench | 数量 |
| --- | ---: |
| `precision_cat3_bench` | 14 |
| `precision_extra35_bench` | 34 |
| `precision_fail_retry_bench` | 3 |
| `precision_fill_missing_bench` | 220 |
| `precision_four_retry_bench` | 3 |
| `precision_levelset_bench` | 15 |
| `precision_metric_bench` | 30 |
| `precision_path_mesh_bench` | 13 |
| `precision_pipeline_bench` | 1 |
| `precision_remain_bench` | 100 |
| `precision_remainder_instantiable_bench` | 4 |

## 使用

```bash
bash submit_err_ok.sh          # 默认 RUNS=1
RUNS=100 bash submit_err_ok.sh
```

