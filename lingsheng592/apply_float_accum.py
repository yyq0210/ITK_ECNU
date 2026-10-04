#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Bake Kunpeng float-accumulation into the Lingsheng ITK tree, behind a CMake switch."""
import os
from pathlib import Path

SRC = Path(os.environ.get("ITK_SRC", "/tmp/itk-src/InsightToolkit-5.4.0"))
INC = Path(os.environ.get("ITK_INC", "/home/share/nsls_yyq/yyq/ITK-5.4.0-static/install/include/ITK-5.4"))
BUILD = Path(os.environ.get("ITK_BUILD", "/tmp/itk-static"))
BUILD_CONF = BUILD / "Modules/Core/Common/itkConfigure.h"

ACCUM_MACRO = r"""
// float pixels accumulate in float when this option is ON. double pixels stay on RealType.
#cmakedefine ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS
#ifdef ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS
#define ITK_ACCUMULATION_TYPE(PixelType)                                                                 \
  typename std::conditional<std::is_same<PixelType, float>::value,                                       \
                            typename NumericTraits<PixelType>::FloatType,                                \
                            typename NumericTraits<PixelType>::RealType>::type
#else
#define ITK_ACCUMULATION_TYPE(PixelType) typename NumericTraits<PixelType>::RealType
#endif
"""

ACCUM_MACRO_DEFINED = ACCUM_MACRO.replace("#cmakedefine ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS",
                                          "#define ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS")

CMAKE_OPTION = '''
# float-pixel filters accumulate in float. double pixels keep RealType.
# OFF restores upstream double accumulation for float pixels.
option(ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS
  "Use float internal accumulation when the pixel type is float" ON)
mark_as_advanced(ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS)
'''

GEN_DATA = r'''
#ifdef ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS
  /* kunpeng: float image I/O, iterate in double, write back float */
  void
  GenerateData() override
  {{
    using OutputPixelType = typename TOutputImage::PixelType;
    if (!std::is_same<OutputPixelType, float>::value)
    {{
      Superclass::GenerateData();
      return;
    }}

    constexpr unsigned int Dimension = ImageDimension;
    using DImage = Image<double, Dimension>;
    using CastInType = CastImageFilter<TInputImage, DImage>;
    using DoubleSelf = {cls}<DImage, DImage>;
    using CastOutType = CastImageFilter<DImage, TOutputImage>;

    typename CastInType::Pointer castIn = CastInType::New();
    castIn->SetInput(this->GetInput());
    castIn->Update();

    typename DoubleSelf::Pointer inner = DoubleSelf::New();
    inner->SetInput(castIn->GetOutput());
    inner->SetNumberOfIterations(this->GetNumberOfIterations());
    inner->SetTimeStep(static_cast<typename DoubleSelf::TimeStepType>(this->GetTimeStep()));
    inner->SetConductanceParameter(this->GetConductanceParameter());
    inner->SetConductanceScalingParameter(this->GetConductanceScalingParameter());
    inner->SetConductanceScalingUpdateInterval(this->GetConductanceScalingUpdateInterval());
    inner->SetUseImageSpacing(this->GetUseImageSpacing());
    inner->SetNumberOfWorkUnits(this->GetNumberOfWorkUnits());
    if (this->m_GradientMagnitudeIsFixed)
    {{
      inner->SetFixedAverageGradientMagnitude(this->GetFixedAverageGradientMagnitude());
    }}
    inner->Update();

    typename CastOutType::Pointer castOut = CastOutType::New();
    castOut->SetInput(inner->GetOutput());
    castOut->GraftOutput(this->GetOutput());
    castOut->Update();
    this->GraftOutput(castOut->GetOutput());
  }}
#endif
'''


def must_replace(path: Path, old: str, new: str, count=1):
    text = path.read_text(encoding="utf-8")
    if old not in text:
        if new in text:
            print(f"already: {path.name}")
            return
        raise SystemExit(f"{path}: expected >= {count} of {old!r}, found 0")
    found = text.count(old)
    if found < count:
        raise SystemExit(f"{path}: expected >= {count} of {old!r}, found {found}")
    if count == 1:
        text = text.replace(old, new, 1)
    else:
        text = text.replace(old, new)
    path.write_text(text, encoding="utf-8")
    print(f"replaced x{found if count != 1 else 1}: {path.name}")


def ensure_type_traits(path: Path):
    text = path.read_text(encoding="utf-8")
    if "#include <type_traits>" in text:
        return
    key = "#include "
    i = text.find(key)
    if i < 0:
        raise SystemExit(f"no include in {path}")
    line_end = text.find("\n", i)
    text = text[: line_end + 1] + "#include <type_traits>\n" + text[line_end + 1 :]
    path.write_text(text, encoding="utf-8")
    print(f"type_traits: {path.name}")


