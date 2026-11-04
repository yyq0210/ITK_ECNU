#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Patch Hausdorff float accum on Lingsheng, rebuild benches, submit retest."""
from __future__ import annotations

import csv
import io
import os
from pathlib import Path

import paramiko

HERE = Path(__file__).resolve().parent
REMOTE = "/home/share/nsls_yyq/yyq/ITK_ECNU/lingsheng592"
OPT94_MEAS = (
    f"{REMOTE}/results/optimize_slow94_20261104_215905/run_1/measurements.csv"
)


def connect():
    pw = os.environ["SSH_PW"]
    c = paramiko.SSHClient()
    c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    c.connect(
        "10.39.0.2",
        username="nsls_yyq",
        password=pw,
        timeout=60,
        banner_timeout=60,
        auth_timeout=60,
        allow_agent=False,
        look_for_keys=False,
    )
    return c


def run(c, cmd, timeout=600):
    stdin, stdout, stderr = c.exec_command(cmd, timeout=timeout)
    out = stdout.read().decode("utf-8", "replace")
    err = stderr.read().decode("utf-8", "replace")
    return out, err


def precise(text, e, kind):
    text = (text or "").strip()
    p = (
        ("e" in text.lower())
        or ("." in text and len(text.split(".")[-1]) >= 6)
        or e == 0.0
        or e >= 1e-4
    )
    if not p and kind == "suite":
        return False
    return True


