#!/bin/bash
# Rebuild benches against the current ITK. Suite benches honor ITK_BENCH_ONLY_FILE.
# Dynamic libstdc++. Gold linker. Do not pass -static-libstdc++.
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
mkdir -p "$GOLD_DIR"
ln -sfn /usr/bin/ld.gold "$GOLD_DIR/ld"

if [[ ! -d "$ITK_DIR" ]]; then
  echo "ITK_DIR missing: $ITK_DIR"
  exit 1
fi
if [[ ! -f "$BENCH_SRC/precision_mode.h" ]]; then
  echo "bench source missing: $BENCH_SRC"
  exit 1
fi

python3 "$HERE/apply_bench_filter.py" "$BENCH_SRC"
if grep -q 'static-libstdc++' "$BENCH_SRC/CMakeLists.txt"; then
  python3 - "$BENCH_SRC/CMakeLists.txt" <<'PY'
import sys
from pathlib import Path
p = Path(sys.argv[1])
lines = [ln for ln in p.read_text(encoding="utf-8").splitlines(True) if "static-libstdc++" not in ln and "static-libgcc" not in ln]
p.write_text("".join(lines), encoding="utf-8")
print("stripped static libstdc++ link options")
PY
fi

if [[ ! -f "$BENCH_BUILD/CMakeCache.txt" ]]; then
  mkdir -p "$BENCH_BUILD"
  "$CMAKE" -S "$BENCH_SRC" -B "$BENCH_BUILD" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER="$CC" \
    -DCMAKE_CXX_COMPILER="$CXX" \
    -DCMAKE_CXX_FLAGS="$FLAGS" \
    -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS" \
    -DITK_DIR="$ITK_DIR"
fi

"$CMAKE" --build "$BENCH_BUILD" -j "$(nproc)"
echo BENCH_OK
