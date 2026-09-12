# -*- coding: utf-8 -*-
"""Fill Excel H/I/J/K/M/N from 1024 remesure + construct-data benches. Serial rows stay empty."""
from __future__ import annotations

import csv
import math
import re
import sys
from pathlib import Path

from openpyxl import load_workbook
from openpyxl.styles import Alignment, Font, PatternFill

DOCS = Path(r"D:\ECNU_HPC\ITK_huawei\docs")
sys.path.insert(0, str(DOCS))
import _fill_cannot_notes as notes  # noqa: E402

XLSX = DOCS / "算法性能测试结果-并行加速_含未绑核.xlsx"
if not XLSX.exists():
    XLSX = DOCS / "算法性能测试结果-并行加速_arm.xlsx"
GREEN = PatternFill("solid", fgColor="C6EFCE")
GRAY = PatternFill("solid", fgColor="D9D9D9")
CENTER = Alignment(horizontal="center", vertical="center", wrap_text=True)
NOTE_FONT = Font(name="宋体", size=9)
WRAP = Alignment(wrap_text=True, vertical="center")

NO_MIXED = {
    "AndImageFilter",
    "OrImageFilter",
    "XorImageFilter",
    "NotImageFilter",
    "BinaryNotImageFilter",
    "ModulusImageFilter",
    "ConnectedComponentImageFilter",
    "RelabelComponentImageFilter",
    "HardConnectedComponentImageFilter",
    "ScalarConnectedComponentImageFilter",
    "ChangeLabelImageFilter",
    "LabelContourImageFilter",
    "ThresholdMaximumConnectedComponentsImageFilter",
    "ThresholdLabelerImageFilter",
    "BinaryImageToLabelMapFilter",
    "BinaryImageToShapeLabelMapFilter",
    "LabelImageToLabelMapFilter",
    "LabelMapToLabelImageFilter",
    "LabelMapToBinaryImageFilter",
    "RelabelLabelMapFilter",
    "ChangeLabelLabelMapFilter",
    "ShapeKeepNObjectsLabelMapFilter",
    "MorphologicalWatershedImageFilter",
    "WatershedImageFilter",
    "TobogganImageFilter",
    "IsolatedWatershedImageFilter",
    "SLICImageFilter",
    "ScalarImageKmeansImageFilter",
    "STAPLEImageFilter",
    "LabelVotingImageFilter",
    "MultiLabelSTAPLEImageFilter",
    "ComposeImageFilter",
    "JoinImageFilter",
    "JoinSeriesImageFilter",
    "ScalarToRGBColormapImageFilter",
    "LabelToRGBImageFilter",
    "VectorMagnitudeImageFilter",
    "GradientImageFilter",
    "InvertDisplacementFieldImageFilter",
    "MeanProjectionImageFilter",
    "MaximumProjectionImageFilter",
    "MinimumProjectionImageFilter",
    "SumProjectionImageFilter",
    "AccumulateImageFilter",
    "ExtractImageFilter",
    "HausdorffDistanceImageFilter",
    "SimilarityIndexImageFilter",
    "ContourMeanDistanceImageFilter",
    "DirectedHausdorffDistanceImageFilter",
    "ContourDirectedMeanDistanceImageFilter",
    "VnlForwardFFTImageFilter",
    "VnlInverseFFTImageFilter",
    "GaussianImageSource",
    "GaborImageSource",
    "StatisticsImageFilter",
    "MinimumMaximumImageFilter",
    "AccumulateImageFilter",
    "MedianProjectionImageFilter",
    "StandardDeviationProjectionImageFilter",
    "BinaryProjectionImageFilter",
    "BinaryThresholdProjectionImageFilter",
    "HardConnectedComponentImageFilter",
    "ThresholdMaximumConnectedComponentsImageFilter",
    "BinaryShapeKeepNObjectsImageFilter",
    "ShapeRelabelImageFilter",
    "LabelVotingImageFilter",
    "STAPLEImageFilter",
    "WatershedImageFilter",
    "IsolatedWatershedImageFilter",
    "MorphologicalWatershedFromMarkersImageFilter",
    "HessianRecursiveGaussianImageFilter",
    "HoughTransform2DCirclesImageFilter",
    "HoughTransform2DLinesImageFilter",
    "GradientRecursiveGaussianImageFilter",
    "ComposeDisplacementFieldsImageFilter",
    "ExponentialDisplacementFieldImageFilter",
    "InverseDisplacementFieldImageFilter",
    "IterativeInverseDisplacementFieldImageFilter",
    "DisplacementFieldToBSplineImageFilter",
    "TransformToDisplacementFieldFilter",
    "VnlForward1DFFTImageFilter",
    "VnlInverse1DFFTImageFilter",
    "VnlRealToHalfHermitianForwardFFTImageFilter",
    "VnlHalfHermitianToRealInverseFFTImageFilter",
    "LevelSetDomainMapImageFilter",
    "LabeledPointSetToPointSetMetricv4",
    "ParameterizationQuadEdgeMeshFilter",
    "LaplacianDeformationQuadEdgeMeshFilterWithHardConstraints",
    "LaplacianDeformationQuadEdgeMeshFilterWithSoftConstraints",
    "FullToHalfHermitianImageFilter",
    "HalfToFullHermitianImageFilter",
    "ComplexToModulusImageFilter",
    "ComplexToRealImageFilter",
    "ComplexToImaginaryImageFilter",
    "ComplexToPhaseImageFilter",
    "MagnitudeAndPhaseToComplexImageFilter",
    "FrequencyBandImageFilter",
    "VectorGradientAnisotropicDiffusionImageFilter",
    "VectorCurvatureAnisotropicDiffusionImageFilter",
    "VectorRescaleIntensityImageFilter",
    "VectorIndexSelectionCastImageFilter",
    "VectorGradientMagnitudeImageFilter",
    "WarpVectorImageFilter",
    "LabelToRGBImageFilter",
    "LabelOverlayImageFilter",
    "LabelMapOverlayImageFilter",
    "LabelMapToRGBImageFilter",
    "PhysicalPointImageSource",
    "BinaryShapeOpeningImageFilter",
    "BinaryStatisticsOpeningImageFilter",
    "BinaryStatisticsKeepNObjectsImageFilter",
    "LabelShapeOpeningImageFilter",
    "LabelShapeKeepNObjectsImageFilter",
    "LabelStatisticsOpeningImageFilter",
    "LabelStatisticsKeepNObjectsImageFilter",
    "StatisticsRelabelImageFilter",
    "BinaryReconstructionByDilationImageFilter",
    "BinaryReconstructionByErosionImageFilter",
    "LabelImageToShapeLabelMapFilter",
    "LabelImageToStatisticsLabelMapFilter",
    "BinaryImageToStatisticsLabelMapFilter",
    "ShapeOpeningLabelMapFilter",
    "ShapeRelabelLabelMapFilter",
    "ShapeUniqueLabelMapFilter",
    "AutoCropLabelMapFilter",
    "ShiftScaleLabelMapFilter",
    "LabelSelectionLabelMapFilter",
    "MergeLabelMapFilter",
    "AggregateLabelMapFilter",
    "PadLabelMapFilter",
    "CropLabelMapFilter",
    "StatisticsOpeningLabelMapFilter",
    "StatisticsRelabelLabelMapFilter",
    "StatisticsKeepNObjectsLabelMapFilter",
    "LabelMapMaskImageFilter",
    "LabelMapToAttributeImageFilter",
    "ChangeRegionLabelMapFilter",
    "ShapeLabelMapFilter",
    "StatisticsLabelMapFilter",
    "ConvertLabelMapFilter",
    "MultiLabelSTAPLEImageFilter",
    "BayesianClassifierInitializationImageFilter",
    "BayesianClassifierImageFilter",
    "VoronoiSegmentationImageFilter",
    "VoronoiPartitioningImageFilter",
    "BlockMatchingImageFilter",
    "LabelOverlapMeasuresImageFilter",
    "VectorConnectedComponentImageFilter",
    "VoronoiSegmentationRGBImageFilter",
    "LabelMapContourOverlayImageFilter",
    "MatrixIndexSelectionImageFilter",
    "SymmetricEigenAnalysisImageFilter",
    "KLMRegionGrowImageFilter",
    "BayesianClassifierImageFilter",
    "MaskFeaturePointSelectionFilter",
    "DiffusionTensor3DReconstructionImageFilter",
    "VectorExpandImageFilter",
    "TimeVaryingVelocityFieldIntegrationImageFilter",
    "InverseDisplacementFieldImageFilter",
    "BSplineControlPointImageFilter",
    "GradientVectorFlowImageFilter",
    "ImplicitManifoldNormalVectorFilter",
    "CurvatureRegistrationFilter",
}