def patch_filters(root: Path):
    feat = root / "Modules/Filtering/ImageFeature/include"
    smooth = root / "Modules/Filtering/Smoothing/include"
    aniso = root / "Modules/Filtering/AnisotropicSmoothing/include"

    b_h = feat / "itkBilateralImageFilter.h"
    ensure_type_traits(b_h)
    must_replace(
        b_h,
        "using OutputPixelRealType = typename NumericTraits<OutputPixelType>::RealType;",
        "using OutputPixelRealType = ITK_ACCUMULATION_TYPE(OutputPixelType);",
    )
    must_replace(
        b_h,
        "using KernelType = Neighborhood<double, Self::ImageDimension>;",
        "using KernelType = Neighborhood<OutputPixelRealType, Self::ImageDimension>;",
    )
    must_replace(
        b_h,
        "using GaussianImageType = Image<double, Self::ImageDimension>;",
        "using GaussianImageType = Image<OutputPixelRealType, Self::ImageDimension>;",
    )
    must_replace(b_h, "  double              m_DynamicRange{};", "  OutputPixelRealType m_DynamicRange{};")
    must_replace(b_h, "  double              m_DynamicRangeUsed{};", "  OutputPixelRealType m_DynamicRangeUsed{};")
    must_replace(
        b_h,
        "  std::vector<double> m_RangeGaussianTable{};",
        "  std::vector<OutputPixelRealType> m_RangeGaussianTable{};",
    )

    b_hxx = feat / "itkBilateralImageFilter.hxx"
    pairs = [
        ("  double                                 norm = 0.0;", "  OutputPixelRealType                    norm = 0.0;"),
        ("  double rangeVariance = m_RangeSigma * m_RangeSigma;",
         "  OutputPixelRealType rangeVariance = static_cast<OutputPixelRealType>(m_RangeSigma * m_RangeSigma);"),
        ("  double rangeGaussianDenom;", "  OutputPixelRealType rangeGaussianDenom;"),
        ("  rangeGaussianDenom = m_RangeSigma * std::sqrt(2.0 * itk::Math::pi);",
         "  rangeGaussianDenom = static_cast<OutputPixelRealType>(m_RangeSigma) * static_cast<OutputPixelRealType>(std::sqrt(2.0 * itk::Math::pi));"),
        ("  double tableDelta;", "  OutputPixelRealType tableDelta;"),
        ("  double v;", "  OutputPixelRealType v;"),
        ("  m_DynamicRange = (static_cast<double>(statistics->GetMaximum()) - static_cast<double>(statistics->GetMinimum()));",
         "  m_DynamicRange = static_cast<OutputPixelRealType>(statistics->GetMaximum()) - static_cast<OutputPixelRealType>(statistics->GetMinimum());"),
        ("  m_DynamicRangeUsed = m_RangeMu * m_RangeSigma;",
         "  m_DynamicRangeUsed = static_cast<OutputPixelRealType>(m_RangeMu * m_RangeSigma);"),
        ("  tableDelta = m_DynamicRangeUsed / static_cast<double>(m_NumberOfRangeGaussianSamples);",
         "  tableDelta = m_DynamicRangeUsed / static_cast<OutputPixelRealType>(m_NumberOfRangeGaussianSamples);"),
        ("  const double                         rangeDistanceThreshold = m_DynamicRangeUsed;",
         "  const OutputPixelRealType            rangeDistanceThreshold = m_DynamicRangeUsed;"),
        ("  const double distanceToTableIndex = static_cast<double>(m_NumberOfRangeGaussianSamples) / m_DynamicRangeUsed;",
         "  const OutputPixelRealType distanceToTableIndex = static_cast<OutputPixelRealType>(m_NumberOfRangeGaussianSamples) / m_DynamicRangeUsed;"),
    ]
    for old, new in pairs:
        must_replace(b_hxx, old, new)

    mean = smooth / "itkMeanImageFilter.h"
    ensure_type_traits(mean)
    must_replace(
        mean,
        "using InputRealType = typename NumericTraits<InputPixelType>::RealType;",
        "using InputRealType = ITK_ACCUMULATION_TYPE(InputPixelType);",
    )
    dg = smooth / "itkDiscreteGaussianImageFilter.h"
    ensure_type_traits(dg)
    must_replace(
        dg,
        "using RealOutputPixelType = typename NumericTraits<OutputPixelType>::RealType;",
        "using RealOutputPixelType = ITK_ACCUMULATION_TYPE(OutputPixelType);",
    )
    box = smooth / "itkBoxUtilities.h"
    ensure_type_traits(box)
    must_replace(
        box,
        "using AccPixType = typename NumericTraits<OutputPixelType>::RealType;",
        "using AccPixType = ITK_ACCUMULATION_TYPE(OutputPixelType);",
        count=2,
    )

    for cls, name in (
        ("CurvatureAnisotropicDiffusionImageFilter", "itkCurvatureAnisotropicDiffusionImageFilter.h"),
        ("GradientAnisotropicDiffusionImageFilter", "itkGradientAnisotropicDiffusionImageFilter.h"),
    ):
        path = aniso / name
        text = path.read_text(encoding="utf-8")
        if "kunpeng: float image I/O" in text:
            print("skip", name)
            continue
        key = '#include "itkAnisotropicDiffusionImageFilter.h"'
        extra = '\n#include "itkCastImageFilter.h"\n#include "itkImage.h"\n#include <type_traits>\n'
        if key not in text:
            raise SystemExit(f"missing include in {name}")
        text = text.replace(key, key + extra, 1)
        anchor = f"  ~{cls}() override = default;"
        if anchor not in text:
            raise SystemExit(f"missing dtor in {name}")
        text = text.replace(anchor, anchor + "\n" + GEN_DATA.format(cls=cls), 1)
        path.write_text(text, encoding="utf-8")
        print("patched", name)


