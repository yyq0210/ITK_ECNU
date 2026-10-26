#!/usr/bin/env python3
"""Download mixable100 round-1 measurements and build the 343 qualify list."""
from __future__ import annotations

import csv
import io
import os
from collections import Counter, defaultdict
from pathlib import Path

import paramiko

HERE = Path(__file__).resolve().parent
REMOTE_MEAS = (
    "/home/share/nsls_yyq/yyq/ITK_ECNU/lingsheng592/results/"
    "mixable100_20261025_182701/run_1/measurements.csv"
)
REMOTE_CAND = "/home/share/nsls_yyq/yyq/ITK_ECNU/lingsheng592/mixable_candidates.tsv"
CSV_PATH = HERE / "mixed_precision_lingsheng592.csv"
OUT_TSV = HERE / "qualified_mix100_343.tsv"
OUT_MD = HERE / "qualified_mix100_343.md"
OUT_MEAS = HERE / "results" / "mixable100_20261025_182701" / "run_1" / "measurements.csv"


def precise(max_abs_text: str, err: float, kind: str) -> bool:
    text = (max_abs_text or "").strip()
    p = (
        ("e" in text.lower())
        or ("." in text and len(text.split(".")[-1]) >= 6)
        or err == 0.0
        or err >= 1e-4
    )
    if not p and kind == "suite":
        return False
    return True


def qualifies(r: dict) -> bool:
    if r.get("status") != "OK":
        return False
    try:
        err = float(r["max_abs"])
        sp = float(r["speedup"])
        mf = float(r["ms_float"])
    except (TypeError, ValueError):
        return False
    if not precise(r.get("max_abs", ""), err, r.get("kind", "")):
        return False
    return err < 1e-5 and mf > 0 and sp > 1.0


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
    return f"{v:.{nd}f}".rstrip("0").rstrip(".")


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


def sftp_get_text(sftp, path: str) -> str:
    with sftp.open(path, "r") as f:
        return f.read().decode("utf-8", "replace")


def main() -> None:
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
    sftp = c.open_sftp()
    meas_text = sftp_get_text(sftp, REMOTE_MEAS)
    cand_text = sftp_get_text(sftp, REMOTE_CAND)
    sftp.close()
    c.close()

    OUT_MEAS.parent.mkdir(parents=True, exist_ok=True)
    OUT_MEAS.write_text(meas_text, encoding="utf-8", newline="\n")

    rows = list(csv.DictReader(io.StringIO(meas_text)))
    cand = {
        r["csv_name"]: r
        for r in csv.DictReader(io.StringIO(cand_text), delimiter="\t")
    }
    mod_by = {}
    if CSV_PATH.exists():
        for r in csv.DictReader(CSV_PATH.open(encoding="utf-8-sig")):
            if r.get("函数名"):
                mod_by[r["函数名"]] = r.get("模块") or "Unknown"

    qrows = [r for r in rows if qualifies(r)]
    out = []
    for r in qrows:
        key = r.get("csv_name") or r.get("operator")
        meta = cand.get(key, {})
        out.append(
            {
                "bench": meta.get("bench") or r.get("bench", ""),
                "kind": meta.get("kind") or r.get("kind", ""),
                "operator": meta.get("operator") or r.get("operator", ""),
                "csv_name": key,
                "runs": "1",
                "ms_float": r.get("ms_float", ""),
                "ms_double": r.get("ms_double", ""),
                "speedup": r.get("speedup", ""),
                "max_abs": r.get("max_abs", ""),
                "module": mod_by.get(key) or mod_by.get(key.split("(")[0], "Unknown"),
            }
        )
    out.sort(key=lambda x: (x["bench"], x["csv_name"]))

    with OUT_TSV.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(
            f,
            fieldnames=["bench", "kind", "operator", "csv_name", "runs"],
            delimiter="\t",
            lineterminator="\n",
        )
        w.writeheader()
        for r in out:
            w.writerow({k: r[k] for k in w.fieldnames})

    # markdown table
    groups: dict[str, list] = defaultdict(list)
    for r in out:
        groups[r["module"]].append(r)

    lines = [
        "# 灵昇 592 核：mixable100 双标合格函数（343）",
        "",
        "来源：作业 **1960127**（cn05128，`mixable100_20261025_182701`），"
        "对已测混精可跑接口 **一轮**、每函数内层 **100** 次计时。",
        "",
        "合格条件（同时）：",
        "",
        "- 可判定的 `max_abs < 1e-5`",
        "- `speedup = double_ms / float_ms > 1`",
        "",
        "| 口径 | 数量 |",
        "| --- | ---: |",
        f"| 候选可跑 | 468 |",
        f"| 本轮测量 OK | {sum(1 for r in rows if r.get('status')=='OK')} |",
        f"| **双标合格（本文 / `qualified_mix100_343.tsv`）** | **{len(out)}** |",
        "",
        "送测跑法：`bash submit_mix100_343.sh`（只测这 343 个）。",
        "",
        "## 按模块汇总",
        "",
        "| 模块 | 个数 |",
        "| --- | ---: |",
    ]
    for mod in sorted(groups, key=lambda m: (-len(groups[m]), m)):
        lines.append(f"| {mod} | {len(groups[mod])} |")
    lines.append(f"| **合计** | **{len(out)}** |")
    lines.append("")
    lines.append("## 分模块名单")
    lines.append("")
    for mod in sorted(groups, key=lambda m: (-len(groups[m]), m)):
        lines.append(f"### {mod}（{len(groups[mod])}）")
        lines.append("")
        lines.append("| 函数 | float_ms | double_ms | 加速比 | max_abs |")
        lines.append("| --- | ---: | ---: | ---: | --- |")
        for r in sorted(groups[mod], key=lambda x: x["csv_name"].lower()):
            lines.append(
                "| `{name}` | {mf} | {md} | {sp} | {err} |".format(
                    name=r["csv_name"],
                    mf=fmt_num(r["ms_float"]),
                    md=fmt_num(r["ms_double"]),
                    sp=fmt_num(r["speedup"]),
                    err=fmt_err(r["max_abs"]),
                )
            )
        lines.append("")

    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")
    benches = Counter(r["bench"] for r in out)
    print(f"qualify={len(out)} -> {OUT_TSV}")
    print(f"md -> {OUT_MD}")
    for b, n in benches.most_common():
        print(f"  {b}\t{n}")


if __name__ == "__main__":
    main()
