#!/bin/bash
set -u
export PATH=/home/pub/gjj/gcc10/bin:/home/pub/cmake/bin:/usr/bin:/usr/local/bin:$PATH
export ITK_GLOBAL_DEFAULT_THREADER=Pool
export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=96
export OMP_NUM_THREADS=96
unset OMP_PROC_BIND OMP_PLACES
IMG=/tmp/BrainProtonDensity1024.png
BIN=/home/pub/yyq/itk_hybrid_precision_demo/build/precision_remain_bench
OUTB=/tmp/remain_fix_bound.csv
OUTU=/tmp/remain_fix_unbound.csv
OPS=(
  CropLabelMapFilter
  LabelMapMaskImageFilter
  DisplacementFieldToBSplineImageFilter
  BlockMatchingImageFilter
)

run_list() {
  local wrap=$1
  local out=$2
  echo "module,operator,ms_float,ms_double,speedup,max_abs,rmse" > "$out"
  for op in "${OPS[@]}"; do
    echo "== $op ==" >&2
    if ! timeout 300 $wrap "$BIN" "$IMG" 3 "$op" >> "$out" 2>/tmp/remain_fix_one.err; then
      echo "misc,$op,FAIL,FAIL,FAIL,timeout_or_crash," >> "$out"
      tail -20 /tmp/remain_fix_one.err >&2 || true
    else
      tail -5 /tmp/remain_fix_one.err >&2 || true
    fi
  done
}

echo "===== REMAIN FIX BOUND $(date) ====="
run_list "taskset -c 0-95" "$OUTB"
echo "===== REMAIN FIX UNBOUND $(date) ====="
run_list "" "$OUTU"
echo "===== REMAIN_FIX_DONE $(date) ====="
cat "$OUTB"
echo "-----"
cat "$OUTU"
