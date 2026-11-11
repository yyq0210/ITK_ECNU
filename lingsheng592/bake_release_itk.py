#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Bake float-accumulation (CMake switch + filter changes) into the release ITK tree.

Preferred delivery: release/ITK-5.4.0-Huawei-Kunpeng.tar.gz already contains the
mixed-precision source. Users extract and cmake with
-DITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS=ON — no external patch step.
"""
from __future__ import annotations

import os
import shutil
import tarfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parent
RELEASE = REPO / "release"
TAR = RELEASE / "ITK-5.4.0-Huawei-Kunpeng.tar.gz"
BAKE_ROOT = RELEASE / "_itk_bake"
SRC_NAME = "ITK-5.4.0-Huawei-Kunpeng"
SRC = BAKE_ROOT / SRC_NAME


def main() -> None:
    if not TAR.exists():
        raise SystemExit(f"missing {TAR}")
    if SRC.exists():
        shutil.rmtree(SRC)
    BAKE_ROOT.mkdir(parents=True, exist_ok=True)
    if not (BAKE_ROOT / SRC_NAME / "CMakeLists.txt").exists():
        print(f"extract {TAR.name}")
        with tarfile.open(TAR, "r:gz") as tf:
            tf.extractall(BAKE_ROOT)

    os.environ["ITK_SRC"] = str(SRC)
    # Do not touch install/build trees while baking the source package.
    os.environ["ITK_INC"] = str(SRC / "_no_install")
    os.environ["ITK_BUILD"] = str(SRC / "_no_build")

    # Import after env is set so module paths resolve.
    import apply_float_accum as afa

    afa.SRC = SRC
    afa.INC = Path(os.environ["ITK_INC"])
    afa.BUILD = Path(os.environ["ITK_BUILD"])
    afa.BUILD_CONF = afa.BUILD / "Modules/Core/Common/itkConfigure.h"

    afa.patch_switch()
    afa.patch_filters(SRC)
    # skip install_headers — packaging source only

    # Verify bake markers.
    checks = [
        SRC / "CMakeLists.txt",
        SRC / "Modules/Core/Common/src/itkConfigure.h.in",
        SRC / "Modules/Filtering/Smoothing/include/itkMeanImageFilter.h",
        SRC / "Modules/Filtering/DistanceMap/include/itkDirectedHausdorffDistanceImageFilter.h",
        SRC / "Modules/Filtering/DistanceMap/include/itkHausdorffDistanceImageFilter.h",
    ]
    for p in checks:
        text = p.read_text(encoding="utf-8")
        if "ITK_ACCUMULATION_TYPE" not in text and "ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS" not in text:
            raise SystemExit(f"bake incomplete: {p}")
        print("ok", p.relative_to(SRC))

    out = RELEASE / "ITK-5.4.0-Huawei-Kunpeng.tar.gz"
    bak = RELEASE / "ITK-5.4.0-Huawei-Kunpeng.tar.gz.bak"
    if out.exists() and not bak.exists():
        shutil.copy2(out, bak)
        print("backup", bak.name)
    tmp = RELEASE / "ITK-5.4.0-Huawei-Kunpeng.tar.gz.new"
    if tmp.exists():
        tmp.unlink()
    print("repack", out.name)
    with tarfile.open(tmp, "w:gz") as tf:
        tf.add(SRC, arcname=SRC_NAME)
    tmp.replace(out)
    print(f"BAKE_OK size={out.stat().st_size}")


if __name__ == "__main__":
    main()
