#!/usr/bin/env python3
# 若缺少 1024 图，从官方切片拉成 1024×1024。
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DATA = ROOT / "data"

PAIRS = [
    ("BrainProtonDensitySlice.png", "BrainProtonDensity1024.png"),
    ("BrainProtonDensitySliceBorder20.png", "BrainProtonDensity1024_fixed.png"),
    ("BrainProtonDensitySliceShifted13x17y.png", "BrainProtonDensity1024_moving.png"),
]


def main() -> None:
    try:
        from PIL import Image
    except ImportError:
        missing = [DATA / d for _, d in PAIRS if not (DATA / d).exists()]
        if missing:
            print("需要 Pillow，或事先放入 1024 图：", missing)
            sys.exit(1)
        print("1024 图已存在，跳过")
        return

    try:
        resample = Image.Resampling.LANCZOS
    except AttributeError:
        resample = Image.LANCZOS

    for src_name, dst_name in PAIRS:
        src = DATA / src_name
        dst = DATA / dst_name
        if not src.exists():
            print("缺少官方图", src)
            sys.exit(1)
        if dst.exists():
            print("keep", dst.name)
            continue
        im = Image.open(src).convert("L").resize((1024, 1024), resample)
        im.save(dst)
        print("wrote", dst.name, im.size)


if __name__ == "__main__":
    main()
