#!/bin/bash
# Re-time every mixable operator with prior max_abs < 1e-5, REPEATS times.
# Count how many still have max_abs < 1e-5 AND speedup > 1 each round.
# Must run on a Lingsheng compute node (nproc --all >= 600).
set -u
source "$(cd "$(dirname "$0")" && pwd)/env.sh"

REPEATS="${REPEATS:-5}"
CANDIDATES="${CANDIDATES:-$HERE/err_ok_candidates.tsv}"

all="$(nproc --all 2>/dev/null || echo 0)"
if [[ "$all" -lt 600 ]]; then
  echo "run this on a Lingsheng compute node (nproc --all=$all)"
  exit 1
fi
if [[ ! -f "$CANDIDATES" ]]; then
  echo "candidates missing: $CANDIDATES"
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
root="${RESULTS:-$HERE/results/err_ok_repeat_$stamp}"
mkdir -p "$root"
{
  echo "HOST=$(hostname)"
  echo "CPUINFO=$(nproc --all)"
  echo "THREADS=$THREADS"
  echo "REPEATS=$REPEATS"
  echo "CANDIDATES=$CANDIDATES"
  echo "IMG=$IMG"
  echo "PIN=$PIN"
  date
} > "$root/node_info.txt"

export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS="$THREADS"
export ITK_BENCH_PRECISION=both
export LD_PRELOAD="$PIN"

run_one_round() {
  local round="$1"
  local out="$root/run_$round"
  mkdir -p "$out/only" "$out/logs"
  local meas="$out/measurements.csv"
  echo "csv_name,operator,bench,kind,ms_float,ms_double,speedup,max_abs,status" > "$meas"

  while IFS="$(printf '\t')" read -r bench kind operator csv_name runs prev tag; do
    [[ "$bench" == "bench" ]] && continue
    [[ -z "${bench:-}" ]] && continue
    if [[ "$kind" == "suite" ]]; then
      printf '%s\n' "$operator" >> "$out/only/${bench}.txt"
      # remember csv_name mapping
      printf '%s\t%s\n' "$operator" "$csv_name" >> "$out/only/${bench}.map"
      continue
    fi
    local safe
    safe="$(printf '%s' "$operator" | tr '/ ' '__')"
    local log="$out/logs/${bench}__${safe}.log"
    echo "RUN$round list $bench $operator"
    set +e
    "$BENCH_BUILD/$bench" "$IMG" 1 "$operator" >"$log" 2>&1
    local rc=$?
    set -e
    # parse last CSV data line (skip header / pinpreload)
    local line
    line="$(grep -E '^[^,]+,[^,]+,[0-9.eE+-]+,' "$log" | tail -n 1 || true)"
    if [[ -z "$line" ]]; then
      echo "$csv_name,$operator,$bench,list,,,,rc=$rc,FAIL" >> "$meas"
      continue
    fi
    # module,operator,ms_float,ms_double,speedup,max_abs,rmse
    local mf md sp err
    mf="$(printf '%s' "$line" | cut -d, -f3)"
    md="$(printf '%s' "$line" | cut -d, -f4)"
    sp="$(printf '%s' "$line" | cut -d, -f5)"
    err="$(printf '%s' "$line" | cut -d, -f6)"
    if [[ "$mf" == "FAIL" || "$md" == "FAIL" ]]; then
      echo "$csv_name,$operator,$bench,list,,,,$err,FAIL" >> "$meas"
    else
      echo "$csv_name,$operator,$bench,list,$mf,$md,$sp,$err,OK" >> "$meas"
    fi
  done < "$CANDIDATES"

  shopt -s nullglob
  for list in "$out/only/"*.txt; do
    local bench
    bench="$(basename "$list" .txt)"
    sort -u "$list" -o "$list"
    export ITK_BENCH_ONLY_FILE="$list"
    local log="$out/logs/${bench}.log"
    echo "RUN$round suite $bench ($(wc -l < "$list") names)"
    set +e
    "$BENCH_BUILD/$bench" "$IMG" 3 >"$log" 2>&1
    set -e
    unset ITK_BENCH_ONLY_FILE
    # suite text lines: Name | ms_float=... ms_double=... speedup=...x | max_abs=...
    while IFS= read -r op; do
      [[ -z "$op" ]] && continue
      local csvn="$op"
      if [[ -f "$out/only/${bench}.map" ]]; then
        local mapped
        mapped="$(awk -F'\t' -v o="$op" '$1==o{print $2; exit}' "$out/only/${bench}.map")"
        [[ -n "$mapped" ]] && csvn="$mapped"
      fi
      local hit
      hit="$(grep -F "$op | ms_float=" "$log" | tail -n 1 || true)"
      if [[ -z "$hit" ]]; then
        echo "$csvn,$op,$bench,suite,,,,,MISS" >> "$meas"
        continue
      fi
      local mf md sp err
      mf="$(printf '%s' "$hit" | sed -n 's/.*ms_float=\([0-9.eE+-]*\).*/\1/p')"
      md="$(printf '%s' "$hit" | sed -n 's/.*ms_double=\([0-9.eE+-]*\).*/\1/p')"
      sp="$(printf '%s' "$hit" | sed -n 's/.*speedup=\([0-9.eE+-]*\)x.*/\1/p')"
      err="$(printf '%s' "$hit" | sed -n 's/.*max_abs=\([0-9.eE+-]*\).*/\1/p')"
      echo "$csvn,$op,$bench,suite,$mf,$md,$sp,$err,OK" >> "$meas"
    done < "$list"
  done

  python3 - "$meas" "$out/round_summary.txt" "$round" <<'PY'
import csv, sys
from pathlib import Path
meas, outp, round_i = Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3]
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
        md = float(r["ms_double"])
    except (TypeError, ValueError):
        fail += 1
        continue
    # suite 0.0000 to 4 decimals is not precise; treat as not judgeable unless scientific / more digits
    text = (r["max_abs"] or "").strip()
    precise = ("e" in text.lower()) or ("." in text and len(text.split(".")[-1]) >= 6) or err == 0.0 or err >= 1e-4
    if not precise and r.get("kind") == "suite":
        # keep prior err-ok assumption for suite 0.0000: still require speedup only? No — user wants err.
        # If unjudgeable, do not count as err_ok this round.
        continue
    if err < 1e-5:
        err_ok += 1
        if mf > 0 and sp > 1.0:
            qualify += 1
