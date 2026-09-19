#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""汇总移植（双精度）和混合精度两次绑核结果，写出误差表。"""
import csv
import glob
import os
import re
import sys
from pathlib import Path

ABS_TOL = float(os.environ.get("MIXED_ABS_TOL", "1e-5"))

PIXEL_CSV = {"fill_missing"}
TIME_ONLY_CSV = {"path_mesh", "levelset", "metric", "remain", "extra35", "cat3"}

SUITE_RE = re.compile(
    r"^(?P<name>[^|]+?)\s*\|\s*ms_float=(?P<msf>[\d.eE+-]+)\s+"
    r"ms_double=(?P<msd>[\d.eE+-]+).*?"
    r"max_abs=(?P<abs>[\d.eENaN+-]+).*?rmse=(?P<rmse>[\d.eENaN+-]+)"
)
WALL_RE = re.compile(
    r"wall_ms:\s*float=(?P<msf>[\d.eE+-]+)\s+double=(?P<msd>[\d.eE+-]+)"
)
VS_RE = re.compile(
    r"vs_double:\s*max_abs=(?P<abs>[\d.eENaN+-]+)\s+rmse=(?P<rmse>[\d.eENaN+-]+)"
)
FLOAT_STOR_RE = re.compile(r"float_storage:\s+(?P<msf>[\d.eE+-]+)\s+ms")
DOUBLE_STOR_RE = re.compile(r"double_storage:\s+(?P<msd>[\d.eE+-]+)\s+ms")
DELTA_RE = re.compile(
    r"delta_metric=(?P<dm>[\d.eENaN+-]+)\s+delta_tx=(?P<dtx>[\d.eENaN+-]+)\s+delta_ty=(?P<dty>[\d.eENaN+-]+)"
)


def _f(s: str):
    if s is None:
        return None
    s = s.strip()
    if s == "" or s.upper() == "FAIL" or s.upper() == "NAN":
        return None
    try:
        return float(s)
    except ValueError:
        return None


def _fmt(v):
    if v is None:
        return ""
    if abs(v) >= 1e-3 and abs(v) < 1e6:
        return f"{v:.6g}"
    return f"{v:.8g}"


def stem_kind(path: Path):
    name = path.name
    for kind in (
        "fill_missing",
        "path_mesh",
        "levelset",
        "metric",
        "remain",
        "extra35",
        "cat3",
    ):
        if name.startswith(kind + "_"):
            return kind
    return path.stem


