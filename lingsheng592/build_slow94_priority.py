#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Build prioritized list of mixable100 ops with err<1e-5 but speedup<=1."""
from __future__ import annotations

import csv
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
MEAS = HERE / "results" / "mixable100_20261025_182701" / "run_1" / "measurements.csv"
CAND = HERE / "mixable_candidates.tsv"
CSV_PATH = HERE / "mixed_precision_lingsheng592.csv"
OUT = HERE / "optimize_err_ok_slow_94.tsv"
OUT_MD = HERE / "optimize_err_ok_slow_94.md"


def precise(text: str, err: float, kind: str) -> bool:
    text = (text or "").strip()
    ok = (
        ("e" in text.lower())
        or ("." in text and len(text.split(".")[-1]) >= 6)
        or err == 0.0
        or err >= 1e-4
    )
    if not ok and kind == "suite":
        return False
    return True


def main() -> None:
    mod = {}
    if CSV_PATH.exists():
        for r in csv.DictReader(CSV_PATH.open(encoding="utf-8-sig")):
            if r.get("函数名"):
                mod[r["函数名"]] = r.get("模块", "")
    meta = {
        r["csv_name"]: r
        for r in csv.DictReader(CAND.open(encoding="utf-8"), delimiter="\t")
    }

    slow = []
    for r in csv.DictReader(MEAS.open(encoding="utf-8")):
        if r.get("status") != "OK":
            continue
        try:
            err = float(r["max_abs"])
            sp = float(r["speedup"])
            mf = float(r["ms_float"])
            md = float(r["ms_double"])
        except (TypeError, ValueError):
            continue
        if not precise(r.get("max_abs", ""), err, r.get("kind", "")):
            continue
        if err >= 1e-5:
            continue
        if mf > 0 and sp > 1.0:
            continue
        name = r.get("csv_name") or r.get("operator")
        m = meta.get(name, {})
        slow.append(
            {
                "name": name,
                "bench": m.get("bench") or r.get("bench", ""),
                "kind": m.get("kind") or r.get("kind", ""),
                "operator": m.get("operator") or r.get("operator", ""),
                "module": mod.get(name, ""),
                "mf": mf,
                "md": md,
                "sp": sp,
                "err": err,
            }
        )

    # near-miss first (sp>=0.95), then by speedup desc, then by time
    slow.sort(key=lambda x: (0 if x["sp"] >= 0.95 else 1, -x["sp"], -max(x["mf"], x["md"])))

    with OUT.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(
            f,
            fieldnames=[
                "rank",
                "csv_name",
                "module",
                "bench",
                "kind",
                "operator",
                "ms_float",
                "ms_double",
                "speedup",
                "max_abs",
                "priority",
            ],
            delimiter="\t",
            lineterminator="\n",
        )
        w.writeheader()
        for i, x in enumerate(slow, 1):
            if x["sp"] >= 0.95:
                pri = "near_miss"
            elif x["sp"] >= 0.8:
                pri = "medium"
            else:
                pri = "hard"
            w.writerow(
                {
                    "rank": i,
                    "csv_name": x["name"],
                    "module": x["module"],
                    "bench": x["bench"],
                    "kind": x["kind"],
                    "operator": x["operator"],
                    "ms_float": f"{x['mf']:.6g}",
                    "ms_double": f"{x['md']:.6g}",
                    "speedup": f"{x['sp']:.6g}",
                    "max_abs": f"{x['err']:.6g}",
                    "priority": pri,
                }
            )

    pri_c = Counter(
        "near_miss" if x["sp"] >= 0.95 else ("medium" if x["sp"] >= 0.8 else "hard")
        for x in slow
    )
    lines = [
        "# 优化优先：误差已 `<1e-5` 但 float 不快（94）",
        "",
        "来源：作业 1960127 / `mixable100` 一轮×100。双标已过 343；本表是剩余主战场。",
        "",
        "| 优先级 | 条件 | 个数 |",
        "| --- | --- | ---: |",
        f"| near_miss | speedup ≥ 0.95 | {pri_c['near_miss']} |",
        f"| medium | 0.8 ≤ speedup < 0.95 | {pri_c['medium']} |",
        f"| hard | speedup < 0.8 | {pri_c['hard']} |",
        f"| **合计** | | **{len(slow)}** |",
        "",
        "策略：先打 near_miss（差一点就过），再看耗时长的 medium；hard 多半是 float 路径本身更重或含 double 度量。",
        "",
        "## Top near_miss / 高耗时",
        "",
        "| # | 函数 | speedup | float_ms | double_ms | 模块 |",
        "| ---: | --- | ---: | ---: | ---: | --- |",
    ]
    for i, x in enumerate(slow[:25], 1):
        lines.append(
            f"| {i} | `{x['name']}` | {x['sp']:.4f} | {x['mf']:.3f} | {x['md']:.3f} | {x['module']} |"
        )
    lines.append("")
    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"wrote {OUT} n={len(slow)} {dict(pri_c)}")
    print(f"wrote {OUT_MD}")
    for i, x in enumerate(slow[:15], 1):
        print(f"{i:2d} sp={x['sp']:.4f} mf={x['mf']:.3f} {x['name']}")


if __name__ == "__main__":
    main()
