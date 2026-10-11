#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Build delivery list from the max-qualify round of err_ok_repeat results."""
from __future__ import annotations

import argparse
import csv
import shutil
from pathlib import Path


def is_precise(max_abs_text: str, err: float, kind: str) -> bool:
    text = (max_abs_text or "").strip()
    precise = (
        ("e" in text.lower())
        or ("." in text and len(text.split(".")[-1]) >= 6)
        or err == 0.0
        or err >= 1e-4
    )
    if not precise and kind == "suite":
        return False
    return True


def qualifies(row: dict) -> bool:
    if row.get("status") != "OK":
        return False
    try:
        err = float(row["max_abs"])
        sp = float(row["speedup"])
        mf = float(row["ms_float"])
    except (TypeError, ValueError):
        return False
    if not is_precise(row.get("max_abs", ""), err, row.get("kind", "")):
        return False
    return err < 1e-5 and mf > 0 and sp > 1.0


def load_candidates(path: Path) -> dict[str, dict]:
    by_csv: dict[str, dict] = {}
    with path.open(encoding="utf-8") as f:
        for row in csv.DictReader(f, delimiter="\t"):
            key = row["csv_name"]
            by_csv[key] = row
    return by_csv


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("result_root", type=Path, help="err_ok_repeat_<stamp> directory")
    ap.add_argument(
        "--candidates",
        type=Path,
        default=Path(__file__).resolve().parent / "err_ok_candidates.tsv",
    )
    ap.add_argument(
        "--out-tsv",
        type=Path,
        default=Path(__file__).resolve().parent / "qualified_functions.tsv",
    )
    ap.add_argument(
        "--summary-out",
        type=Path,
        default=None,
        help="optional local copy of stability + upper-bound notes",
    )
    args = ap.parse_args()
    root: Path = args.result_root

    cand = load_candidates(args.candidates)
    round_counts: list[tuple[int, int, list[dict]]] = []
    for i in range(1, 32):
        meas = root / f"run_{i}" / "measurements.csv"
        if not meas.exists():
            continue
        rows = list(csv.DictReader(meas.open(encoding="utf-8")))
        qrows = [r for r in rows if qualifies(r)]
        summary = root / f"run_{i}" / "round_summary.txt"
        reported = None
        if summary.exists():
            d = dict(
                line.strip().split("=", 1)
                for line in summary.read_text(encoding="utf-8").splitlines()
                if "=" in line
            )
            reported = int(d.get("qualify_err_and_faster", "0"))
        count = reported if reported is not None else len(qrows)
        round_counts.append((i, count, qrows))

    if not round_counts:
        raise SystemExit(f"no run_*/measurements.csv under {root}")

    counts = [c for _, c, _ in round_counts]
    best_i, best_c, best_rows = max(round_counts, key=lambda t: (t[1], -t[0]))

    # Prefer candidate metadata for TSV columns
    out_rows = []
    missing_meta = []
    for r in best_rows:
        key = r.get("csv_name") or r.get("operator")
        meta = cand.get(key)
        if meta:
            out_rows.append(
                {
                    "bench": meta["bench"],
                    "kind": meta["kind"],
                    "operator": meta["operator"],
                    "csv_name": meta["csv_name"],
                    "runs": meta.get("runs", "1"),
                }
            )
        else:
            missing_meta.append(key)
            out_rows.append(
                {
                    "bench": r.get("bench", ""),
                    "kind": r.get("kind", ""),
                    "operator": r.get("operator", ""),
                    "csv_name": key,
                    "runs": "1",
                }
            )

    # stable sort by existing candidate order if possible
    order = {name: i for i, name in enumerate(cand)}
    out_rows.sort(key=lambda r: order.get(r["csv_name"], 10**9))

    if args.out_tsv.exists():
        bak = args.out_tsv.with_suffix(".tsv.bak_before_upper_bound")
        shutil.copy2(args.out_tsv, bak)

    with args.out_tsv.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(
            f,
            fieldnames=["bench", "kind", "operator", "csv_name", "runs"],
            delimiter="\t",
            lineterminator="\n",
        )
        w.writeheader()
        w.writerows(out_rows)

    stab = root / "stability_summary.txt"
    stab_text = stab.read_text(encoding="utf-8") if stab.exists() else ""
    note = [
        f"source={root}",
        f"round_qualify_counts={counts}",
        f"round_min={min(counts)}",
        f"round_max={max(counts)}",
        f"upper_bound_round={best_i}",
        f"upper_bound_count={best_c}",
        f"delivery_tsv={args.out_tsv}",
        f"delivery_rows={len(out_rows)}",
        f"missing_candidate_meta={len(missing_meta)}",
        "policy=delivery_list_is_floating_interval_upper_bound",
    ]
    if stab_text:
        note.append("--- stability_summary.txt ---")
        note.append(stab_text.rstrip())

    summary_path = args.summary_out or (root / "upper_bound_delivery.txt")
    if not summary_path.is_absolute() and args.summary_out:
        summary_path = Path(__file__).resolve().parent / args.summary_out
    summary_path.parent.mkdir(parents=True, exist_ok=True)
    summary_path.write_text("\n".join(note) + "\n", encoding="utf-8")

    print("\n".join(note))
    if missing_meta:
        print("WARN missing meta sample:", missing_meta[:10])


if __name__ == "__main__":
    main()
