#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""将鲲鹏 float 累加优化合入 ITK 5.4 源码树（交付烘焙，无 patch 脚本依赖）。"""
import argparse
import re
import shutil
import sys
import tarfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent

README_KUNPENG = """\
# ITK 5.4.0 — 华为鲲鹏（ARM64）优化版

基于 Insight Toolkit 5.4.0，在华为鲲鹏 920 平台完成全模块 CPU 移植与混合精度算子层优化。

## 相对 upstream 5.4.0 的主要变更

- 热点 Filter（Bilateral、Mean、DiscreteGaussian、Box 均值等）在 **float 像素** 路径下采用 **float 内部累加**，以启用 NEON 向量化并降低计算开销。
- **CAD/GAD 等有限差分扩散** 在 float 像素时 **内部用 double 迭代**，再写回 float，避免 50 步误差放大。
- 应用层推荐 `Image<float>` 用于滤波/分割；配准场景建议 float 影像 + double 度量/变换。

## 编译（鲲鹏 920，Release）

```bash
mkdir build && cd build
cmake .. \\
  -DCMAKE_BUILD_TYPE=Release \\
  -DCMAKE_TOOLCHAIN_FILE=../CMake/KunpengToolchain.cmake \\
  -DITK_BUILD_DEFAULT_MODULES=ON
cmake --build . -j$(nproc)
```

或手动指定：

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release \\
  -DCMAKE_CXX_FLAGS="-O3 -march=armv8.2-a+crypto" \\
  -DITK_BUILD_DEFAULT_MODULES=ON
```

## 说明

- 本包为 **完整 ITK 源码**，优化已合入，**无需**额外补丁或 apply 脚本。
- GPU/OpenCL 模块在 CPU 节点上不启用，以 CPU 等价 Filter 交付。
- 功能与性能验收见配套 `ITK-Kunpeng-Validation`。

## 许可

遵循 ITK 原有 BSD 风格开源许可，见 `LICENSE` / `NOTICE`。
"""


def rel(*parts):
    return str(Path(*parts))


# (relative_path, old, new) — 按顺序应用；已合入则跳过
REPLACEMENTS = [
    # --- batch 1: core float accum typedef ---
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.h"),
        "using OutputPixelRealType = typename NumericTraits<OutputPixelType>::RealType;",
        "using OutputPixelRealType = typename NumericTraits<OutputPixelType>::FloatType;",
    ),
    (
        rel("Modules", "Filtering", "Smoothing", "include", "itkMeanImageFilter.h"),
        "using InputRealType = typename NumericTraits<InputPixelType>::RealType;",
        "using InputRealType = typename NumericTraits<InputPixelType>::FloatType;",
    ),
    (
        rel("Modules", "Filtering", "Smoothing", "include", "itkDiscreteGaussianImageFilter.h"),
        "using RealOutputPixelType = typename NumericTraits<OutputPixelType>::RealType;",
        "using RealOutputPixelType = typename NumericTraits<OutputPixelType>::FloatType;",
    ),
    # --- batch 2: bilateral full float path ---
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.h"),
        "using KernelType = Neighborhood<double, Self::ImageDimension>;",
        "using KernelType = Neighborhood<OutputPixelRealType, Self::ImageDimension>;",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.h"),
        "using GaussianImageType = Image<double, Self::ImageDimension>;",
        "using GaussianImageType = Image<OutputPixelRealType, Self::ImageDimension>;",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.h"),
        "  double              m_DynamicRange{};",
        "  OutputPixelRealType m_DynamicRange{};",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.h"),
        "  double              m_DynamicRangeUsed{};",
        "  OutputPixelRealType m_DynamicRangeUsed{};",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.h"),
        "  std::vector<double> m_RangeGaussianTable{};",
        "  std::vector<OutputPixelRealType> m_RangeGaussianTable{};",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.hxx"),
        "  double                                 norm = 0.0;",
        "  OutputPixelRealType                    norm = 0.0;",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.hxx"),
        "  double rangeVariance = m_RangeSigma * m_RangeSigma;",
        "  OutputPixelRealType rangeVariance = static_cast<OutputPixelRealType>(m_RangeSigma * m_RangeSigma);",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.hxx"),
        "  double rangeGaussianDenom;",
        "  OutputPixelRealType rangeGaussianDenom;",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.hxx"),
        "  rangeGaussianDenom = m_RangeSigma * std::sqrt(2.0 * itk::Math::pi);",
        "  rangeGaussianDenom = static_cast<OutputPixelRealType>(m_RangeSigma) * static_cast<OutputPixelRealType>(std::sqrt(2.0 * itk::Math::pi));",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.hxx"),
        "  double tableDelta;",
        "  OutputPixelRealType tableDelta;",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.hxx"),
        "  double v;",
        "  OutputPixelRealType v;",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.hxx"),
        "  m_DynamicRange = (static_cast<double>(statistics->GetMaximum()) - static_cast<double>(statistics->GetMinimum()));",
        "  m_DynamicRange = static_cast<OutputPixelRealType>(statistics->GetMaximum()) - static_cast<OutputPixelRealType>(statistics->GetMinimum());",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.hxx"),
        "  m_DynamicRangeUsed = m_RangeMu * m_RangeSigma;",
        "  m_DynamicRangeUsed = static_cast<OutputPixelRealType>(m_RangeMu * m_RangeSigma);",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.hxx"),
        "  tableDelta = m_DynamicRangeUsed / static_cast<double>(m_NumberOfRangeGaussianSamples);",
        "  tableDelta = m_DynamicRangeUsed / static_cast<OutputPixelRealType>(m_NumberOfRangeGaussianSamples);",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.hxx"),
        "  const double                         rangeDistanceThreshold = m_DynamicRangeUsed;",
        "  const OutputPixelRealType            rangeDistanceThreshold = m_DynamicRangeUsed;",
    ),
    (
        rel("Modules", "Filtering", "ImageFeature", "include", "itkBilateralImageFilter.hxx"),
        "  const double distanceToTableIndex = static_cast<double>(m_NumberOfRangeGaussianSamples) / m_DynamicRangeUsed;",
        "  const OutputPixelRealType distanceToTableIndex = static_cast<OutputPixelRealType>(m_NumberOfRangeGaussianSamples) / m_DynamicRangeUsed;",
    ),
    # --- batch 3: BoxMean only（CAD/GAD 有限差分保持 double，避免迭代误差放大）---
    (
        rel("Modules", "Filtering", "Smoothing", "include", "itkBoxUtilities.h"),
        "using AccPixType = typename NumericTraits<OutputPixelType>::RealType;",
        "using AccPixType = typename NumericTraits<OutputPixelType>::FloatType;",
    ),
]