MEASURED_NOTE = (
    "已实测（1024×1024；缺专用输入的已造二值/整数/标签/向量/两图/卷积核）。"
    "未绑核=不指定 CPU、系统自己调度；绑核=钉在 0–95 号核上。"
    "混合精度=同一算法用单精度跑一遍、用双精度再跑一遍，比时间和误差。"
)
PATH_MESH_NOTE = (
    "已造数据实测。"
    "路径：脑切片抽等高线，或程序里画一条折线。"
    "网格：程序生成球面（ITK RegularSphere），不用网上模型。"
    "未绑核=不指定 CPU；绑核=钉 0–95。"
    "混合精度=同一套几何用单精度坐标跑一遍、用双精度再跑一遍。"
)
LEVELSET_NOTE = (
    "已造数据实测水平集（2D，8 次迭代）。"
    "初值=圆心种子圆的有符号距离图；速度场=平滑后的梯度幅值再经 Sigmoid。"
    "未绑核=不指定 CPU；绑核=钉 0–95。"
    "混合精度=同一初值/速度场用单精度跑一遍、用双精度再跑一遍。"
)
METRIC_NOTE = (
    "已造数据实测配准度量/点云。"
    "两张图：固定图=1024 脑切片，移动图=平移 5、3 像素后重采样。"
    "点集：从图像每隔 8 像素抽样（核方法隔 32 像素）。"
    "未绑核=不指定 CPU；绑核=钉 0–95。"
    "混合精度=同一套输入用单精度跑一遍、用双精度再跑一遍。"
)
CAT3_NOTE = (
    "已造第3类合法输入实测。"
    "网格=开边界平面盘/球面 QuadEdge；Simplex=球面三角网转单纯形+20³ 盒梯度；"
    "路径=方形折线转傅里叶+正交条带 merit；形状先验=圆距离图+Sphere PCA；"
    "SpatialObject=椭圆光栅化；标签点集=两类三维点；domain map=重叠标签列表图。"
    "未绑核=不指定 CPU；绑核=钉 0–95。"
    "混合精度=能比单/双精度坐标或灰度的才填 K/M，标签/domain map 留空。"
)
NO_MIXED_NOTE = (
    "已造数据实测绑核/未绑核（1024×1024）。"
    "混合精度不适用：输出是整数标签/按位运算/向量/投影降维，不是灰度单通道的单精度 vs 双精度对比。"
    "故 K/M 留空。"
)

