#!/bin/bash
# 官方第 2 步：混合精度。绑核、全部可测函数。每个函数 Image<float> 和 Image<double> 各跑一遍，并算 max_abs。
# 全流程：bash scripts/run_test.sh
set -u
export ITK_BENCH_PRECISION=both
exec bash "$(cd "$(dirname "$0")" && pwd)/run_bound_mode.sh"
