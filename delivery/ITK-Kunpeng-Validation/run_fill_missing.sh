#!/bin/bash
set -euo pipefail
# 每个算子单独进程，避免一个 SIGSEGV 带走全部
export PATH=/home/pub/gjj/gcc10/bin:/home/pub/cmake/bin:/usr/bin:/usr/local/bin:$PATH
export ITK_GLOBAL_DEFAULT_THREADER=Pool
export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=96
export OMP_NUM_THREADS=96
IMG=${IMG:-/home/pub/yyq/ITK-5.4.0/Examples/Data/BrainProtonDensitySlice.png}
BIN=${BIN:-/tmp/fill_missing_bench/precision_fill_missing_bench}
OUT=${OUT:-/tmp/fill_missing_20260910.csv}
RUNS=${RUNS:-3}
TIMEOUT_SEC=${TIMEOUT_SEC:-90}

echo "module,operator,ms_float,ms_double,speedup,max_abs,rmse" > "$OUT"
mapfile -t OPS < <("$BIN" --list)
echo "ops=${#OPS[@]} -> $OUT"
for op in "${OPS[@]}"; do
  [[ -z "$op" ]] && continue
  echo "== $op ==" >&2
  if ! timeout "$TIMEOUT_SEC" taskset -c 0-95 "$BIN" "$IMG" "$RUNS" "$op" >> "$OUT" 2>/tmp/fill_missing_one.err; then
    echo "misc,$op,FAIL,FAIL,FAIL,timeout_or_crash," >> "$OUT"
    tail -3 /tmp/fill_missing_one.err >&2 || true
  fi
done
echo "DONE $(grep -c ',FAIL,' "$OUT" || true) fails / ${#OPS[@]} ops"
wc -l "$OUT"
