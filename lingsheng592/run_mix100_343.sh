#!/bin/bash
# Retest only the 343 mixable100 dual-criteria operators (err < 1e-5 and speedup > 1).
# Must run on a Lingsheng compute node (nproc --all >= 600).
set -u
source "$(cd "$(dirname "$0")" && pwd)/env.sh"

QUALIFIED="${QUALIFIED:-$HERE/qualified_mix100_343.tsv}"
RUNS="${RUNS:-1}"

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
out="${RESULTS:-$HERE/results/mix100_343_$stamp}"
mkdir -p "$out/only" "$out/logs"
{
  echo "HOST=$(hostname)"
  echo "CPUINFO=$(nproc --all)"
  echo "THREADS=$THREADS"
  echo "RUNS=$RUNS"
  echo "QUALIFIED=$QUALIFIED"
  echo "PIN=$PIN"
  echo "IMG=$IMG"
  date
} > "$out/node_info.txt"

export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS="$THREADS"
export ITK_BENCH_PRECISION=both
export LD_PRELOAD="$PIN"

meas="$out/measurements.csv"
echo "csv_name,operator,bench,kind,ms_float,ms_double,speedup,max_abs,status" > "$meas"

ok=0
fail=0
while IFS="$(printf '\t')" read -r bench kind operator csv_name runs _rest; do
  [[ "$bench" == "bench" ]] && continue
  [[ -z "${bench:-}" ]] && continue
  local_runs="$RUNS"
  if [[ -n "${runs:-}" && "$runs" != "runs" && "$RUNS" == "1" ]]; then
    local_runs="$runs"
  fi
  if [[ "$kind" == "suite" ]]; then
    printf '%s\n' "$operator" >> "$out/only/${bench}.txt"
    printf '%s\t%s\n' "$operator" "$csv_name" >> "$out/only/${bench}.map"
    continue
  fi
  safe="$(printf '%s' "$operator" | tr '/ ' '__')"
  log="$out/logs/${bench}__${safe}.log"
  echo "RUN list $bench $operator runs=$local_runs"
  set +e
  "$BENCH_BUILD/$bench" "$IMG" "$local_runs" "$operator" >"$log" 2>&1
  rc=$?
  set -e
  line="$(grep -E '^[^,]+,[^,]+,[0-9.eE+-]+,' "$log" | tail -n 1 || true)"
  if [[ -z "$line" ]]; then
    echo "$csv_name,$operator,$bench,list,,,,rc=$rc,FAIL" >> "$meas"
    fail=$((fail + 1))
    continue
  fi
  mf="$(printf '%s' "$line" | cut -d, -f3)"
  md="$(printf '%s' "$line" | cut -d, -f4)"
  sp="$(printf '%s' "$line" | cut -d, -f5)"
  err="$(printf '%s' "$line" | cut -d, -f6)"
  if [[ "$mf" == "FAIL" || "$md" == "FAIL" ]]; then
    echo "$csv_name,$operator,$bench,list,,,,$err,FAIL" >> "$meas"
    fail=$((fail + 1))
  else
    echo "$csv_name,$operator,$bench,list,$mf,$md,$sp,$err,OK" >> "$meas"
    ok=$((ok + 1))
  fi
done < "$QUALIFIED"

shopt -s nullglob
for list in "$out/only/"*.txt; do
  bench="$(basename "$list" .txt)"
  sort -u "$list" -o "$list"
  export ITK_BENCH_ONLY_FILE="$list"
  log="$out/logs/${bench}.log"
  echo "RUN suite $bench ($(wc -l < "$list") names) runs=$RUNS"
  set +e
  "$BENCH_BUILD/$bench" "$IMG" "$RUNS" >"$log" 2>&1
  set -e
  unset ITK_BENCH_ONLY_FILE
  while IFS= read -r op; do
    [[ -z "$op" ]] && continue
    csvn="$op"
    if [[ -f "$out/only/${bench}.map" ]]; then
      mapped="$(awk -F'\t' -v o="$op" '$1==o{print $2; exit}' "$out/only/${bench}.map")"
      [[ -n "$mapped" ]] && csvn="$mapped"
    fi
    hit="$(grep -F "$op | ms_float=" "$log" | tail -n 1 || true)"
    if [[ -z "$hit" ]]; then
      echo "$csvn,$op,$bench,suite,,,,,MISS" >> "$meas"
      fail=$((fail + 1))
      continue
    fi
    mf="$(printf '%s' "$hit" | sed -n 's/.*ms_float=\([0-9.eE+-]*\).*/\1/p')"
    md="$(printf '%s' "$hit" | sed -n 's/.*ms_double=\([0-9.eE+-]*\).*/\1/p')"
    sp="$(printf '%s' "$hit" | sed -n 's/.*speedup=\([0-9.eE+-]*\)x.*/\1/p')"
    err="$(printf '%s' "$hit" | sed -n 's/.*max_abs=\([0-9.eE+-]*\).*/\1/p')"
    echo "$csvn,$op,$bench,suite,$mf,$md,$sp,$err,OK" >> "$meas"
    ok=$((ok + 1))
  done < "$list"
done

python3 - "$meas" "$out/summary.txt" <<'PY'
import csv, sys
from pathlib import Path
meas, outp = Path(sys.argv[1]), Path(sys.argv[2])
rows = list(csv.DictReader(meas.open(encoding="utf-8")))
ok = fail = qualify = err_ok = 0
for r in rows:
    if r.get("status") != "OK":
        fail += 1
        continue
    ok += 1
    try:
        err = float(r["max_abs"])
        sp = float(r["speedup"])
        mf = float(r["ms_float"])
    except (TypeError, ValueError):
        fail += 1
        continue
    text = (r["max_abs"] or "").strip()
    precise = ("e" in text.lower()) or ("." in text and len(text.split(".")[-1]) >= 6) or err == 0.0 or err >= 1e-4
    if not precise and r.get("kind") == "suite":
        continue
    if err < 1e-5:
        err_ok += 1
        if mf > 0 and sp > 1.0:
            qualify += 1
outp.write_text(
    f"measured_ok={ok}\nfail_or_miss={fail}\nerr_ok={err_ok}\nqualify_err_and_faster={qualify}\nMIX100_343_DONE\n",
    encoding="utf-8",
)
print(f"ok={ok} fail={fail} err_ok={err_ok} qualify={qualify}")
PY

{
  echo "ok_steps=$ok fail_steps=$fail"
  date
  echo MIX100_343_DONE
} >> "$out/node_info.txt"
echo "MIX100_343_DONE $out"