def main():
    c = connect()
    sftp = c.open_sftp()

    # upload scripts
    for name in (
        "apply_float_accum.py",
        "env.sh",
        "compile_benches.sh",
        "apply_bench_filter.py",
        "run_mixable_100.sh",
        "submit_optimize_slow94.sh",
    ):
        sftp.put(str(HERE / name), f"{REMOTE}/{name}")
        print("PUT", name)

    # fetch opt94 measurements to build still-slow + hausdorff candidates
    with sftp.open(OPT94_MEAS, "r") as f:
        meas_text = f.read().decode("utf-8", "replace")
    with sftp.open(f"{REMOTE}/optimize_near_miss_candidates.tsv", "r") as f:
        cand_text = f.read().decode("utf-8", "replace")

    meta = {
        r["csv_name"]: r
        for r in csv.DictReader(io.StringIO(cand_text), delimiter="\t")
    }
    still = []
    for r in csv.DictReader(io.StringIO(meas_text)):
        name = r.get("csv_name") or r.get("operator")
        if r.get("status") != "OK":
            # still include fails for retest after patch
            m = meta.get(name)
            if m:
                still.append(m)
            continue
        try:
            err = float(r["max_abs"])
            sp = float(r["speedup"])
            mf = float(r["ms_float"])
        except (TypeError, ValueError):
            continue
        if not precise(r.get("max_abs", ""), err, r.get("kind", "")):
            continue
        if err < 1e-5 and mf > 0 and sp > 1.0:
            continue  # already flipped
        m = meta.get(name)
        if m:
            still.append(m)

    # ensure Hausdorff pair present
    for must in (
        "DirectedHausdorffDistanceImageFilter",
        "HausdorffDistanceImageFilter",
    ):
        if not any(x["csv_name"] == must for x in still):
            # try from mixable candidates on disk locally
            local_mix = HERE / "mixable_candidates.tsv"
            if local_mix.exists():
                for row in csv.DictReader(local_mix.open(encoding="utf-8"), delimiter="\t"):
                    if row["csv_name"] == must:
                        still.append(
                            {
                                "bench": row["bench"],
                                "kind": row["kind"],
                                "operator": row["operator"],
                                "csv_name": row["csv_name"],
                                "runs": "100",
                                "prev_speedup": "",
                                "tag": "hausdorff_force",
                            }
                        )
                        break

    # dedupe
    seen = set()
    uniq = []
    for r in still:
        k = r["csv_name"]
        if k in seen:
            continue
        seen.add(k)
        uniq.append(r)

    cand_path = HERE / "optimize_after_hausdorff_candidates.tsv"
    with cand_path.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(
            f,
            fieldnames=[
                "bench",
                "kind",
                "operator",
                "csv_name",
                "runs",
                "prev_speedup",
                "tag",
            ],
            delimiter="\t",
            lineterminator="\n",
        )
        w.writeheader()
        for r in uniq:
            w.writerow(
                {
                    "bench": r["bench"],
                    "kind": r["kind"],
                    "operator": r["operator"],
                    "csv_name": r["csv_name"],
                    "runs": "100",
                    "prev_speedup": r.get("prev_speedup", ""),
                    "tag": r.get("tag", "still_slow"),
                }
            )
    sftp.put(str(cand_path), f"{REMOTE}/optimize_after_hausdorff_candidates.tsv")
    print(f"candidates={len(uniq)}")

    # patch + rebuild benches on server
    patch_cmd = rf"""
set -euo pipefail
source /opt/gridview/donau/scheduler/client/module/donaucli_bashrc >/dev/null 2>&1 || true
cd {REMOTE}
sed -i 's/\r$//' env.sh apply_float_accum.py compile_benches.sh run_mixable_100.sh
source ./env.sh
echo "ITK_TARBALL=$ITK_TARBALL"
echo "ITK_SRC=$ITK_SRC"
echo "ITK_INC=$ITK_INC"
echo "ITK_BUILD=$ITK_BUILD"
test -f "$ITK_TARBALL"
test -d "$ITK_INC"
if [[ ! -f "$ITK_SRC/CMakeLists.txt" ]]; then
  echo "EXTRACT_START"
  mkdir -p "$(dirname "$ITK_SRC")"
  # remove partial extract if present
  rm -rf "$ITK_SRC"
  tar -xzf "$ITK_TARBALL" -C "$(dirname "$ITK_SRC")"
  test -f "$ITK_SRC/CMakeLists.txt"
  echo "EXTRACT_OK"
else
  echo "SRC_ALREADY_PRESENT"
fi
export ITK_SRC ITK_INC ITK_BUILD
python3 apply_float_accum.py
test -f "$ITK_INC/itkConfigure.h"
grep -q 'ITK_ACCUMULATION_TYPE' "$ITK_INC/itkConfigure.h"
grep -q 'ITK_ACCUMULATION_TYPE' "$ITK_INC/itkDirectedHausdorffDistanceImageFilter.h"
grep -q 'ITK_ACCUMULATION_TYPE' "$ITK_INC/itkHausdorffDistanceImageFilter.h"
echo '=== VERIFY HAUSDORFF ==='
grep -n 'ITK_ACCUMULATION_TYPE\|using RealType' "$ITK_INC/itkDirectedHausdorffDistanceImageFilter.h" | head -20
grep -n 'ITK_ACCUMULATION_TYPE\|using RealType' "$ITK_INC/itkHausdorffDistanceImageFilter.h" | head -20
echo '=== COMPILE BENCHES ==='
bash compile_benches.sh
echo BENCH_REBUILD_OK
"""
    print("patching remotely (extract+patch+rebuild; may take several minutes)...")
    out, err = run(c, patch_cmd, timeout=7200)
    print(out[-6000:] if len(out) > 6000 else out)
    if err.strip():
        print("STDERR:", err[-3000:])
    if "BENCH_REBUILD_OK" not in out:
        sftp.close()
        c.close()
        raise SystemExit("bench rebuild failed")

    submit_cmd = rf"""
set -e
source /opt/gridview/donau/scheduler/client/module/donaucli_bashrc >/dev/null 2>&1
cd {REMOTE}
export REPEATS=1
export RUNS=100
export CANDIDATES={REMOTE}/optimize_after_hausdorff_candidates.tsv
stamp=$(date +%Y%m%d_%H%M%S)
export RESULTS={REMOTE}/results/optimize_hausdorff_$stamp
mkdir -p results
dsub -q q_hpcapp \
  -n itk-haus94 \
  -nl '!cn22976 !cn23018' \
  -R 'cpu=592,mem=65536MB' \
  -T 172800 \
  -oo results/optimize_hausdorff.out \
  -eo results/optimize_hausdorff.err \
  bash -lc "REPEATS=1 RUNS=100 CANDIDATES='$CANDIDATES' RESULTS='$RESULTS' bash '{REMOTE}/run_mixable_100.sh'"
echo SUBMITTED_RESULTS=$RESULTS
djob -u nsls_yyq | head -15
"""
    out2, err2 = run(c, submit_cmd, timeout=120)
    print(out2)
    if err2.strip():
        print("STDERR:", err2[-1000:])
    sftp.close()
    c.close()
    print("ALL_DONE")


if __name__ == "__main__":
    main()
