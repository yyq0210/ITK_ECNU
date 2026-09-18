#!/bin/bash
# 1024² 重测：已有 fill_missing + pipeline/conv/diffusion/registration
# 绑核 taskset 0-95；未绑核无 taskset。均为 96 线程。
set -u
export PATH=/home/pub/gjj/gcc10/bin:/home/pub/cmake/bin:/usr/bin:/usr/local/bin:$PATH
export ITK_GLOBAL_DEFAULT_THREADER=Pool
export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=96
export OMP_NUM_THREADS=96
unset OMP_PROC_BIND OMP_PLACES

DATA=/home/pub/yyq/ITK-5.4.0/Examples/Data
BIN=/home/pub/yyq/itk_hybrid_precision_demo/build
IMG=/tmp/BrainProtonDensity1024.png
FIX=/tmp/BrainProtonDensity1024_fixed.png
MOV=/tmp/BrainProtonDensity1024_moving.png

python3 - <<'PY'
from PIL import Image
import os
try:
    r = Image.Resampling.LANCZOS
except AttributeError:
    r = Image.LANCZOS
src = "/home/pub/yyq/ITK-5.4.0/Examples/Data"
pairs = [
    (f"{src}/BrainProtonDensitySlice.png", "/tmp/BrainProtonDensity1024.png"),
    (f"{src}/BrainProtonDensitySliceBorder20.png", "/tmp/BrainProtonDensity1024_fixed.png"),
    (f"{src}/BrainProtonDensitySliceShifted13x17y.png", "/tmp/BrainProtonDensity1024_moving.png"),
]
for a, b in pairs:
    im = Image.open(a).convert("L").resize((1024, 1024), r)
    im.save(b)
    print("wrote", b, im.size)
PY

run_fill() {
  local tag=$1
  local wrap=$2
  local out=$3
  echo "module,operator,ms_float,ms_double,speedup,max_abs,rmse" > "$out"
  mapfile -t OPS < <("$BIN/precision_fill_missing_bench" --list)
  echo "$tag fill ops=${#OPS[@]}"
  for op in "${OPS[@]}"; do
    [[ -z "$op" ]] && continue
    echo "== $tag $op ==" >&2
    if ! timeout 300 $wrap "$BIN/precision_fill_missing_bench" "$IMG" 3 "$op" >> "$out" 2>/tmp/fill1024_one.err; then
      echo "misc,$op,FAIL,FAIL,FAIL,timeout_or_crash," >> "$out"
      tail -3 /tmp/fill1024_one.err >&2 || true
    fi
  done
}

run_eve() {
  local tag=$1
  local wrap=$2
  local out=$3
  {
    echo "===== PIPELINE $tag $(date) ====="
    timeout 900 $wrap "$BIN/precision_pipeline_bench" "$IMG" 3
    echo "===== CONV $tag $(date) ====="
    timeout 1800 $wrap "$BIN/precision_conv_bench" "$IMG" 3
    echo "===== DIFFUSION $tag $(date) ====="
    timeout 1800 $wrap "$BIN/precision_diffusion_bench" "$IMG" 50 0.125 3 3
    echo "===== REGISTRATION $tag $(date) ====="
    timeout 1800 $wrap "$BIN/precision_registration_bench" "$FIX" "$MOV" 50 3
    echo "===== EVE_DONE $tag $(date) ====="
  } > "$out" 2>&1
}

echo "===== START 1024 $(date) ====="
ls -lh "$IMG" "$FIX" "$MOV"

echo "===== BOUND 1024 ====="
run_fill bound "taskset -c 0-95" /tmp/fill_missing_1024_bound.csv
run_eve bound "taskset -c 0-95" /tmp/evening_1024_bound.txt

echo "===== UNBOUND 1024 ====="
run_fill unbound "" /tmp/fill_missing_1024_unbound.csv
run_eve unbound "" /tmp/evening_1024_unbound.txt

echo "===== ALL_DONE 1024 $(date) ====="
wc -l /tmp/fill_missing_1024_bound.csv /tmp/fill_missing_1024_unbound.csv \
      /tmp/evening_1024_bound.txt /tmp/evening_1024_unbound.txt
