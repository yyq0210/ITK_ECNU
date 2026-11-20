#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Build retest list: all mixable100 ops with precise max_abs < 1e-5."""
from __future__ import annotations

import csv
from collections import Counter, defaultdict
from pathlib import Path

HERE = Path(__file__).resolve().parent
MEAS = HERE / "results" / "mixable100_20261025_182701" / "run_1" / "measurements.csv"
CAND = HERE / "mixable_candidates.tsv"
Q355 = HERE / "qualified_mix100_355.tsv"
CSV_PATH = HERE / "mixed_precision_lingsheng592.csv"
OUT_TSV = HERE / "qualified_err_ok_437.tsv"
OUT_MD = HERE / "qualified_err_ok_437.md"


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
    meta = {
        r["csv_name"]: r
        for r in csv.DictReader(CAND.open(encoding="utf-8"), delimiter="\t")
    }
    for r in list(meta.values()):
        meta.setdefault(r["operator"], r)

    mod = {}
    if CSV_PATH.exists():
        for r in csv.DictReader(CSV_PATH.open(encoding="utf-8-sig")):
            if r.get("函数名"):
                mod[r["函数名"]] = r.get("模块", "")

    dual355 = {
        (r["bench"], r["operator"])
        for r in csv.DictReader(Q355.open(encoding="utf-8"), delimiter="\t")
    }

    rows = []
    seen = set()
    for r in csv.DictReader(MEAS.open(encoding="utf-8")):
        if r.get("status") != "OK":
            continue
        try:
            err = float(r["max_abs"])
            sp = float(r["speedup"])
            mf = float(r["ms_float"])
        except (TypeError, ValueError):
            continue
        kind = r.get("kind", "")
        if not precise(r.get("max_abs", ""), err, kind):
            continue
        if err >= 1e-5:
            continue
        name = r.get("csv_name") or r.get("operator")
        m = meta.get(name, {})
        bench = m.get("bench") or r.get("bench") or ""
        operator = m.get("operator") or r.get("operator") or ""
        csv_name = m.get("csv_name") or name
        kind = m.get("kind") or kind or "list"
        key = (bench, operator)
        if key in seen or not bench or not operator:
            continue
        seen.add(key)
        dual = mf > 0 and sp > 1.0
        rows.append(
            {
                "bench": bench,
                "kind": kind,
                "operator": operator,
                "csv_name": csv_name,
                "runs": "1",
                "speedup": sp,
                "max_abs": err,
                "dual": dual,
                "in_355": key in dual355,
                "module": mod.get(csv_name, ""),
            }
        )

    # keep any 355 entry missing from mixable100 err_ok (should be none)
    for r in csv.DictReader(Q355.open(encoding="utf-8"), delimiter="\t"):
        key = (r["bench"], r["operator"])
        if key in seen:
            continue
        seen.add(key)
        rows.append(
            {
                "bench": r["bench"],
                "kind": r["kind"],
                "operator": r["operator"],
                "csv_name": r["csv_name"],
                "runs": r.get("runs") or "1",
                "speedup": "",
                "max_abs": "",
                "dual": True,
                "in_355": True,
                "module": mod.get(r["csv_name"], ""),
            }
        )

    rows.sort(key=lambda x: (x["bench"], x["kind"], x["operator"]))
    n = len(rows)
    dual_n = sum(1 for x in rows if x["dual"] or x["in_355"])
    err_only = n - sum(1 for x in rows if x["in_355"])

    out = HERE / f"qualified_err_ok_{n}.tsv"
    out_md = HERE / f"qualified_err_ok_{n}.md"
    # also stable alias names expected by scripts
    alias_tsv = HERE / "qualified_err_ok.tsv"
    alias_md = HERE / "qualified_err_ok.md"

    with out.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(
            f,
            fieldnames=["bench", "kind", "operator", "csv_name", "runs"],
            delimiter="\t",
            lineterminator="\n",
        )
        w.writeheader()
        for x in rows:
            w.writerow(
                {
                    "bench": x["bench"],
                    "kind": x["kind"],
                    "operator": x["operator"],
                    "csv_name": x["csv_name"],
                    "runs": x["runs"],
                }
            )
    alias_tsv.write_text(out.read_text(encoding="utf-8"), encoding="utf-8")

    by_bench = Counter(x["bench"] for x in rows)
    lines = [
        f"# 误差达标复测名单（{n}）",
        "",
        "来源：`mixable100` 一轮×100（作业目录 `mixable100_20261025_182701/run_1`）中，",
        "可判定且 `max_abs < 1e-5` 的全部函数；并并入当前送测 355 中若有遗漏项。",
        "",
        "本名单**只要求误差达标**，不要求加速比 >1。用于把「误差已过、速度未过」一并纳入复测。",
        "",
        "| 项 | 数量 |",
        "| --- | ---: |",
        f"| 误差达标合计 | **{n}** |",
        f"| 其中已在 355 双标名单 | {sum(1 for x in rows if x['in_355'])} |",
        f"| 误差达标但原不在 355 | {err_only} |",
        "",
        "## 按 bench",
        "",
        "| bench | 数量 |",
        "| --- | ---: |",
    ]
    for b, c in sorted(by_bench.items()):
        lines.append(f"| `{b}` | {c} |")
    lines += [
        "",
        "## 使用",
        "",
        "```bash",
        "bash submit_err_ok.sh          # 默认 RUNS=1",
        "RUNS=100 bash submit_err_ok.sh",
        "```",
        "",
    ]
    text = "\n".join(lines) + "\n"
    out_md.write_text(text, encoding="utf-8")
    alias_md.write_text(text, encoding="utf-8")

    # rename numbered outputs if n != 437 expectation
    if out != OUT_TSV and OUT_TSV.exists():
        OUT_TSV.unlink()
    if out_md != OUT_MD and OUT_MD.exists():
        OUT_MD.unlink()
    if out != OUT_TSV:
        out.replace(HERE / f"qualified_err_ok_{n}.tsv")
    print(f"wrote {out.name} / {alias_tsv.name}: n={n} in_355={sum(1 for x in rows if x['in_355'])} err_only={err_only}")


if __name__ == "__main__":
    main()