def parse_csv(path: Path):
    rows = []
    kind = stem_kind(path)
    with path.open(encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.lstrip("\ufeff").strip()
            if not line or line.startswith("#"):
                continue
            if line.startswith("module,operator"):
                continue
            parts = line.split(",", 6)
            if len(parts) < 5:
                continue
            module, op = parts[0].strip(), parts[1].strip()
            if op.lower() == "operator":
                continue
            fail = parts[2].strip().upper() == "FAIL" or parts[3].strip().upper() == "FAIL"
            rec = {
                "source": kind,
                "module": module,
                "operator": op,
                "ms_float": None if fail else _f(parts[2]),
                "ms_double": None if fail else _f(parts[3]),
                "speedup": None if fail or len(parts) < 5 else _f(parts[4]),
                "max_abs": None,
                "rmse": None,
                "fail": fail,
                "fail_reason": "",
                "pixel_error": kind in PIXEL_CSV,
            }
            if fail:
                rec["fail_reason"] = parts[5].strip() if len(parts) > 5 else ""
            else:
                rec["max_abs"] = _f(parts[5]) if len(parts) > 5 else None
                rec["rmse"] = _f(parts[6]) if len(parts) > 6 else None
            rows.append(rec)
    return rows


def parse_suite(path: Path, pixel_error: bool):
    rows = []
    section = "suite"
    pending_bilateral = None
    msf = msd = None
    with path.open(encoding="utf-8", errors="replace") as f:
        for raw in f:
            line = raw.lstrip("\ufeff").strip()
            if line.startswith("===== PIPELINE"):
                section = "pipeline"
                continue
            if line.startswith("===== CONV"):
                section = "conv"
                continue
            if line.startswith("===== BILATERAL"):
                section = "bilateral"
                continue
            if line.startswith("===== DIFFUSION"):
                section = "diffusion"
                continue
            if line.startswith("===== REGISTRATION"):
                section = "registration"
                msf = msd = None
                continue
            if line.startswith("====="):
                continue

            m = SUITE_RE.search(line)
            if m:
                rows.append(
                    {
                        "source": section,
                        "module": section,
                        "operator": m.group("name").strip(),
                        "ms_float": _f(m.group("msf")),
                        "ms_double": _f(m.group("msd")),
                        "speedup": None,
                        "max_abs": _f(m.group("abs")),
                        "rmse": _f(m.group("rmse")),
                        "fail": False,
                        "fail_reason": "",
                        "pixel_error": pixel_error,
                    }
                )
                continue

            m = WALL_RE.search(line)
            if m:
                pending_bilateral = {
                    "source": "bilateral",
                    "module": "bilateral",
                    "operator": "BilateralImageFilter",
                    "ms_float": _f(m.group("msf")),
                    "ms_double": _f(m.group("msd")),
                    "speedup": None,
                    "max_abs": None,
                    "rmse": None,
                    "fail": False,
                    "fail_reason": "",
                    "pixel_error": pixel_error,
                }
                continue
            m = VS_RE.search(line)
            if m and pending_bilateral is not None:
                pending_bilateral["max_abs"] = _f(m.group("abs"))
                pending_bilateral["rmse"] = _f(m.group("rmse"))
                rows.append(pending_bilateral)
                pending_bilateral = None
                continue

            m = FLOAT_STOR_RE.search(line)
            if m:
                msf = _f(m.group("msf"))
                continue
            m = DOUBLE_STOR_RE.search(line)
            if m:
                msd = _f(m.group("msd"))
                continue
            m = DELTA_RE.search(line)
            if m:
                dtx = _f(m.group("dtx")) or 0.0
                dty = _f(m.group("dty")) or 0.0
                dm = _f(m.group("dm")) or 0.0
                rows.append(
                    {
                        "source": "registration",
                        "module": "registration",
                        "operator": "ImageRegistrationMethodv4",
                        "ms_float": msf,
                        "ms_double": msd,
                        "speedup": None,
                        "max_abs": max(abs(dtx), abs(dty), abs(dm)),
                        "rmse": abs(dm),
                        "fail": False,
                        "fail_reason": f"delta_metric={dm:g}; delta_tx={dtx:g}; delta_ty={dty:g}",
                        "pixel_error": pixel_error,
                    }
                )
    if pending_bilateral is not None:
        rows.append(pending_bilateral)
    return rows


def index_rows(rows):
    by_pair = {}
    by_op = {}
    for r in rows:
        by_pair[(r["module"], r["operator"])] = r
        by_op[r["operator"]] = r
    return by_pair, by_op


def load_tag(out: Path, tag: str, pixel_suite: bool):
    rows = []
    for csv_path in sorted(out.glob(f"*_{tag}.csv")):
        rows.extend(parse_csv(csv_path))
    suite = out / f"suite_{tag}.txt"
    if suite.is_file():
        rows.extend(parse_suite(suite, pixel_suite))
    return rows


def note_for(rec, port):
    bits = []
    if rec["fail"]:
        bits.append("混精失败")
        if rec["fail_reason"]:
            bits.append(rec["fail_reason"][:80])
    if port and port["fail"]:
        bits.append("移植失败")
        if port["fail_reason"]:
            bits.append(port["fail_reason"][:80])
    if rec["source"] in TIME_ONLY_CSV:
        bits.append("该组只记时间，CSV 里 max_abs 不是像素误差")
    elif rec["pixel_error"] and not rec["fail"]:
        bits.append("float 图相对 double 图像素误差")
    if rec["source"] == "registration" and rec["fail_reason"]:
        bits.append(rec["fail_reason"])
    return "；".join(bits)


def status_for(rec, port):
    if rec["fail"]:
        return "混精FAIL"
    if port and port["fail"]:
        return "移植FAIL"
    if rec["source"] in TIME_ONLY_CSV:
        return "无像素误差"
    if rec["max_abs"] is None:
        return "无误差"
    if rec["max_abs"] > ABS_TOL:
        return f"误差>{ABS_TOL:g}"
    return "OK"


def main():
    out = Path(sys.argv[1] if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent / "results")
    out.mkdir(parents=True, exist_ok=True)

    mixed = load_tag(out, "mixed_bound", True)
    port = load_tag(out, "double_bound", False)
    if not mixed:
        print("找不到混合精度结果。请先跑: bash scripts/run_mixed.sh", file=sys.stderr)
        print(f"目录: {out}", file=sys.stderr)
        return 1
    if not port:
        print("警告: 找不到移植（双精度）结果，只汇总混合精度。建议先跑 bash scripts/run_port_double.sh")

    port_pair, port_op = index_rows(port)

    merged = []
    for rec in mixed:
        p = port_pair.get((rec["module"], rec["operator"])) or port_op.get(rec["operator"])
        sp = rec["speedup"]
        if sp is None and rec["ms_float"] and rec["ms_float"] > 0 and rec["ms_double"] is not None:
            sp = rec["ms_double"] / rec["ms_float"]
        merged.append(
            {
                "来源": rec["source"],
                "模块": rec["module"],
                "函数": rec["operator"],
                "ms_float": rec["ms_float"],
                "ms_double_混精": rec["ms_double"],
                "ms_double_移植": None if p is None else p["ms_double"],
                "加速比": sp,
                "max_abs": rec["max_abs"],
                "rmse": rec["rmse"],
                "状态": status_for(rec, p),
                "备注": note_for(rec, p),
            }
        )

    csv_path = out / "mixed_error_summary.csv"
    txt_path = out / "mixed_error_summary.txt"
    fields = [
        "来源",
        "模块",
        "函数",
        "ms_float",
        "ms_double_混精",
        "ms_double_移植",
        "加速比",
        "max_abs",
        "rmse",
        "状态",
        "备注",
    ]
    with csv_path.open("w", encoding="utf-8-sig", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        for row in merged:
            out_row = dict(row)
            for k in ("ms_float", "ms_double_混精", "ms_double_移植", "加速比", "max_abs", "rmse"):
                out_row[k] = _fmt(row[k])
            w.writerow(out_row)

    pixel = [r for r in merged if r["状态"] not in ("无像素误差", "混精FAIL", "移植FAIL") and r["max_abs"] is not None]
    over = [r for r in pixel if r["max_abs"] > ABS_TOL]
    fail_m = [r for r in merged if r["状态"] == "混精FAIL"]
    fail_p = [r for r in merged if r["状态"] == "移植FAIL"]
    ok = [r for r in pixel if r["max_abs"] <= ABS_TOL]
    no_pix = [r for r in merged if r["状态"] == "无像素误差"]

    lines = [
        f"混合精度误差汇总  tol={ABS_TOL:g}",
        f"目录: {out}",
        f"总条数: {len(merged)}",
        f"移植失败: {len(fail_p)}",
        f"混精失败: {len(fail_m)}",
        f"有像素对比: {len(pixel)}",
        f"  max_abs <= {ABS_TOL:g}: {len(ok)}",
        f"  max_abs >  {ABS_TOL:g}: {len(over)}",
        f"只记时间、无像素误差: {len(no_pix)}",
        "",
        f"明细 CSV: {csv_path}",
        "",
    ]
    if over:
        over_sorted = sorted(over, key=lambda r: r["max_abs"], reverse=True)
        lines.append(f"误差超过 {ABS_TOL:g} 的函数（按 max_abs 从大到小）:")
        for r in over_sorted:
            lines.append(
                f"  {r['函数']}\tmax_abs={_fmt(r['max_abs'])}\trmse={_fmt(r['rmse'])}\t来源={r['来源']}"
            )
        lines.append("")
    if fail_m:
        lines.append("混精 FAIL:")
        for r in fail_m:
            lines.append(f"  {r['函数']}\t{r['备注']}")
        lines.append("")
    txt_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("\n".join(lines))
    return 0


if __name__ == "__main__":
    sys.exit(main())