def patch_switch():
    cmake = SRC / "CMakeLists.txt"
    text = cmake.read_text(encoding="utf-8")
    if "ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS" not in text:
        anchor = 'option(ITK_USE_FLOAT_SPACE_PRECISION "Use single precision for origin/spacing/directions in itk::Image" OFF)\nmark_as_advanced(ITK_USE_FLOAT_SPACE_PRECISION)\n'
        if anchor not in text:
            raise SystemExit("cmake anchor missing")
        text = text.replace(anchor, anchor + CMAKE_OPTION, 1)
        cmake.write_text(text, encoding="utf-8")
        print("cmake option added")
    conf_in = SRC / "Modules/Core/Common/src/itkConfigure.h.in"
    text = conf_in.read_text(encoding="utf-8")
    if "ITK_ACCUMULATION_TYPE" not in text:
        anchor = "#cmakedefine ITK_USE_FLOAT_SPACE_PRECISION\n"
        if anchor not in text:
            raise SystemExit("configure.in anchor missing")
        text = text.replace(anchor, anchor + "\n" + ACCUM_MACRO, 1)
        conf_in.write_text(text, encoding="utf-8")
        print("configure.in updated")
    for path in (BUILD_CONF, INC / "itkConfigure.h"):
        if not path.exists():
            print("skip missing", path)
            continue
        text = path.read_text(encoding="utf-8")
        if "ITK_ACCUMULATION_TYPE" in text:
            print("configure already", path)
            continue
        anchor = "/* #undef ITK_USE_FLOAT_SPACE_PRECISION */\n"
        if anchor not in text:
            anchor = "#cmakedefine ITK_USE_FLOAT_SPACE_PRECISION\n"
        if anchor not in text:
            raise SystemExit(f"configure anchor missing: {path}")
        text = text.replace(anchor, anchor + "\n" + ACCUM_MACRO_DEFINED, 1)
        path.write_text(text, encoding="utf-8")
        print("define added", path)
    cache = BUILD / "CMakeCache.txt"
    if not cache.exists():
        print("skip missing", cache)
        return
    ctext = cache.read_text(encoding="utf-8")
    if "ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS" not in ctext:
        ctext += "\nITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS:BOOL=ON\n"
        cache.write_text(ctext, encoding="utf-8")
        print("cache flag added")


def install_headers():
    if not INC.is_dir():
        print("skip install copy; include dir missing", INC)
        return
    mapping = {
        SRC / "Modules/Filtering/ImageFeature/include/itkBilateralImageFilter.h": INC / "itkBilateralImageFilter.h",
        SRC / "Modules/Filtering/ImageFeature/include/itkBilateralImageFilter.hxx": INC / "itkBilateralImageFilter.hxx",
        SRC / "Modules/Filtering/Smoothing/include/itkMeanImageFilter.h": INC / "itkMeanImageFilter.h",
        SRC / "Modules/Filtering/Smoothing/include/itkDiscreteGaussianImageFilter.h": INC / "itkDiscreteGaussianImageFilter.h",
        SRC / "Modules/Filtering/Smoothing/include/itkBoxUtilities.h": INC / "itkBoxUtilities.h",
        SRC / "Modules/Filtering/AnisotropicSmoothing/include/itkCurvatureAnisotropicDiffusionImageFilter.h": INC / "itkCurvatureAnisotropicDiffusionImageFilter.h",
        SRC / "Modules/Filtering/AnisotropicSmoothing/include/itkGradientAnisotropicDiffusionImageFilter.h": INC / "itkGradientAnisotropicDiffusionImageFilter.h",
    }
    for src, dst in mapping.items():
        dst.write_text(src.read_text(encoding="utf-8"), encoding="utf-8")
        print("installed", dst.name)


def main():
    patch_switch()
    patch_filters(SRC)
    install_headers()
    print("PATCH_OK")


if __name__ == "__main__":
    main()
