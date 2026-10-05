#!/bin/bash
# One process per operator so a segfault cannot kill the other three.
# Must run on a Lingsheng compute node (nproc --all >= 600).
set -u
source "$(cd "$(dirname "$0")" && pwd)/env.sh"

all="$(nproc --all 2>/dev/null || echo 0)"
if [[ "$all" -lt 600 ]]; then
  echo "run this on a Lingsheng compute node (nproc --all=$all)"
  exit 1
fi
bench="$BENCH_BUILD/precision_four_retry_bench"
if [[ ! -x "$bench" && ! -f "$bench" ]]; then
  echo "bench missing: $bench"
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

stamp="$(date +%Y%m%d_%H%M%S)"
out="${RESULTS:-$HERE/results/four_retry_$stamp}"
mkdir -p "$out/logs"
{
  echo "HOST=$(hostname)"
  echo "CPUINFO=$(nproc --all)"
  echo "THREADS=$THREADS"
  echo "PIN=$PIN"
  echo "IMG=$IMG"
  date
} > "$out/node_info.txt"

export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS="$THREADS"
export ITK_BENCH_PRECISION=both
export LD_PRELOAD="$PIN"

csv="$out/four_retry.csv"
echo "module,operator,ms_float,ms_double,speedup,max_abs,rmse" > "$csv"
ok=0
fail=0
for op in \
  VectorNeighborhoodOperatorImageFilter \
  UnaryFrequencyDomainFilter \
  BSplineSyNImageRegistrationMethod \
  TimeVaryingBSplineVelocityFieldImageRegistrationMethod
do
  log="$out/logs/${op}.log"
  echo "RUN $op"
  set +e
  "$bench" "$IMG" 1 "$op" >"$log" 2>&1
  rc=$?
  set -e
  if [[ "$rc" -eq 0 ]]; then
    grep -v '^module,' "$log" | grep ',' >> "$csv" || true
    ok=$((ok + 1))
  else
    echo "Image,$op,FAIL,FAIL,FAIL,rc=$rc," | tee -a "$csv" "$out/fail.txt"
    fail=$((fail + 1))
    echo "FAIL $op rc=$rc"
  fi
done

{
  echo "ok_steps=$ok fail_steps=$fail"
  date
  echo FOUR_RETRY_DONE
} >> "$out/node_info.txt"
echo "FOUR_RETRY_DONE $out"
