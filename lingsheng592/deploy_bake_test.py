#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Upload baked Huawei ITK package, verify SOURCE_BAKED, submit compile+smoke job."""
from __future__ import annotations

import os
from pathlib import Path

import paramiko

HERE = Path(__file__).resolve().parent
REPO = HERE.parent
REMOTE = "/home/share/nsls_yyq/yyq/ITK_ECNU/lingsheng592"
YYQ = "/home/share/nsls_yyq/yyq"
TAR_LOCAL = REPO / "release" / "ITK-5.4.0-Huawei-Kunpeng.tar.gz"
TAR_REMOTE = f"{YYQ}/ITK-5.4.0-Huawei-Kunpeng.tar.gz"


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


def main() -> None:
    if not TAR_LOCAL.exists():
        raise SystemExit(f"missing {TAR_LOCAL}")
    c = connect()
    sftp = c.open_sftp()
    print(f"PUT {TAR_LOCAL.name} ({TAR_LOCAL.stat().st_size} bytes) -> {TAR_REMOTE}")
    sftp.put(str(TAR_LOCAL), TAR_REMOTE)
    for name in (
        "env.sh",
        "compile_itk.sh",
        "compile_benches.sh",
        "apply_float_accum.py",
        "bake_release_itk.py",
        "apply_bench_filter.py",
        "run_mixable_100.sh",
    ):
        sftp.put(str(HERE / name), f"{REMOTE}/{name}")
        print("PUT", name)
    # tiny smoke candidate list
    smoke = HERE / "bake_smoke_candidates.tsv"
    smoke.write_text(
        "bench\tkind\toperator\tcsv_name\truns\n"
        "precision_fill_missing_bench\tlist\tDirectedHausdorffDistanceImageFilter\tDirectedHausdorffDistanceImageFilter\t3\n"
        "precision_fill_missing_bench\tlist\tHausdorffDistanceImageFilter\tHausdorffDistanceImageFilter\t3\n"
        "precision_remain_bench\tlist\tMeanImageFilter\tMeanImageFilter\t3\n",
        encoding="utf-8",
        newline="\n",
    )
    sftp.put(str(smoke), f"{REMOTE}/bake_smoke_candidates.tsv")
    sftp.close()

    verify = rf"""
set -euo pipefail
cd {REMOTE}
sed -i 's/\r$//' env.sh compile_itk.sh compile_benches.sh apply_float_accum.py
source ./env.sh
export ITK_TARBALL={TAR_REMOTE}
export ITK_SRC={YYQ}/itk-src/ITK-5.4.0-Huawei-Kunpeng
echo ITK_TARBALL=$ITK_TARBALL
echo ITK_SRC=$ITK_SRC
rm -rf "$ITK_SRC"
mkdir -p "$(dirname "$ITK_SRC")"
tar -xzf "$ITK_TARBALL" -C "$(dirname "$ITK_SRC")"
test -f "$ITK_SRC/CMakeLists.txt"
grep -q 'ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS' "$ITK_SRC/CMakeLists.txt"
grep -q 'ITK_ACCUMULATION_TYPE' "$ITK_SRC/Modules/Filtering/Smoothing/include/itkMeanImageFilter.h"
grep -q 'ITK_ACCUMULATION_TYPE' "$ITK_SRC/Modules/Filtering/DistanceMap/include/itkDirectedHausdorffDistanceImageFilter.h"
grep -q 'ITK_ACCUMULATION_TYPE' "$ITK_SRC/Modules/Filtering/DistanceMap/include/itkHausdorffDistanceImageFilter.h"
echo SOURCE_BAKED_OK
# sync headers into current install for immediate bench smoke (full rebuild via dsub)
INC={YYQ}/ITK-5.4.0-static/install/include/ITK-5.4
for f in itkMeanImageFilter.h itkDirectedHausdorffDistanceImageFilter.h itkHausdorffDistanceImageFilter.h \
         itkDiscreteGaussianImageFilter.h itkBoxUtilities.h itkBilateralImageFilter.h itkBilateralImageFilter.hxx \
         itkCurvatureAnisotropicDiffusionImageFilter.h itkGradientAnisotropicDiffusionImageFilter.h; do
  src=$(find "$ITK_SRC" -name "$f" | head -1)
  test -n "$src"
  cp -f "$src" "$INC/$f"
  echo synced $f
done
# ensure configure macro present in install
if ! grep -q 'ITK_ACCUMULATION_TYPE' "$INC/itkConfigure.h"; then
  echo 'INSTALL_CONFIG_MISSING_MACRO'
  exit 1
fi
grep -q 'ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS' "$INC/itkConfigure.h"
bash compile_benches.sh
echo BENCH_OK
"""
    print("verify+bench on login node...")
    out, err = run(c, verify, timeout=3600)
    print(out[-4000:] if len(out) > 4000 else out)
    if err.strip():
        print("STDERR:", err[-2000:])
    if "SOURCE_BAKED_OK" not in out or "BENCH_OK" not in out:
        raise SystemExit("verify/bench failed")

    submit = rf"""
set -euo pipefail
source /opt/gridview/donau/scheduler/client/module/donaucli_bashrc >/dev/null 2>&1 || true
cd {REMOTE}
stamp=$(date +%Y%m%d_%H%M%S)
export RESULTS={REMOTE}/results/bake_smoke_$stamp
export CANDIDATES={REMOTE}/bake_smoke_candidates.tsv
export REPEATS=1
export RUNS=3
mkdir -p results
dsub -q q_hpcapp \
  -n itk-bake-smk \
  -nl '!cn22976 !cn23018' \
  -R 'cpu=592,mem=65536MB' \
  -T 7200 \
  -oo results/bake_smoke.out \
  -eo results/bake_smoke.err \
  bash -lc "REPEATS=1 RUNS=3 CANDIDATES='$CANDIDATES' RESULTS='$RESULTS' bash '{REMOTE}/run_mixable_100.sh'"
echo SUBMITTED_RESULTS=$RESULTS
djob -u nsls_yyq | head -15
"""
    out2, err2 = run(c, submit, timeout=120)
    print(out2)
    if err2.strip():
        print("STDERR:", err2[-1000:])
    c.close()
    print("ALL_DONE")


if __name__ == "__main__":
    main()
