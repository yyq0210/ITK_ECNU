#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Build mixable_candidates.tsv: every previously-tested mixed-precision op."""
from __future__ import annotations

import csv
from pathlib import Path

HERE = Path(__file__).resolve().parent
CSV_PATH = HERE / "mixed_precision_lingsheng592.csv"
ERR_OK = HERE / "err_ok_candidates.tsv"
OUT = HERE / "mixable_candidates.tsv"
SKIP_OUT = HERE / "mixable_candidates_skipped.txt"

# Already-known mappings for ops that were measured but err >= 1e-5
# (not in err_ok_candidates.tsv). Prefer list benches that still expose the name.
EXTRA_MAP = {
    "BinaryErodeImageFilter": ("precision_fill_missing_bench", "list", "BinaryErodeImageFilter"),
    "ErodeObjectMorphologyImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "ErodeObjectMorphologyImageFilter",
    ),
    "CurvatureFlowImageFilter": ("precision_pipeline_bench", "suite", "CurvatureFlow"),
    "MinMaxCurvatureFlowImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "MinMaxCurvatureFlowImageFilter",
    ),
    "DanielssonDistanceMapImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "DanielssonDistanceMapImageFilter",
    ),
    "SignedDanielssonDistanceMapImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "SignedDanielssonDistanceMapImageFilter",
    ),
    "SignedMaurerDistanceMapImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "SignedMaurerDistanceMapImageFilter",
    ),
    "BilateralImageFilter": (
        "precision_conv_bench",
        "suite",
        "Bilateral(domain=4,range=50)",
    ),
    "SobelEdgeDetectionImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "SobelEdgeDetectionImageFilter",
    ),
    "ZeroCrossingBasedEdgeDetectionImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "ZeroCrossingBasedEdgeDetectionImageFilter",
    ),
    "ResampleImageFilter": ("precision_pipeline_bench", "suite", "ResampleIdentity"),
    "DivideImageFilter": ("precision_fill_missing_bench", "list", "DivideImageFilter"),
    "ExpImageFilter": ("precision_fill_missing_bench", "list", "ExpImageFilter"),
    "InvertIntensityImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "InvertIntensityImageFilter",
    ),
    "PowImageFilter": ("precision_fill_missing_bench", "list", "PowImageFilter"),
    "SigmoidImageFilter": ("precision_fill_missing_bench", "list", "SigmoidImageFilter"),
    "BinaryContourImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "BinaryContourImageFilter",
    ),
    "AdditiveGaussianNoiseImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "AdditiveGaussianNoiseImageFilter",
    ),
    "SaltAndPepperNoiseImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "SaltAndPepperNoiseImageFilter",
    ),
    "ShotNoiseImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "ShotNoiseImageFilter",
    ),
    "SpeckleNoiseImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "SpeckleNoiseImageFilter",
    ),
    "DoubleThresholdImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "DoubleThresholdImageFilter",
    ),
    "DiscreteGaussianImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "DiscreteGaussianImageFilter",
    ),
    "MultiResolutionPyramidImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "MultiResolutionPyramidImageFilter",
    ),
    "RecursiveMultiResolutionPyramidImageFilter": (
        "precision_fill_missing_bench",
        "list",
        "RecursiveMultiResolutionPyramidImageFilter",
    ),
    "UnaryFrequencyDomainFilter": (
        "precision_four_retry_bench",
        "list",
        "UnaryFrequencyDomainFilter",
    ),
}

# No current bench exposes these as --list / ONLY_FILE names.
SKIP = {
    "FastMarchingExtensionImageFilter",
    "FastMarchingImageFilter",
    "FastMarchingUpwindGradientImageFilter",
}


def main() -> None:
    err_rows = list(csv.DictReader(ERR_OK.open(encoding="utf-8"), delimiter="\t"))
    by_csv = {r["csv_name"]: r for r in err_rows}

    mix_rows = list(csv.DictReader(CSV_PATH.open(encoding="utf-8-sig")))
    mixable = [r for r in mix_rows if r.get("灵昇复测") == "已测"]

    out_rows = []
    skipped = []
    for r in mixable:
        name = r["函数名"]
        if name in SKIP:
            skipped.append(f"{name}\tno_bench_list_entry")
            continue
        if name in by_csv:
            src = by_csv[name]
            out_rows.append(
                {
                    "bench": src["bench"],
                    "kind": src["kind"],
                    "operator": src["operator"],
                    "csv_name": src["csv_name"],
                    "runs": "100",
                    "prev_speedup": src.get("prev_speedup", ""),
                    "tag": src.get("tag", "from_err_ok"),
                }
            )
            continue
        if name in EXTRA_MAP:
            bench, kind, op = EXTRA_MAP[name]
            out_rows.append(
                {
                    "bench": bench,
                    "kind": kind,
                    "operator": op,
                    "csv_name": name,
                    "runs": "100",
                    "prev_speedup": r.get("灵昇592加速比", ""),
                    "tag": "err_ge_1e-5_extra",
                }
            )
            continue
        skipped.append(f"{name}\tunmapped")

    # stable order: by bench then csv_name
    out_rows.sort(key=lambda x: (x["bench"], x["kind"], x["csv_name"]))

    with OUT.open("w", encoding="utf-8", newline="") as f:
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
        w.writerows(out_rows)

    SKIP_OUT.write_text("\n".join(skipped) + ("\n" if skipped else ""), encoding="utf-8")
    print(f"mixable_csv={len(mixable)} written={len(out_rows)} skipped={len(skipped)} -> {OUT}")
    for s in skipped:
        print("SKIP", s)


if __name__ == "__main__":
    main()