KEEP_EVENING = notes.KEEP_EVENING if hasattr(notes, "KEEP_EVENING") else {
    "SmoothingRecursiveGaussianImageFilter",
    "DiscreteGaussianImageFilter",
    "MeanImageFilter",
    "BoxMeanImageFilter",
    "BilateralImageFilter",
    "ConvolutionImageFilter",
    "FFTConvolutionImageFilter",
    "GradientMagnitudeImageFilter",
    "NormalizeImageFilter",
    "ResampleImageFilter",
    "SobelEdgeDetectionImageFilter",
    "GrayscaleDilateImageFilter",
    "BinaryThresholdImageFilter",
    "SignedMaurerDistanceMapImageFilter",
    "CurvatureFlowImageFilter",
    "OtsuThresholdImageFilter",
    "CurvatureAnisotropicDiffusionImageFilter",
    "GradientAnisotropicDiffusionImageFilter",
    "ImageRegistrationMethodv4",
    "MattesMutualInformationImageToImageMetricv4",
}

EVE_NAME = {
    "RecursiveGaussian": "SmoothingRecursiveGaussianImageFilter",
    "DiscreteGaussian": "DiscreteGaussianImageFilter",
    "OtsuThreshold": "OtsuThresholdImageFilter",
    "GradientMagnitude": "GradientMagnitudeImageFilter",
    "Normalize": "NormalizeImageFilter",
    "ResampleIdentity": "ResampleImageFilter",
    "SobelEdge": "SobelEdgeDetectionImageFilter",
    "GrayscaleDilate": "GrayscaleDilateImageFilter",
    "BinaryThreshold": "BinaryThresholdImageFilter",
    "SignedMaurerDistance": "SignedMaurerDistanceMapImageFilter",
    "CurvatureFlow": "CurvatureFlowImageFilter",
    "Bilateral(domain=4,range=50)": "BilateralImageFilter",
    "DiscreteGaussian(sigma=4)": "DiscreteGaussianImageFilter",
    "Mean(radius=15)": "MeanImageFilter",
    "BoxMean(radius=15)": "BoxMeanImageFilter",
    "FFTConvolution(kernel=31,sigma=4)": "FFTConvolutionImageFilter",
}


