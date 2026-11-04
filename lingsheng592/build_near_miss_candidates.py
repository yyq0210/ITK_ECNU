#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Build retest candidates: err-ok but speedup<=1, priority near_miss first."""
from __future__ import annotations

import csv
from pathlib import Path

HERE = Path(__file__).resolve().parent
SRC = HERE / "optimize_err_ok_slow_94.tsv"
OUT = HERE / "optimize_near_miss_candidates.tsv"


def main() -> None:
    rows = list(csv.DictReader(SRC.open(encoding="utf-8"), delimiter="\t"))
    # retest all 94, but file name keeps near_miss focus; include medium/hard too for one job
    # User start: prioritize near_miss; include all 94 in one retest pass.
    out_rows = []
    for r in rows:
        out_rows.append(
            {
                "bench": r["bench"],
                "kind": r["kind"],
                "operator": r["operator"],
                "csv_name": r["csv_name"],
                "runs": "100",
                "prev_speedup": r["speedup"],
                "tag": r["priority"],
            }
        )
    with OUT.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(
            f,
            fieldnames=["bench", "kind", "operator", "csv_name", "runs", "prev_speedup", "tag"],
            delimiter="\t",
            lineterminator="\n",
        )
        w.writeheader()
        w.writerows(out_rows)
    print(f"wrote {OUT} n={len(out_rows)}")


if __name__ == "__main__":
    main()
