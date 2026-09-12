#!/bin/bash
# 只跑 construct2 相对上一轮 construct 多出来的算子
set -u
export PATH=/home/pub/gjj/gcc10/bin:/home/pub/cmake/bin:/usr/bin:/usr/local/bin:$PATH
export ITK_GLOBAL_DEFAULT_THREADER=Pool
export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=96
export OMP_NUM_THREADS=96
unset OMP_PROC_BIND OMP_PLACES
IMG=/tmp/BrainProtonDensity1024.png
OLD=/home/pub/yyq/itk_hybrid_precision_demo/build/precision_fill_missing_bench
NEW=/home/pub/yyq/itk_hybrid_precision_demo/build/precision_construct_bench
PREV=/tmp/construct_1024_bound.csv
OUTB=/tmp/construct2_1024_bound.csv
OUTU=/tmp/construct2_1024_unbound.csv

mapfile -t EXTRA < <(comm -23 <("$NEW" --list | sort) <({ "$OLD" --list; awk -F, 'NR>1{print $2}' "$PREV"; } | sort -u))
echo "construct2_ops=${#EXTRA[@]}"
printf '%s\n' "${EXTRA[@]}" | head

skip_serial() {
  case "$1" in
    *ByReconstruction*|*HMaxima*|*HMinima*|*HConvex*|*HConcave*|*Fillhole*|*GrindPeak*|*RegionalMaxima*|*RegionalMinima*|*BinaryThinning*|*BinaryPruning*|*FastMarching*|*ConnectedThreshold*|*ConfidenceConnected*|*IsolatedConnected*|*NeighborhoodConnected*|*VectorConfidence*)
      return 0 ;;
  esac
  return 1
}

run_list() {
  local wrap=$1
  local out=$2
  echo "module,operator,ms_float,ms_double,speedup,max_abs,rmse" > "$out"
  for op in "${EXTRA[@]}"; do
    [[ -z "$op" ]] && continue
    if skip_serial "$op"; then
      echo "skip serial $op" >&2
      continue
    fi
    echo "== extra2 $op ==" >&2
    if ! timeout 300 $wrap "$NEW" "$IMG" 3 "$op" >> "$out" 2>/tmp/construct2_one.err; then
      echo "misc,$op,FAIL,FAIL,FAIL,timeout_or_crash," >> "$out"
      tail -2 /tmp/construct2_one.err >&2 || true
    fi
  done
}

echo "===== CONSTRUCT2 BOUND $(date) ====="
run_list "taskset -c 0-95" "$OUTB"
echo "===== CONSTRUCT2 UNBOUND $(date) ====="
run_list "" "$OUTU"
echo "===== CONSTRUCT2_DONE $(date) ====="
wc -l "$OUTB" "$OUTU"
grep -c FAIL "$OUTB" "$OUTU" || true
