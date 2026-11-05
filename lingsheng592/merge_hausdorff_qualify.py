#!/usr/bin/env python3
"""Merge Hausdorff-wave dual-qualify ops into delivery list (343 + unique new).

Does NOT include the opt94 noise-only flips. Ops already in the 343 baseline
are skipped so the delivery count is 343 + unique newcomers.
"""
from __future__ import annotations

import csv
import io
import os
from collections import Counter, defaultdict
from pathlib import Path

import paramiko

HERE = Path(__file__).resolve().parent
BASE_TSV = HERE / "qualified_mix100_343.tsv"
CAND_TSV = HERE / "optimize_after_hausdorff_candidates.tsv"
CSV_PATH = HERE / "mixed_precision_lingsheng592.csv"
# filled after merge; filenames follow final count
OUT_TSV = HERE / "qualified_mix100_355.tsv"
OUT_MD = HERE / "qualified_mix100_355.md"
REMOTE_HAUS = (
    "/home/share/nsls_yyq/yyq/ITK_ECNU/lingsheng592/results/"
    "optimize_hausdorff_20261104_234046/run_1/measurements.csv"
)
REMOTE_BASE = (
    "/home/share/nsls_yyq/yyq/ITK_ECNU/lingsheng592/results/"
    "mixable100_20261025_182701/run_1/measurements.csv"
)


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
    base = {
        r["csv_name"]: r
        for r in csv.DictReader(BASE_TSV.open(encoding="utf-8"), delimiter="\t")
    }
    cand = {
        r["csv_name"]: r
        for r in csv.DictReader(CAND_TSV.open(encoding="utf-8"), delimiter="\t")
    }
    mod_by = {}
    if CSV_PATH.exists():
        for r in csv.DictReader(CSV_PATH.open(encoding="utf-8-sig")):
            if r.get("函数名"):
                mod_by[r["函数名"]] = r.get("模块") or "Unknown"

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
    haus_text = sftp_get_text(sftp, REMOTE_HAUS)
    base_meas_text = sftp_get_text(sftp, REMOTE_BASE)
    sftp.close()
    c.close()

    haus_rows = list(csv.DictReader(io.StringIO(haus_text)))
    base_meas = {
        (r.get("csv_name") or r.get("operator")): r
        for r in csv.DictReader(io.StringIO(base_meas_text))
    }

    wave_dual = []
    already_in_base = []
    added = []
    for r in haus_rows:
        key = r.get("csv_name") or r.get("operator")
        if not qualifies(r):
            continue
        wave_dual.append(key)
        if key in base:
            already_in_base.append(key)
            continue
        meta = cand.get(key, {})
        added.append(
            {
                "bench": meta.get("bench") or r.get("bench", ""),
                "kind": meta.get("kind") or r.get("kind", "list"),
                "operator": meta.get("operator") or r.get("operator") or key,
                "csv_name": key,
                "runs": "1",
                "ms_float": r.get("ms_float", ""),
                "ms_double": r.get("ms_double", ""),
                "speedup": r.get("speedup", ""),
                "max_abs": r.get("max_abs", ""),
                "module": mod_by.get(key) or mod_by.get(key.split("(")[0], "Unknown"),
                "source": "hausdorff_wave",
            }
        )
    added.sort(key=lambda x: x["csv_name"].lower())
    if len(wave_dual) != 13:
        raise SystemExit(
            f"expected 13 wave dual-qualifies, got {len(wave_dual)}: {wave_dual}"
        )
    print(
        f"wave_dual={len(wave_dual)} already_in_343={already_in_base} "
        f"unique_new={len(added)}"
    )

    out = []
    for key, meta in base.items():
        m = base_meas.get(key, {})
        out.append(
            {
                "bench": meta["bench"],
                "kind": meta["kind"],
                "operator": meta["operator"],
                "csv_name": key,
                "runs": "1",
                "ms_float": m.get("ms_float", ""),
                "ms_double": m.get("ms_double", ""),
                "speedup": m.get("speedup", ""),
                "max_abs": m.get("max_abs", ""),
                "module": mod_by.get(key) or mod_by.get(key.split("(")[0], "Unknown"),
                "source": "mixable100_343",
            }
        )
    out.extend(added)
    out.sort(key=lambda x: (x["bench"], x["csv_name"]))

    groups: dict[str, list] = defaultdict(list)
    for r in out:
        groups[r["module"]].append(r)

    n = len(out)
    out_tsv = HERE / f"qualified_mix100_{n}.tsv"
    out_md = HERE / f"qualified_mix100_{n}.md"
    out_tsv_name = out_tsv.name
    lines = [
        f"# 灵昇 592 核：当前送测名单（{n}）",
        "",
        f"**本文是当前送测主口径。** 在 mixable100 双标 **343** 基础上，并入 Hausdorff float 累加复测"
        f"（作业 **2026986**，`optimize_hausdorff_20261104_234046`）中新达标且原先不在 343 内的 **{len(added)}** 个"
        f"（该波共双标 13 个，其中已在 343 内：{', '.join(f'`{x}`' for x in already_in_base) or '无'}）。"
        "不含 opt94 仅噪声翻盘的 19 个。",
        "",
        "合格条件（同时）：",
        "",
        "- 可判定的 `max_abs < 1e-5`",
        "- `speedup = double_ms / float_ms > 1`",
        "",
        "| 口径 | 数量 |",
        "| --- | ---: |",
        "| mixable100 双标基线 | 343 |",
        f"| 本波复测双标 | 13 |",
        f"| 其中已在 343 | {len(already_in_base)} |",
        f"| 净新增（不含 noise-19） | {len(added)} |",
        f"| **当前送测 / 双标合格（本文 / `{out_tsv_name}`）** | **{n}** |",
        "",
        f"其他使用者重测这 {n} 个：见仓库 `lingsheng592/README.md`「推荐测试流程」，或直接：",
        "",
        "```bash",
        "cd .../lingsheng592",
        f"bash submit_mix100_{n}.sh",
        "```",
        "",
        f"## 本波双标 13（净新增 {len(added)}）",
        "",
        "| 函数 | float_ms | double_ms | 加速比 | max_abs | 模块 | 相对 343 |",
        "| --- | ---: | ---: | ---: | --- | --- | --- |",
    ]
    wave_rows = []
    for r in haus_rows:
        key = r.get("csv_name") or r.get("operator")
        if key not in wave_dual:
            continue
        wave_rows.append(
            {
                "csv_name": key,
                "ms_float": r.get("ms_float", ""),
                "ms_double": r.get("ms_double", ""),
                "speedup": r.get("speedup", ""),
                "max_abs": r.get("max_abs", ""),
                "module": mod_by.get(key) or "Unknown",
                "flag": "已在 343" if key in already_in_base else "新增",
            }
        )
    wave_rows.sort(key=lambda x: -float(x["speedup"] or 0))
    for row in wave_rows:
        lines.append(
            "| `{name}` | {mf} | {md} | {sp} | {err} | {mod} | {flag} |".format(
                name=row["csv_name"],
                mf=fmt_num(row["ms_float"]),
                md=fmt_num(row["ms_double"]),
                sp=fmt_num(row["speedup"]),
                err=fmt_err(row["max_abs"]),
                mod=row["module"],
                flag=row["flag"],
            )
        )
    lines += [
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

    with out_tsv.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(
            f,
            fieldnames=["bench", "kind", "operator", "csv_name", "runs"],
            delimiter="\t",
            lineterminator="\n",
        )
        w.writeheader()
        for r in out:
            w.writerow({k: r[k] for k in w.fieldnames})
    out_md.write_text("\n".join(lines) + "\n", encoding="utf-8")
    for stale in (OUT_TSV, OUT_MD):
        if stale not in (out_tsv, out_md) and stale.exists():
            stale.unlink(missing_ok=True)
    benches = Counter(r["bench"] for r in out)
    print(f"base={len(base)} added={len(added)} total={n} -> {out_tsv}")
    print(f"md -> {out_md}")
    for r in added:
        print(f"  + {r['csv_name']}\tspeedup={r['speedup']}")
    for b, cnt in benches.most_common():
        print(f"  {b}\t{cnt}")


if __name__ == "__main__":
    main()