def _finite(x: float) -> bool:
    return math.isfinite(x) and abs(x) < 1e6


def parse_csv(path: Path) -> dict[str, dict[str, float]]:
    out: dict[str, dict[str, float]] = {}
    if not path.exists():
        return out
    with path.open(encoding="utf-8") as f:
        for row in csv.DictReader(f):
            op = (row.get("operator") or "").strip()
            mf = (row.get("ms_float") or "").strip()
            md = (row.get("ms_double") or "").strip()
            if not op or md in ("", "FAIL") or mf == "FAIL":
                continue
            rec: dict[str, float] = {}
            try:
                rec["ms_double"] = float(md)
            except ValueError:
                continue
            try:
                rec["ms_float"] = float(mf)
            except ValueError:
                pass
            ma = (row.get("max_abs") or "").strip()
            try:
                v = float(ma)
                if _finite(v):
                    rec["max_abs"] = v
            except ValueError:
                pass
            out[op] = rec
    return out


def parse_evening(path: Path) -> dict[str, dict[str, float]]:
    out: dict[str, dict[str, float]] = {}
    if not path.exists():
        return out
    text = path.read_text(encoding="utf-8", errors="replace")
    pat = re.compile(
        r"^(?P<name>[^\n|]+?)\s*\|\s*ms_float=(?P<f>[\d.]+)\s+ms_double=(?P<d>[\d.]+)",
        re.M,
    )
    for m in pat.finditer(text):
        fn = EVE_NAME.get(m.group("name").strip())
        if not fn:
            continue
        out[fn] = {"ms_float": float(m.group("f")), "ms_double": float(m.group("d"))}
    for m in re.finditer(
        r"^CAD\(iter=\d+\)\s*\|\s*ms_float=([\d.]+)\s+ms_double=([\d.]+)", text, re.M
    ):
        out["CurvatureAnisotropicDiffusionImageFilter"] = {
            "ms_float": float(m.group(1)),
            "ms_double": float(m.group(2)),
        }
    for m in re.finditer(
        r"^GAD\(iter=\d+\)\s*\|\s*ms_float=([\d.]+)\s+ms_double=([\d.]+)", text, re.M
    ):
        out["GradientAnisotropicDiffusionImageFilter"] = {
            "ms_float": float(m.group(1)),
            "ms_double": float(m.group(2)),
        }
    mf = re.search(r"float_storage:\s+([\d.]+)\s+ms", text)
    md = re.search(r"double_storage:\s+([\d.]+)\s+ms", text)
    if mf and md:
        rec = {"ms_float": float(mf.group(1)), "ms_double": float(md.group(1))}
        out["ImageRegistrationMethodv4"] = rec
        out["MattesMutualInformationImageToImageMetricv4"] = rec
    return out