DEV_MARKER_RE = re.compile(r" /\* yyq: [^*]* \*/")


def strip_dev_markers(text):
    return DEV_MARKER_RE.sub("", text)


def apply_replacements(itk_root):
    log = []
    for relpath, old, new in REPLACEMENTS:
        path = itk_root / relpath
        if not path.is_file():
            raise FileNotFoundError(f"missing ITK file: {path}")
        text = path.read_text(encoding="utf-8")
        text = strip_dev_markers(text)
        if old not in text:
            if new in text:
                log.append(f"skip (already applied): {relpath}")
                continue
            raise ValueError(f"pattern not found in {relpath}:\n  {old[:80]}...")
        text = text.replace(old, new, 1)
        path.write_text(text, encoding="utf-8")
        log.append(f"applied: {relpath}")

    return log


def strip_all_dev_markers(itk_root):
    """Remove development markers from all copied sources."""
    count = 0
    modules = itk_root / "Modules"
    if not modules.is_dir():
        return count
    for path in modules.rglob("*"):
        if path.suffix not in (".h", ".hxx", ".cxx", ".cpp"):
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        if "yyq:" not in text:
            continue
        cleaned = strip_dev_markers(text)
        if cleaned != text:
            path.write_text(cleaned, encoding="utf-8")
            count += 1
    return count


def copy_itk_tree(src, dst):
    ignore = shutil.ignore_patterns(
        "build*",
        ".git",
        "CMakeCache.txt",
        "CMakeFiles",
        "__pycache__",
    )

    if dst.exists():
        shutil.rmtree(dst)
    shutil.copytree(src, dst, ignore=ignore)


def install_extras(itk_root):
    (itk_root / "README-KUNPENG.md").write_text(README_KUNPENG, encoding="utf-8")
    toolchain_src = ROOT / "cmake" / "KunpengToolchain.cmake"
    toolchain_dst = itk_root / "CMake" / "KunpengToolchain.cmake"
    toolchain_dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(toolchain_src, toolchain_dst)


def make_tarball(source_dir, tarball):
    with tarfile.open(tarball, "w:gz") as tar:
        tar.add(source_dir, arcname=source_dir.name)


def prepare_validation(out_dir):
    src = ROOT / "ITK-Kunpeng-Validation"
    if out_dir.exists():
        shutil.rmtree(out_dir)
    shutil.copytree(src, out_dir)


def main():
    parser = argparse.ArgumentParser(description="Bake Kunpeng float-accum optimizations into ITK 5.4 source.")
    parser.add_argument("itk_src", type=Path, help="ITK 5.4 source tree (upstream or dev copy)")
    parser.add_argument(
        "out_dir",
        type=Path,
        nargs="?",
        help="Output directory; omit with --in-place to modify itk_src directly",
    )
    parser.add_argument("--in-place", action="store_true", help="Modify itk_src in place (dev only)")
    parser.add_argument("-t", "--tarball", action="store_true", help="Create .tar.gz next to out_dir")
    parser.add_argument(
        "--validation-dir",
        type=Path,
        help="Also emit ITK-Kunpeng-Validation to this directory",
    )
    args = parser.parse_args()

    itk_src = args.itk_src.resolve()
    if not (itk_src / "Modules").is_dir():
        print(f"ERROR: not an ITK source tree: {itk_src}", file=sys.stderr)
        return 1

    if args.in_place:
        target = itk_src
        print(f"=== apply Kunpeng optimizations (in-place): {target} ===")
    else:
        if not args.out_dir:
            print("ERROR: out_dir required unless --in-place", file=sys.stderr)
            return 1
        target = args.out_dir.resolve()
        print(f"=== prepare Kunpeng ITK release ===")
        print(f"ITK_SRC={itk_src}")
        print(f"OUT_DIR={target}")
        copy_itk_tree(itk_src, target)

    for line in apply_replacements(target):
        print(line)
    try:
        from apply_cad_gad_double_iterate import patch_tree as patch_cad_gad
    except ImportError:
        sys.path.insert(0, str(ROOT))
        from apply_cad_gad_double_iterate import patch_tree as patch_cad_gad
    for line in patch_cad_gad(target):
        print(line)
    stripped = strip_all_dev_markers(target)
    if stripped:
        print(f"stripped dev markers from {stripped} file(s)")
    install_extras(target)
    print(f"wrote: {target / 'README-KUNPENG.md'}")
    print(f"wrote: {target / 'CMake' / 'KunpengToolchain.cmake'}")

    if args.tarball and not args.in_place:
        tarball = target.parent / f"{target.name}.tar.gz"
        make_tarball(target, tarball)
        print(f"tarball: {tarball}")

    if args.validation_dir:
        prepare_validation(args.validation_dir.resolve())
        print(f"validation: {args.validation_dir}")

    print("=== done ===")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
