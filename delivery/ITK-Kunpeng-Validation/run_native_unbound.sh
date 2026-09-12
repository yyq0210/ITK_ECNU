#!/bin/bash
# 原生实现 / 未绑核：1 线程 + CPU0，与 96 核绑核同一二进制、同一图像
set -u
export PATH=/home/pub/gjj/gcc10/bin:/home/pub/cmake/bin:/usr/bin:/usr/local/bin:$PATH
export ITK_GLOBAL_DEFAULT_THREADER=Pool
export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=1
export OMP_NUM_THREADS=1
unset OMP_PROC_BIND OMP_PLACES

IMG=/home/pub/yyq/ITK-5.4.0/Examples/Data/BrainProtonDensitySlice.png
DATA=/home/pub/yyq/ITK-5.4.0/Examples/Data
BIN=/home/pub/yyq/itk_hybrid_precision_demo/build
FILL_OUT=/tmp/native_fill_missing_1t.csv
EVE_OUT=/tmp/native_evening_1t.txt

echo "===== START $(date) threads=$(nproc) ====="
echo "ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=$ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS"

echo "module,operator,ms_float,ms_double,speedup,max_abs,rmse" > "$FILL_OUT"
mapfile -t OPS < <("$BIN/precision_fill_missing_bench" --list)
echo "fill_missing ops=${#OPS[@]}"
for op in "${OPS[@]}"; do
  [[ -z "$op" ]] && continue
  echo "== 1t $op ==" >&2
  if ! timeout 180 taskset -c 0 "$BIN/precision_fill_missing_bench" "$IMG" 3 "$op" >> "$FILL_OUT" 2>/tmp/native_one.err; then
    echo "misc,$op,FAIL,FAIL,FAIL,timeout_or_crash," >> "$FILL_OUT"
    tail -5 /tmp/native_one.err >&2 || true
  fi
done
echo "FILL_DONE $(grep -c ',FAIL,' "$FILL_OUT" || true) fails / ${#OPS[@]} ops"

{
  echo "===== PIPELINE 1T $(date) ====="
  timeout 600 taskset -c 0 "$BIN/precision_pipeline_bench" "$IMG" 3
  echo "===== CONV 1T $(date) ====="
  timeout 900 taskset -c 0 "$BIN/precision_conv_bench" "$IMG" 3
  echo "===== DIFFUSION 1T $(date) ====="
  timeout 1200 taskset -c 0 "$BIN/precision_diffusion_bench" "$IMG" 50 0.125 3 3
  echo "===== REGISTRATION 1T $(date) ====="
  timeout 1200 taskset -c 0 "$BIN/precision_registration_bench" \
    "$DATA/BrainProtonDensitySliceBorder20.png" \
    "$DATA/BrainProtonDensitySliceShifted13x17y.png" 50 3
  echo "===== EVENING_DONE $(date) ====="
} > "$EVE_OUT" 2>&1

echo "===== ALL_DONE $(date) ====="
wc -l "$FILL_OUT" "$EVE_OUT"
