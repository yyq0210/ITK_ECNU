#!/bin/bash
# 未绑核 = 不设 CPU 亲和性（无 taskset），原生实现，ITK 默认线程
set -u
export PATH=/home/pub/gjj/gcc10/bin:/home/pub/cmake/bin:/usr/bin:/usr/local/bin:$PATH
export ITK_GLOBAL_DEFAULT_THREADER=Pool
export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=96
export OMP_NUM_THREADS=96
unset OMP_PROC_BIND OMP_PLACES

IMG=/home/pub/yyq/ITK-5.4.0/Examples/Data/BrainProtonDensitySlice.png
DATA=/home/pub/yyq/ITK-5.4.0/Examples/Data
BIN=/home/pub/yyq/itk_hybrid_precision_demo/build
FILL_OUT=/tmp/unbound_fill_missing.csv
EVE_OUT=/tmp/unbound_evening.txt

echo "===== START $(date) nproc=$(nproc) ====="
echo "NO taskset; threads=$ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS"

echo "module,operator,ms_float,ms_double,speedup,max_abs,rmse" > "$FILL_OUT"
mapfile -t OPS < <("$BIN/precision_fill_missing_bench" --list)
echo "fill_missing ops=${#OPS[@]}"
for op in "${OPS[@]}"; do
  [[ -z "$op" ]] && continue
  echo "== unbound $op ==" >&2
  if ! timeout 180 "$BIN/precision_fill_missing_bench" "$IMG" 3 "$op" >> "$FILL_OUT" 2>/tmp/unbound_one.err; then
    echo "misc,$op,FAIL,FAIL,FAIL,timeout_or_crash," >> "$FILL_OUT"
    tail -5 /tmp/unbound_one.err >&2 || true
  fi
done
echo "FILL_DONE $(grep -c ',FAIL,' "$FILL_OUT" || true) fails / ${#OPS[@]} ops"

{
  echo "===== PIPELINE UNBOUND $(date) ====="
  timeout 600 "$BIN/precision_pipeline_bench" "$IMG" 3
  echo "===== CONV UNBOUND $(date) ====="
  timeout 900 "$BIN/precision_conv_bench" "$IMG" 3
  echo "===== DIFFUSION UNBOUND $(date) ====="
  timeout 1200 "$BIN/precision_diffusion_bench" "$IMG" 50 0.125 3 3
  echo "===== REGISTRATION UNBOUND $(date) ====="
  timeout 1200 "$BIN/precision_registration_bench" \
    "$DATA/BrainProtonDensitySliceBorder20.png" \
    "$DATA/BrainProtonDensitySliceShifted13x17y.png" 50 3
  echo "===== EVENING_DONE $(date) ====="
} > "$EVE_OUT" 2>&1

echo "===== ALL_DONE $(date) ====="
wc -l "$FILL_OUT" "$EVE_OUT"
