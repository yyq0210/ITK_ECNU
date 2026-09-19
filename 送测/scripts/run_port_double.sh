#!/bin/bash
# 官方第 1 步：移植测试。绑核、全部可测函数、只跑双精度。成功 = 能在鲲鹏上跑通。
# 全流程：bash scripts/run_test.sh
set -u
export ITK_BENCH_PRECISION=double
exec bash "$(cd "$(dirname "$0")" && pwd)/run_bound_mode.sh"
