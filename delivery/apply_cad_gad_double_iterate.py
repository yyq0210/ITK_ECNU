#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""CAD/GAD：float 图像仍走 float I/O，迭代在 double 缓冲中完成后再写回。"""
from pathlib import Path
import argparse
import sys

CAD_INCLUDES = """
#include "itkCastImageFilter.h"
#include "itkImage.h"
#include <type_traits>
"""

GEN_DATA_TEMPLATE = r'''
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
'''

FILES = [
    (
        Path("Modules/Filtering/AnisotropicSmoothing/include/itkCurvatureAnisotropicDiffusionImageFilter.h"),
        "CurvatureAnisotropicDiffusionImageFilter",
        "  ~CurvatureAnisotropicDiffusionImageFilter() override = default;",
    ),
    (
        Path("Modules/Filtering/AnisotropicSmoothing/include/itkGradientAnisotropicDiffusionImageFilter.h"),
        "GradientAnisotropicDiffusionImageFilter",
        "  ~GradientAnisotropicDiffusionImageFilter() override = default;",
    ),
]


def patch_file(path: Path, cls: str, anchor: str) -> str:
    text = path.read_text(encoding="utf-8")
    if "kunpeng: iterate CAD/GAD in double" in text or "DoubleSelf = " + cls in text:
        return "skip (already applied)"
    if '#include "itkCastImageFilter.h"' not in text:
        key = '#include "itkAnisotropicDiffusionImageFilter.h"'
        if key not in text:
            raise ValueError(f"missing include anchor in {path}")
        text = text.replace(key, key + CAD_INCLUDES, 1)
    if anchor not in text:
        raise ValueError(f"missing method anchor in {path}: {anchor}")
    method = "  /* kunpeng: iterate CAD/GAD in double */\n" + GEN_DATA_TEMPLATE.format(cls=cls)
    text = text.replace(anchor, anchor + "\n" + method, 1)
    path.write_text(text, encoding="utf-8")
    return "applied"


def patch_tree(root: Path):
    logs = []
    for rel, cls, anchor in FILES:
        p = root / rel
        if not p.is_file():
            raise FileNotFoundError(p)
        logs.append(f"{rel}: {patch_file(p, cls, anchor)}")
    return logs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("itk_src", type=Path)
    args = ap.parse_args()
    root = args.itk_src
    for line in patch_tree(root):
        print(line)


if __name__ == "__main__":
    main()
