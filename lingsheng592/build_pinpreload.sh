#!/bin/bash
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
mkdir -p "$GOLD_DIR"
ln -sfn /usr/bin/ld.gold "$GOLD_DIR/ld"
"$CC" -shared -fPIC -O2 -pthread -B"$GOLD_DIR" -fno-use-linker-plugin \
  -o "$PIN" "$HERE/pinpreload.c" -ldl
echo "PIN_OK $PIN"
