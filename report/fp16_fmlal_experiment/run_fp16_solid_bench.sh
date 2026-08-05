#!/usr/bin/env bash
# FP16 solid 基准：warmup + 30 对交错 + 中位数，去除测量噪声
set -euo pipefail

EXP="${EXP:-/home/pub/yyq/fp16_fmlal_experiment}"
GCC=/home/pub/gjj/gcc10/bin/g++
MARCH=armv8.2-a+crypto+fp16
REPORT="${REPORT:-/tmp/fp16_solid_$(date +%F_%H%M%S).log}"
RAW="/tmp/brain1024.f32"
W=1024
H=1024
PAIRS=30
WARMUP=5
THREADS=96

exec > >(tee "$REPORT") 2>&1

echo "=== FP16 solid benchmark (de-noised) ==="
echo "date=$(date -Iseconds) report=$REPORT"
echo "pairs=$PAIRS warmup=$WARMUP threads=$THREADS"

cd "$EXP"
mkdir -p build

if [[ ! -f "$RAW" ]]; then
  python3 <<'PY'
from PIL import Image
import struct
src = "/home/pub/yyq/ITK-5.4.0/Examples/Data/BrainProtonDensitySlice256x256.png"
im = Image.open(src)
try:
    resample = Image.Resampling.LANCZOS
except AttributeError:
    resample = Image.LANCZOS
im = im.resize((1024, 1024), resample).convert("F")
px = list(im.getdata())
with open("/tmp/brain1024.f32", "wb") as f:
    for v in px:
        f.write(struct.pack("<f", float(v)))
print("wrote /tmp/brain1024.f32")
PY
fi

echo
echo "######## compile fp16_solid_bench ########"
env -i HOME="$HOME" PATH=/home/pub/gjj/gcc10/bin:/usr/bin:/usr/local/bin \
  "$GCC" -O3 -fopenmp -std=c++17 -march=$MARCH \
  fp16_solid_bench.cxx -o build/fp16_solid_bench

echo
echo "######## run solid bench ########"
export OMP_PROC_BIND=true
export OMP_PLACES=cores
taskset -c 0-95 ./build/fp16_solid_bench "$RAW" "$W" "$H" "$PAIRS" "$WARMUP" "$THREADS"

echo
echo "=== done === report=$REPORT"