def merge_side(fill: dict, eve: dict) -> dict[str, dict[str, float]]:
    out = dict(fill)
    for k, v in fill.items():
        if k in KEEP_EVENING:
            out.pop(k, None)
    for k, v in eve.items():
        out[k] = v
    return out


def set_num(cell, value: float) -> None:
    cell.value = round(value, 4)
    cell.number_format = "0.0000"
    cell.alignment = CENTER
    cell.fill = GREEN


def main() -> None:
    bound = merge_side(
        parse_csv(DOCS / "fill_missing_1024_bound.csv"),
        parse_evening(DOCS / "evening_1024_bound.txt"),
    )
    unbound = merge_side(
        parse_csv(DOCS / "fill_missing_1024_unbound.csv"),
        parse_evening(DOCS / "evening_1024_unbound.txt"),
    )
    for p in (
        DOCS / "construct_1024_bound.csv",
        DOCS / "construct2_1024_bound.csv",
        DOCS / "construct3_1024_bound.csv",
        DOCS / "path_mesh_1024_bound.csv",
        DOCS / "levelset_1024_bound.csv",
        DOCS / "metric_1024_bound.csv",
        DOCS / "remain_1024_bound.csv",
        DOCS / "remain_fix_bound.csv",
        DOCS / "extra35_1024_bound.csv",
        DOCS / "cat3_1024_bound.csv",
    ):
        bound.update(parse_csv(p))
    for p in (
        DOCS / "construct_1024_unbound.csv",
        DOCS / "construct2_1024_unbound.csv",
        DOCS / "construct3_1024_unbound.csv",
        DOCS / "path_mesh_1024_unbound.csv",
        DOCS / "levelset_1024_unbound.csv",
        DOCS / "metric_1024_unbound.csv",
        DOCS / "remain_1024_unbound.csv",
        DOCS / "remain_fix_unbound.csv",
        DOCS / "extra35_1024_unbound.csv",
        DOCS / "cat3_1024_unbound.csv",
    ):
        unbound.update(parse_csv(p))

    kinds = notes.load_kind()
    src = XLSX if XLSX.exists() else DOCS / "算法性能测试结果-并行加速_arm.xlsx"
    if not src.exists():
        src = DOCS / "算法性能测试结果-并行加速_补测已填.xlsx"
    wb = load_workbook(src)
    ws = wb["itk"]
    n_fill = n_serial = n_reason = 0
    for r in range(2, ws.max_row + 1):
        name = ws.cell(r, 3).value or ""
        module = ws.cell(r, 2).value or ""
        kind = kinds.get(name, (module, "ok"))[1]
        serial = notes.is_serial(name, module, kind)
        ws.cell(r, 4).value = "串行" if serial else "并行"
        ws.cell(r, 4).alignment = Alignment(horizontal="center", vertical="center")
        if serial:
            n_serial += 1
            for col in (8, 9, 10, 11, 13, 14):
                ws.cell(r, col).value = None
            cell = ws.cell(r, 15)
            cell.value = notes.SERIAL_NOTE
            cell.fill = GRAY
            cell.font = NOTE_FONT
            cell.alignment = WRAP
            continue

        b = bound.get(name)
        u = unbound.get(name)
        mixed_ok = name not in NO_MIXED
        if b or u:
            n_fill += 1
            if u and "ms_double" in u:
                set_num(ws.cell(r, 8), u["ms_double"])
            if b and "ms_double" in b:
                set_num(ws.cell(r, 9), b["ms_double"])
            if ws.cell(r, 8).value is not None and ws.cell(r, 9).value is not None:
                ws.cell(r, 10).value = f"=H{r}/I{r}"
                ws.cell(r, 10).alignment = CENTER
                ws.cell(r, 10).fill = GREEN
            if mixed_ok and b and "ms_float" in b:
                set_num(ws.cell(r, 11), b["ms_float"])
                ws.cell(r, 13).value = f"=I{r}/K{r}"
                ws.cell(r, 13).alignment = CENTER
                ws.cell(r, 13).fill = GREEN
                if "max_abs" in b:
                    ws.cell(r, 14).value = b["max_abs"]
                    ws.cell(r, 14).alignment = CENTER
            else:
                ws.cell(r, 11).value = None
                ws.cell(r, 13).value = None
                if not mixed_ok:
                    ws.cell(r, 14).value = None
            cell = ws.cell(r, 15)
            if name in {
                "BorderQuadEdgeMeshFilter",
                "ParameterizationQuadEdgeMeshFilter",
                "NormalQuadEdgeMeshFilter",
                "QuadricDecimationQuadEdgeMeshFilter",
                "SquaredEdgeLengthDecimationQuadEdgeMeshFilter",
                "LaplacianDeformationQuadEdgeMeshFilterWithHardConstraints",
                "LaplacianDeformationQuadEdgeMeshFilterWithSoftConstraints",
                "OrthogonalSwath2DPathFilter",
                "LevelSetDomainMapImageFilter",
                "LabeledPointSetToPointSetMetricv4",
                "GeodesicActiveContourShapePriorLevelSetImageFilter",
                "DeformableSimplexMesh3DFilter",
                "DeformableSimplexMesh3DBalloonForceFilter",
                "DeformableSimplexMesh3DGradientConstraintForceFilter",
                "ImageToSpatialObjectRegistrationMethod",
            }:
                cell.value = CAT3_NOTE
            elif "Path" in name or "QuadEdgeMesh" in name or "ContourExtractor" in name or "Swath" in name:
                cell.value = PATH_MESH_NOTE
            elif "LevelSet" in name or name in {
                "CollidingFrontsImageFilter",
                "BinaryMaskToNarrowBandPointSetFilter",
                "ExtensionVelocitiesImageFilter",
            }:
                cell.value = LEVELSET_NOTE
            elif (
                "Metric" in name
                or "PointSet" in name
                or name == "BSplineScatteredDataPointSetToImageFilter"
            ):
                cell.value = METRIC_NOTE
            elif "LabelMap" in name or name in {
                "BlockMatchingImageFilter",
                "PatchBasedDenoisingImageFilter",
                "HoughTransform2DLinesImageFilter",
                "VoronoiSegmentationImageFilter",
                "VoronoiPartitioningImageFilter",
            }:
                cell.value = (
                    "已造数据实测。"
                    "缺专用输入的已造：二值连通域→LabelMap、位移场、复数频谱、向量场、平移第二张图。"
                    "块匹配/Patch去噪/Hough/Voronoi 用 96×96 裁块，避免 1024² 超时。"
                    "未绑核=不指定 CPU；绑核=钉 0–95。"
                    "混合精度=能比灰度单/双精度的才填 K/M，标签/向量/复数留空。"
                )
            else:
                cell.value = NO_MIXED_NOTE if not mixed_ok else MEASURED_NOTE
            cell.fill = GREEN
            cell.font = NOTE_FONT
            cell.alignment = WRAP
            continue

        cell = ws.cell(r, 15)
        cell.value = notes.reason(name, module, kind)
        cell.fill = GRAY
        cell.font = NOTE_FONT
        cell.alignment = WRAP
        n_reason += 1

    if "口径说明" in wb.sheetnames:
        w2 = wb["口径说明"]
        w2["A14"] = (
            "串行算法不测绑核、不测混合精度，H/I/J/K/M 留空。"
            "并行：H=未绑核 ms_double，I=绑核 ms_double，K=绑核混合精度 ms_float；"
            "J=H/I，M=I/K。图幅 1024×1024。空间卷积在 1024² 跳过。"
        )

    out = XLSX
    try:
        wb.save(out)
        saved = out
    except OSError:
        saved = DOCS / "算法性能测试结果-并行加速_1024造数据.xlsx"
        wb.save(saved)
    print(f"filled={n_fill} serial={n_serial} reason={n_reason} bound={len(bound)} unbound={len(unbound)} saved={saved}")


if __name__ == "__main__":
    main()
