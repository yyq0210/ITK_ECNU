#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Build stable_every_round_217.md from measurements + module CSV."""
from __future__ import annotations

import csv
from collections import defaultdict
from pathlib import Path

HERE = Path(__file__).resolve().parent
STABLE_TXT = HERE / "stable_every_round_217.txt"
CSV_PATH = HERE / "mixed_precision_lingsheng592.csv"
MEAS = HERE / "results" / "err_ok_repeat_20261011_205128" / "run_1" / "measurements.csv"
OUT_MD = HERE / "stable_every_round_217.md"


def fmt_num(x, nd=4) -> str:
    if x is None or x == "":
        return ""
    try:
        v = float(x)
    except (TypeError, ValueError):
        return str(x)
    if v == 0:
        return "0"
    if abs(v) < 1e-4:
        return f"{v:.2e}"
    s = f"{v:.{nd}f}".rstrip("0").rstrip(".")
    return s


def fmt_err(x) -> str:
    if x is None or x == "":
        return ""
    try:
        v = float(x)
    except (TypeError, ValueError):
        return str(x)
    if v == 0:
        return "0"
    if v < 1e-4:
        return f"{v:.2e}"
    return f"{v:.8f}".rstrip("0").rstrip(".")


def main() -> None:
    stable = [l for l in STABLE_TXT.read_text(encoding="utf-8").splitlines() if l.strip()]
    rows = list(csv.DictReader(CSV_PATH.open(encoding="utf-8-sig")))
    by = {r["函数名"]: r for r in rows if r.get("函数名")}
    r1: dict[str, dict] = {}
    if MEAS.exists():
        for r in csv.DictReader(MEAS.open(encoding="utf-8")):
            name = r.get("csv_name") or r.get("operator")
            if name:
                r1[name] = r

    groups: dict[str, list] = defaultdict(list)
    for name in stable:
        meta = by.get(name) or by.get(name.split("(")[0])
        mod = (meta or {}).get("模块") or "Unknown"
        groups[mod].append((name, r1.get(name, {}), meta))

    lines: list[str] = []
    lines += [
        "# 灵昇 592 核：五轮均达标函数（217）",
        "",
        "来源：作业 **1859213**（cn22968，`err_ok_repeat_20261011_205128`），"
        "对约 442 个「误差已 `<1e-5`」的能做混合精度接口连跑 **5** 轮。",
        "",
        "达标条件（每轮同时满足）：",
        "",
        "- 可判定的 `max_abs < 1e-5`（套件里仅 4 位小数的 `0.0000` 不算）",
        "- `speedup = double_ms / float_ms > 1`",
        "",
        "| 口径 | 数量 |",
        "| --- | ---: |",
        "| 各轮合格数 | 300, 280, 286, 284, 281 |",
        "| 浮动区间 | [280, 300] |",
        "| **五轮都达标（本文）** | **217** |",
        "| 送测名单（区间上界，见 `qualified_functions.tsv`） | 300 |",
        "| 至少一轮达标 | 350 |",
        "",
        "本文是稳定交集，不是送测上界名单。纯名单见 `stable_every_round_217.txt`。",
        "",
        "下表时间/误差取自第 1 轮 `measurements.csv`（五轮均达标，第 1 轮亦在内）；"
        "模块名来自 `mixed_precision_lingsheng592.csv`。",
        "",
        "## 按模块汇总",
        "",
        "| 模块 | 个数 |",
        "| --- | ---: |",
    ]
    for mod in sorted(groups, key=lambda m: (-len(groups[m]), m)):
        lines.append(f"| {mod} | {len(groups[mod])} |")
    lines.append(f"| **合计** | **{len(stable)}** |")
    lines.append("")
    lines.append("## 分模块名单")
    lines.append("")

    for mod in sorted(groups, key=lambda m: (-len(groups[m]), m)):
        lines.append(f"### {mod}（{len(groups[mod])}）")
        lines.append("")
        lines.append("| 函数 | float_ms | double_ms | 加速比 | max_abs |")
        lines.append("| --- | ---: | ---: | ---: | --- |")
        for name, m, _meta in sorted(groups[mod], key=lambda t: t[0].lower()):
            lines.append(
                "| `{name}` | {mf} | {md} | {sp} | {err} |".format(
                    name=name,
                    mf=fmt_num(m.get("ms_float")),
                    md=fmt_num(m.get("ms_double")),
                    sp=fmt_num(m.get("speedup")),
                    err=fmt_err(m.get("max_abs")),
                )
            )
        lines.append("")

    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"wrote {OUT_MD} names={len(stable)} modules={len(groups)}")


if __name__ == "__main__":
    main()