outp.write_text(
    f"round={round_i}\nmeasured_ok={ok}\nfail_or_miss={fail}\nerr_ok={err_ok}\nqualify_err_and_faster={qualify}\n",
    encoding="utf-8",
)
print(f"ROUND {round_i} err_ok={err_ok} qualify={qualify} fail={fail}")
PY
}

for ((i = 1; i <= REPEATS; i++)); do
  echo "==== REPEAT $i / $REPEATS ===="
  run_one_round "$i"
done

python3 - "$root" "$REPEATS" <<'PY'
import csv, sys
from pathlib import Path
root = Path(sys.argv[1])
n = int(sys.argv[2])
counts = []
for i in range(1, n + 1):
    p = root / f"run_{i}" / "round_summary.txt"
    if not p.exists():
        continue
    d = dict(line.strip().split("=", 1) for line in p.read_text(encoding="utf-8").splitlines() if "=" in line)
    counts.append(int(d.get("qualify_err_and_faster", "0")))
# per-operator: how many rounds qualify
from collections import defaultdict
op_ok = defaultdict(int)
op_seen = defaultdict(int)
for i in range(1, n + 1):
    meas = root / f"run_{i}" / "measurements.csv"
    if not meas.exists():
        continue
    for r in csv.DictReader(meas.open(encoding="utf-8")):
        name = r.get("csv_name") or r.get("operator")
        if r.get("status") != "OK":
            continue
        try:
            err = float(r["max_abs"])
            sp = float(r["speedup"])
            mf = float(r["ms_float"])
        except (TypeError, ValueError):
            continue
        text = (r["max_abs"] or "").strip()
        precise = ("e" in text.lower()) or ("." in text and len(text.split(".")[-1]) >= 6) or err == 0.0 or err >= 1e-4
        if not precise and r.get("kind") == "suite":
            continue
        op_seen[name] += 1
        if err < 1e-5 and mf > 0 and sp > 1.0:
            op_ok[name] += 1

stable = sum(1 for k, v in op_ok.items() if v == n and op_seen.get(k, 0) == n)
any_q = sum(1 for k, v in op_ok.items() if v >= 1)
median_rounds = sorted(counts)[len(counts) // 2] if counts else 0
summary = root / "stability_summary.txt"
lines = [
    f"repeats={n}",
    f"round_qualify_counts={counts}",
    f"round_min={min(counts) if counts else ''}",
    f"round_max={max(counts) if counts else ''}",
    f"round_median={median_rounds}",
    f"operators_qualify_at_least_once={any_q}",
    f"operators_qualify_every_round={stable}",
    "ERR_OK_REPEAT_DONE",
]
summary.write_text("\n".join(lines) + "\n", encoding="utf-8")
print("\n".join(lines))
PY

echo "ERR_OK_REPEAT_DONE $root"
