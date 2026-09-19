#!/bin/bash
# 汇总 run_port_double.sh + run_mixed.sh 跑完后的混合精度误差。
set -u
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
if [[ ! -f "$OUT/suite_mixed_bound.txt" ]] && ! ls "$OUT"/*_mixed_bound.csv >/dev/null 2>&1; then
  echo "找不到混合精度结果。请先: bash $ROOT/scripts/run_mixed.sh"
  echo "目录: $OUT"
  exit 1
fi
if [[ ! -f "$OUT/suite_double_bound.txt" ]] && ! ls "$OUT"/*_double_bound.csv >/dev/null 2>&1; then
  echo "警告: 找不到移植（双精度）结果，只汇总混合精度。建议先跑 bash $ROOT/scripts/run_port_double.sh"
fi
exec python3 "$ROOT/scripts/summarize_mixed_error.py" "$OUT"
