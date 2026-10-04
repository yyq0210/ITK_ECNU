#!/bin/bash
# Test only functions with precise max_abs < 1e-5 and speedup > 1 on the current library.
# Run this on a Lingsheng compute node from submit_592.sh. 592 threads, per-thread pin.
set -u
source "$(cd "$(dirname "$0")" && pwd)/env.sh"

all="$(nproc --all 2>/dev/null || echo 0)"
if [[ "$all" -lt 600 ]]; then
  echo "run this on a Lingsheng compute node (nproc --all=$all)"
  exit 1
fi
if [[ ! -x "$BENCH_BUILD/precision_fill_missing_bench" && ! -f "$BENCH_BUILD/precision_fill_missing_bench" ]]; then
  echo "benches are not built: $BENCH_BUILD"
  exit 1
fi
if [[ ! -f "$IMG" ]]; then
  echo "image missing: $IMG"
  exit 1
fi
if [[ ! -f "$PIN" ]]; then
  echo "pinpreload missing: $PIN"
  exit 1
fi
if [[ ! -f "$QUALIFIED" ]]; then
  echo "list missing: $QUALIFIED"
  exit 1
fi

stamp="$(date +%Y%m%d_%H%M%S)"
out="${RESULTS:-$HERE/results/$stamp}"
mkdir -p "$out/only" "$out/logs"
{
  echo "HOST=$(hostname)"
  echo "CPUINFO=$(nproc --all)"
  echo "NPROC=$(nproc)"
  echo "THREADS=$THREADS"
  echo "PIN=$PIN"
  echo "IMG=$IMG"
  echo "ITK_DIR=$ITK_DIR"
  date
} > "$out/node_info.txt"

export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS="$THREADS"
export ITK_BENCH_PRECISION=both
export LD_PRELOAD="$PIN"

ok=0
fail=0
while IFS="$(printf '\t')" read -r bench kind operator runs _rest; do
  [[ "$bench" == "bench" ]] && continue
  [[ -z "${bench:-}" ]] && continue
  if [[ "$kind" == "suite" ]]; then
    printf '%s\n' "$operator" >> "$out/only/$bench.txt"
    continue
  fi
  safe="$(printf '%s' "$operator" | tr '/ ' '__')"
  log="$out/logs/${bench}__${safe}.log"
  echo "RUN list $bench $operator"
  if "$BENCH_BUILD/$bench" "$IMG" "$runs" "$operator" >"$log" 2>&1; then
    ok=$((ok + 1))
  else
    echo "FAIL $bench $operator" | tee -a "$out/fail.txt"
    fail=$((fail + 1))
  fi
done < "$QUALIFIED"

shopt -s nullglob
for list in "$out/only/"*.txt; do
  bench="$(basename "$list" .txt)"
  sort -u "$list" -o "$list"
  export ITK_BENCH_ONLY_FILE="$list"
  log="$out/logs/${bench}.log"
  echo "RUN suite $bench ($(wc -l < "$list") names)"
  runs=3
  if "$BENCH_BUILD/$bench" "$IMG" "$runs" >"$log" 2>&1; then
    ok=$((ok + 1))
  else
    echo "FAIL suite $bench" | tee -a "$out/fail.txt"
    fail=$((fail + 1))
  fi
  unset ITK_BENCH_ONLY_FILE
done

{
  echo "ok_steps=$ok fail_steps=$fail"
  date
  echo QUALIFIED_592_DONE
} >> "$out/node_info.txt"
echo "QUALIFIED_592_DONE $out"
