#!/bin/bash
set -euo pipefail
python3 <<'PY'
from pathlib import Path
src = Path("/tmp/precision_remain_bench.cxx")
dst = Path("/home/pub/yyq/itk_hybrid_precision_demo/precision_remain_bench.cxx")
dst.write_bytes(src.read_bytes().replace(b"\r", b""))
print("wrote", dst, dst.stat().st_size)
PY
bash /tmp/compile_remain.sh
