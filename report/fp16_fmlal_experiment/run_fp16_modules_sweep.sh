#!/usr/bin/env bash
# 全模块 FP16 存储 + 拓宽性能 sweep（+fp16，非 FMLAL）
set -euo pipefail

EXP="${EXP:-/home/pub/yyq/fp16_fmlal_experiment}"
GCC=/home/pub/gjj/gcc10/bin/g++
MARCH=armv8.2-a+crypto+fp16
REPORT="${REPORT:-/tmp/fp16_modules_sweep_$(date +%F_%H%M%S).log}"
RAW="/tmp/brain1024.f32"
W=1024
H=1024
RUNS=3
THREADS=96

exec > >(tee "$REPORT") 2>&1

echo "=== FP16 storage + widen — all modules sweep ==="
echo "date=$(date -Iseconds) report=$REPORT"

cd "$EXP"
mkdir -p build

echo
echo "######## 1. 准备 1024² float 原始数据 ########"
python3 <<PY
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
with open("$RAW", "wb") as f:
    for v in px:
        f.write(struct.pack("<f", float(v)))
print("wrote", "$RAW", "pixels", len(px))
PY

echo
echo "######## 2. 编译 fp16_modules_sweep_bench (+fp16) ########"
env -i HOME="$HOME" PATH=/home/pub/gjj/gcc10/bin:/usr/bin:/usr/local/bin \
  "$GCC" -O3 -fopenmp -std=c++17 -march=$MARCH \
  fp16_modules_sweep_bench.cxx -o build/fp16_modules_sweep_bench

echo
echo "######## 3. 全模块 sweep ########"
taskset -c 0-95 ./build/fp16_modules_sweep_bench "$RAW" "$W" "$H" "$RUNS" "$THREADS"

echo
echo "=== done === report=$REPORT"
