# 送测包公共环境。用法：source "$(dirname "$0")/env.sh"
# 官方流程：bash scripts/run_test.sh
_THIS="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$_THIS/.." && pwd)"

ITK_DIR="${ITK_DIR:-/home/pub/yyq/ITK-5.4.0/build-yyq}"
DATA="$ROOT/data"
SRC="$ROOT/src"
BIN="$ROOT/build"
OUT="$ROOT/results"

SLICE="$DATA/BrainProtonDensitySlice.png"
IMG="$DATA/BrainProtonDensity1024.png"
FIX="$DATA/BrainProtonDensity1024_fixed.png"
MOV="$DATA/BrainProtonDensity1024_moving.png"

if [[ -x /home/pub/gjj/gcc10/bin/g++ ]]; then
  export PATH=/home/pub/gjj/gcc10/bin:/home/pub/cmake/bin:/usr/bin:/usr/local/bin${PATH:+:$PATH}
  CC="${CC:-/home/pub/gjj/gcc10/bin/gcc}"
  CXX="${CXX:-/home/pub/gjj/gcc10/bin/g++}"
  CMAKE="${CMAKE:-/home/pub/cmake/bin/cmake}"
else
  CC="${CC:-gcc}"
  CXX="${CXX:-g++}"
  CMAKE="${CMAKE:-cmake}"
fi

export ITK_GLOBAL_DEFAULT_THREADER=Pool
export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS="${ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS:-96}"
export OMP_NUM_THREADS="$ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS"
unset OMP_PROC_BIND OMP_PLACES

BIND_CPUS="${BIND_CPUS:-0-95}"
RUNS="${RUNS:-3}"

mkdir -p "$OUT" "$BIN"
