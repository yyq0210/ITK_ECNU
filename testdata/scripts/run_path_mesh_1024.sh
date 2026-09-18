#!/bin/bash
set -u
export PATH=/home/pub/gjj/gcc10/bin:/home/pub/cmake/bin:/usr/bin:/usr/local/bin:$PATH
export ITK_GLOBAL_DEFAULT_THREADER=Pool
export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=96
export OMP_NUM_THREADS=96
unset OMP_PROC_BIND OMP_PLACES
IMG=/tmp/BrainProtonDensity1024.png
BIN=/home/pub/yyq/itk_hybrid_precision_demo/build/precision_path_mesh_bench
OUTB=/tmp/path_mesh_1024_bound.csv
OUTU=/tmp/path_mesh_1024_unbound.csv

mapfile -t OPS < <("$BIN" --list)
echo "path_mesh_ops=${#OPS[@]}"

run_list() {
  local wrap=$1
  local out=$2
  echo "module,operator,ms_float,ms_double,speedup,max_abs,rmse" > "$out"
  for op in "${OPS[@]}"; do
    [[ -z "$op" ]] && continue
    echo "== $op ==" >&2
    if ! timeout 180 $wrap "$BIN" "$IMG" 3 "$op" >> "$out" 2>/tmp/path_mesh_one.err; then
      echo "misc,$op,FAIL,FAIL,FAIL,timeout_or_crash," >> "$out"
      tail -3 /tmp/path_mesh_one.err >&2 || true
    fi
  done
}

echo "===== PATHMESH BOUND $(date) ====="
run_list "taskset -c 0-95" "$OUTB"
echo "===== PATHMESH UNBOUND $(date) ====="
run_list "" "$OUTU"
echo "===== PATHMESH_DONE $(date) ====="
wc -l "$OUTB" "$OUTU"
