#!/bin/bash
# 官方送测全流程：编译 → 移植（只双精度）→ 混合精度 → 误差汇总。
# 用法:
#   bash scripts/run_test.sh
#   SKIP_COMPILE=1 bash scripts/run_test.sh    # 已编译过时跳过 compile
set -euo pipefail
DIR="$(cd "$(dirname "$0")" && pwd)"

if [[ "${SKIP_COMPILE:-0}" != "1" ]]; then
  bash "$DIR/compile.sh"
else
  source "$DIR/env.sh"
  if [[ ! -x "$BIN/precision_pipeline_bench" ]]; then
    echo "未编译。去掉 SKIP_COMPILE，或先执行: bash $DIR/compile.sh"
    exit 1
  fi
fi

echo "===== 1/3 移植：绑核、全部函数、只双精度 ====="
bash "$DIR/run_port_double.sh"

echo "===== 2/3 混合精度：绑核、全部函数、float+double ====="
bash "$DIR/run_mixed.sh"

echo "===== 3/3 汇总混合精度误差 ====="
bash "$DIR/summarize_mixed_error.sh"

echo "===== TEST_DONE $(date) ====="
echo "看 results/mixed_error_summary.txt 和 results/mixed_error_summary.csv"
