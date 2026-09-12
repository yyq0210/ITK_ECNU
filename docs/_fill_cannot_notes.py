# -*- coding: utf-8 -*-
"""Rewrite 备注: why a filter cannot be tested / bound / mixed-precision."""
from __future__ import annotations

import csv
import re
from pathlib import Path

from openpyxl import load_workbook
from openpyxl.styles import Alignment, Font, PatternFill

DOCS = Path(r"D:\ECNU_HPC\ITK_huawei\docs")
XLSX = DOCS / "算法性能测试结果-并行加速_含未绑核.xlsx"
TSV = DOCS / "itk_filters_classified.tsv"

NOTE_FONT = Font(name="宋体", size=9)
WRAP = Alignment(wrap_text=True, vertical="center")
GRAY = PatternFill("solid", fgColor="D9D9D9")
GREEN = PatternFill("solid", fgColor="C6EFCE")

MEASURED_NOTE = (
    "已实测（1024×1024；缺专用输入的已造二值/整数/标签/向量/两图/卷积核）。"
    "未绑核=不指定 CPU、系统自己调度；绑核=钉在 0–95 号核上。"
    "混合精度=同一算法用单精度跑一遍、用双精度再跑一遍，比时间和误差。"
)

SERIAL_NAME_RE = re.compile(
    r"ByReconstruction|HMaxima|HMinima|HConvex|HConcave|Fillhole|GrindPeak|"
    r"RegionalMaxima|RegionalMinima|BinaryThinning|BinaryPruning|"
    r"OpeningByReconstruction|ClosingByReconstruction|"
    r"FastMarching"
)
SERIAL_MODULES = {"FastMarching", "RegionGrowing"}

SERIAL_NOTE = (
    "串行算法：不是把图像切成块多线程算。"
    "不测绑核（钉 96 核也不会按并行加速填）。"
    "不测混合精度。"
)


def load_kind() -> dict[str, tuple[str, str]]:
    out = {}
    with TSV.open(encoding="utf-8") as f:
        for row in csv.DictReader(f, delimiter="\t"):
            out[row["function"]] = (row["module"], row["kind"])
    return out


def is_serial(name: str, module: str, kind: str) -> bool:
    if "no_new" in (kind or "") or "gpu" in (kind or ""):
        return False
    if (name or "").endswith("FilterBase") or (name or "").endswith("ImageFilterBase"):
        return False
    mod = (module or "").split("/")[-1]
    if mod in SERIAL_MODULES:
        return True
    return bool(SERIAL_NAME_RE.search(name or ""))


def reason(name: str, module: str, kind: str) -> str:
    k = kind or ""
    n = name
    module = (module or "").split("/")[-1]

    if "gpu" in k or n.startswith("GPU"):
        return (
            "无法测试：GPU 滤波器，鲲鹏构建未启用 CUDA/OpenCL，无设备可跑。"
            "绑核：不适用。"
            "混合精度：未测。"
        )

    if "no_new" in k or n.endswith("FilterBase") or n.endswith("ImageFilterBase"):
        return (
            "无法独立测试：基类/抽象接口，不能单独 New() 计时。"
            "绑核：不适用。"
            "混合精度：不适用。"
        )

    if module == "QuadEdgeMeshFiltering" or "QuadEdgeMesh" in n or "MeshFilter" in n:
        return (
            "测不了：输入是三角网格（顶点+面），不是一张灰度图，不能用脑切片来跑。"
            "绑核：未测。"
            "混合精度：没有灰度像素，不能做单精度 vs 双精度对比。"
        )

    if module == "Path" or "PathFilter" in n or "PathTo" in n or n.startswith("ImageToPath"):
        return (
            "测不了：输入/输出是曲线路径，不是图像。"
            "绑核：未测。"
            "混合精度：不适用。"
        )

    if "SpatialObject" in n:
        return (
            "测不了：输入是几何物体（SpatialObject），不是灰度图。"
            "绑核：未测。"
            "混合精度：不适用。"
        )

    if module == "LabelMap" and ("LabelMap" in n or n.endswith("LabelMapFilter")):
        return (
            "测不了：处理的是『物体标签表』（每个连通区域一条记录），不是一张灰度图，不能用脑切片 PNG 直接跑。"
            "绑核：未测。"
            "混合精度：标签不是浮点像素，不能做单精度 vs 双精度对比。"
        )

    if "PointSet" in n or n.endswith("Metric") or n.endswith("Metricv4"):
        if "no_new" in k or n.endswith("Metric") and n in {
            "PointSetToImageMetric",
            "PointSetToPointSetMetric",
            "ImageToImageMetric",
            "ImageToImageMetricv4",
            "HistogramImageToImageMetric",
            "CompareHistogramImageToImageMetric",
            "PointSetToPointSetMetricv4",
            "PointSetToPointSetMetricWithIndexv4",
        }:
            return (
                "无法独立测试：基类/抽象接口，不能单独 New() 计时。"
                "绑核：不适用。"
                "混合精度：不适用。"
            )
        if n == "LabeledPointSetToPointSetMetricv4":
            return (
                "造了标签点集，但 ITK 5.4 该度量内部虚拟点坐标类型和整数标签点集对不上，无法实例化计时。"
                "绑核：未测。"
                "混合精度：未测。"
            )
        return (
            "已尽量造两图/点集实测；本函数仍缺可 New() 的完整协议或运行失败。"
            "绑核：未得到合法墙钟。"
            "混合精度：未测。"
        )

    if "non_scalar" in k or re.search(
        r"Vector|RGB|RGBA|Complex|Tensor|DisplacementField|Covariant|JoinImage",
        n,
    ):
        return (
            "混合精度做不了：每个像素不是一个灰度值，而是向量/彩色/复数/张量。"
            "本表混合精度=同一张灰度图用单精度跑一遍、用双精度再跑一遍，比时间和误差；这种像素套不上。"
            "绑核：未测（需要对应类型的输入，不是普通 MRI 切片）。"
        )

    if module == "DisplacementField" or "DisplacementField" in n or "VelocityField" in n:
        return (
            "测不了：需要位移场/速度场（每个像素是一个向量），不是灰度 MRI。"
            "绑核：未测。"
            "混合精度：不是单通道灰度，不能做单精度 vs 双精度对比。"
        )

    if module == "DeformableMesh" or "SimplexMesh" in n:
        return (
            "无法测试：需要 3D Simplex Mesh，不能用 2D 切片计时。"
            "绑核：未测。"
            "混合精度：不适用。"
        )

    if module == "FEM" or n.startswith("FEM") or "PhysicsBasedNonRigid" in n:
        return (
            "无法用 2D MRI 切片直接测试：需要 FEM 网格与材料模型。"
            "绑核：未测。"
            "混合精度：不适用。"
        )

    if module in ("LevelSets", "LevelSetsv4"):
        if "ShapePrior" in n:
            return (
                "无法测试：需要形状先验（PCA 统计形状模型），不能只靠种子距离图和速度场。"
                "绑核：未测。"
                "混合精度：未测。"
            )
        if n in {
            "SegmentationLevelSetImageFilter",
            "SparseFieldLevelSetImageFilter",
            "ParallelSparseFieldLevelSetImageFilter",
        }:
            return (
                "无法独立测试：要自己挂差分函数（DifferenceFunction），不是开箱即用。"
                "同类可跑的 GAC/Threshold/Curves 等已造数据实测（圆心种子距离图+Sigmoid 速度场）。"
                "绑核/混合精度：本行不单独计时。"
            )
        if n == "LevelSetDomainMapImageFilter":
            return (
                "测不了：LevelSetsv4 的多区域 domain map，不是单张水平集演化。"
                "绑核：未测。"
                "混合精度：未测。"
            )
        return (
            "无法独立测试：基类/抽象接口或缺少可 New() 的完整输入协议。"
            "绑核：不适用。"
            "混合精度：不适用。"
        )

    if module == "PDEDeformable" or "DemonsRegistration" in n or n.endswith("RegistrationFilter"):
        if n == "ImageRegistrationMethodv4":
            return MEASURED_NOTE
        return (
            "无法按单图滤波口径测试：可变形配准需要固定图+移动图+位移场迭代。"
            "绑核：未按该输入协议测。"
            "混合精度：未测。"
        )

    if n.startswith("FFTW"):
        return (
            "无法测试：FFTW 后端，本机构建默认走 Vnl FFT，未启用 FFTW，无法链接实测。"
            "绑核：不适用。"
            "混合精度：未测。"
        )

    if module == "FFT" and (
        n.startswith("Forward")
        or n.startswith("Inverse")
        or "Hermitian" in n
        or "ComplexToComplex" in n
        or n.startswith("Vnl")
    ):
        if "Base" in n or n in {
            "ForwardFFTImageFilter",
            "InverseFFTImageFilter",
            "Forward1DFFTImageFilter",
            "Inverse1DFFTImageFilter",
        }:
            return (
                "无法独立测试：FFT 抽象接口，真正跑的是 Vnl/FFTW 后端。"
                "绑核：不适用。"
                "混合精度：不适用。"
            )
        return (
            "混合精度做不了：输出是复数频谱，不是灰度值，没法逐个像素比较单精度和双精度结果。"
            "绑核：未测。"
        )

    if n in {
        "AndImageFilter",
        "OrImageFilter",
        "XorImageFilter",
        "NotImageFilter",
        "BinaryNotImageFilter",
        "ModulusImageFilter",
    }:
        return (
            "混合精度做不了：这是按位与/或/异或，像素必须是整数，用单精度/双精度浮点没有意义。"
            "绑核：算法可以多线程，本次未测。"
            "测试：未出墙钟。"
        )

    if "ConnectedComponent" in n or n in {
        "RelabelComponentImageFilter",
        "HardConnectedComponentImageFilter",
        "ThresholdMaximumConnectedComponentsImageFilter",
        "LabelContourImageFilter",
        "ChangeLabelImageFilter",
        "SLICImageFilter",
        "WatershedImageFilter",
        "MorphologicalWatershedImageFilter",
        "MorphologicalWatershedFromMarkersImageFilter",
        "TobogganImageFilter",
        "IsolatedWatershedImageFilter",
        "ScalarImageKmeansImageFilter",
        "BayesianClassifierImageFilter",
        "BayesianClassifierInitializationImageFilter",
        "LabelVotingImageFilter",
        "MultiLabelSTAPLEImageFilter",
        "STAPLEImageFilter",
    }:
        return (
            "混合精度做不了：输出是整数编号（第 1 块、第 2 块……），不是灰度值，单精度 vs 双精度对比没有意义。"
            "绑核：部分可并行，本次未测。"
            "测试：未出墙钟。"
        )

    if "Projection" in n or n in {
        "AccumulateImageFilter",
        "GetAverageSliceImageFilter",
        "ExtractImageFilter",
        "JoinSeriesImageFilter",
        "CropImageFilter",
    }:
        return (
            "测不了：输出图的尺寸/维数和输入不一致，没法和原图逐像素对比。"
            "绑核：未测。"
            "混合精度：未测。"
        )

    if module == "ImageSources" or n.endswith("ImageSource"):
        return (
            "无法按滤波口径测试：这是图像生成器，没有输入图。"
            "绑核：生成过程可并行，但与移植后滤波绑核口径不同，未填。"
            "混合精度：无输入像素类型对比。"
        )

    if n == "CastImageFilter":
        return (
            "未单独报墙钟：Cast 已嵌在每条 full_double 对照链里。"
            "绑核：随宿主滤波器。"
            "混合精度：本身就是类型转换，不是混合精度收益对象。"
        )

    if (module == "ImageFilterBase" and n != "CastImageFilter") or n in {
        "BinaryFunctorImageFilter",
        "BinaryGeneratorImageFilter",
        "UnaryFunctorImageFilter",
        "UnaryGeneratorImageFilter",
        "TernaryFunctorImageFilter",
        "BoxImageFilter",
        "KernelImageFilter",
        "NeighborhoodOperatorImageFilter",
        "MaskNeighborhoodOperatorImageFilter",
        "MovingHistogramImageFilter",
        "MovingHistogramImageFilterBase",
        "BinaryMorphologyImageFilter",
        "MorphologyImageFilter",
        "ObjectMorphologyImageFilter",
        "AdaptImageFilter",
        "InPlaceLabelMapFilter",
        "LabelMapFilter",
        "NoiseBaseImageFilter",
        "RegionGrowImageFilter",
        "VoronoiSegmentationImageFilterBase",
        "AnchorErodeDilateImageFilter",
        "AnchorOpenCloseImageFilter",
        "BasicDilateImageFilter",
        "BasicErodeImageFilter",
        "MovingHistogramDilateImageFilter",
        "MovingHistogramErodeImageFilter",
        "MovingHistogramMorphologicalGradientImageFilter",
        "MovingHistogramMorphologyImageFilter",
        "VanHerkGilWermanDilateImageFilter",
        "VanHerkGilWermanErodeImageFilter",
        "VanHerkGilWermanErodeDilateImageFilter",
    }:
        return (
            "无法独立测试：框架/基类，需Functor、核或派生类才能实例化，不是业务算子。"
            "绑核：不适用。"
            "混合精度：不适用。"
        )

    if module == "FastMarching" or n.startswith("FastMarching"):
        return (
            "串行算法：Fast Marching 按优先队列一个点一个点往外推，不是把图像切成块多线程算。"
            "绑核：不适用（钉 96 核也不会变快），本列按串行填。"
            "混合精度：距离可以用单/双精度算，本次未测墙钟。"
        )

    if re.search(
        r"ByReconstruction|HMaxima|HMinima|HConvex|HConcave|Fillhole|GrindPeak|"
        r"RegionalMaxima|RegionalMinima|BinaryThinning|BinaryPruning|"
        r"OpeningByReconstruction|ClosingByReconstruction",
        n,
    ):
        return (
            "串行算法：重建/测地/细化是从前往后传播，不是把图像切成块并行算。"
            "绑核：不适用（钉 96 核也不会变快），本列按串行填。"
            "混合精度：灰度形态学可以用单/双精度，本次未测墙钟。"
        )

    if module == "Common" or module == "RegistrationMethodsv4":
        return (
            "无法按单图滤波口径测试：配准方法/度量/优化器需要固定图+移动图+变换。"
            "绑核：未按该协议单独测（表中配准代表为 ImageRegistrationMethodv4）。"
            "混合精度：未对该函数单独测。"
        )

    if module in ("Classifiers", "MarkovRandomFieldsClassifiers", "Voronoi", "KLMRegionGrowing"):
        return (
            "无法直接测试：需要多种子/训练标签或 RGB，单张 float MRI 不能给出可重复墙钟。"
            "绑核：未测。"
            "混合精度：未测。"
        )

    if module == "RegionGrowing":
        return (
            "串行算法：从种子点往外灌水生长，不是把图像切成块并行算。"
            "绑核：不适用（钉 96 核也不会变快），本列按串行填。"
            "混合精度：未测（还缺统一种子点协议）。"
        )

    if n.startswith("CastImageFilter"):
        return (
            "未单独报墙钟：Cast 已嵌在每条 full_double 对照链里。"
            "绑核：随宿主滤波器。"
            "混合精度：本身就是类型转换，不是混合精度收益对象。"
        )

    # remaining "ok" but not measured: still need a concrete reason
    if module == "Deconvolution":
        return (
            "测不了：需要模糊核，而且迭代次数一变时间就变，没法跟其它滤波用同一套参数比。"
            "绑核：未测。"
            "混合精度：未测。"
        )

    if module == "BiasCorrection":
        return (
            "无法在本协议下测：N4/MRI bias 需要 3D 体数据与网格控制点，2D 切片不是其标准输入。"
            "绑核：未测。"
            "混合精度：未测。"
        )

    if module == "MathematicalMorphology":
        return (
            "未出墙钟：需要特定形状的结构元素，本次只测了半径 1 的圆盘核那一批。"
            "绑核：多数可并行，本次未测。"
            "混合精度：灰度形态学可以用单/双精度，本次未测。"
        )

    if module == "ImageGrid":
        return (
            "未出墙钟：几何变换/重采样需要形变场或控制点，不是对原图原地滤波。"
            "绑核：未测。"
            "混合精度：未测。"
        )

    if module == "ImageFeature":
        return (
            "未出墙钟：Hessian/Hough/Canny 需要三维或额外参数，本次只测了 Sobel、Laplace 等。"
            "绑核：未测。"
            "混合精度：Hessian 每个像素是矩阵不是灰度，不能做单精度 vs 双精度灰度对比；其余未测。"
        )

    if module == "Thresholding":
        return (
            "未出墙钟：直方图阈值这类其实能在灰度切片上跑，本次只测了 BinaryThreshold 和 Otsu。"
            "绑核：可并行，其余未测。"
            "混合精度：可以用单/双精度，其余未测。"
        )

    if module == "ImageIntensity":
        return (
            "未出墙钟：需要掩膜或直方图匹配等第二张图，单张脑切片不够。"
            "绑核：未测。"
            "混合精度：未测。"
        )

    if module == "ImageStatistics":
        return (
            "测不了：投影/统计会把图压成一条或一个数，没法跟原图逐像素对比单精度和双精度。"
            "绑核：未测。"
            "混合精度：未测。"
        )

    if module == "LabelMap":
        return (
            "测不了：处理的是物体标签，不是灰度像素。"
            "绑核：未测。"
            "混合精度：标签不是浮点灰度，不能做单精度 vs 双精度对比。"
        )

    if module == "BinaryMathematicalMorphology":
        return (
            "未出墙钟：需要二值图和结构元素；膨胀/腐蚀同类已测。"
            "绑核：可并行，本次未测。"
            "混合精度：输出只有 0/1，单精度 vs 双精度没有收益。"
        )

    if module == "ImageNoise":
        return (
            "未出墙钟：噪声类其实能在灰度切片上跑，本次没编进测试程序。"
            "绑核：可并行，未测。"
            "混合精度：可以用单/双精度，未测。"
        )

    if module == "AntiAlias":
        return (
            "未出墙钟：需要二值输入，本次未编入 bench。"
            "绑核：未测。"
            "混合精度：可以用单/双精度，未测。"
        )

    if module == "DistanceMap":
        return (
            "未出墙钟：SignedMaurer/Danielsson 已测；其余需要轮廓线或一对图，单张灰度切片不够。"
            "绑核：未测。"
            "混合精度：Hausdorff 给出的是一个距离数，不是整图逐像素对比。"
        )

    if module == "CurvatureFlow":
        return (
            "未出墙钟：CurvatureFlow 已测；MinMax/BinaryMinMax 需模板半径，未纳入同一协议。"
            "绑核：未测。"
            "混合精度：未测。"
        )

    if module == "Smoothing":
        return (
            "未出墙钟：Mean/Median/DiscreteGaussian/SmoothingRecursiveGaussian 已测；本函数未纳入同一协议。"
            "绑核：未测。"
            "混合精度：未测。"
        )

    if module == "Convolution":
        return (
            "未出墙钟：Convolution/FFTConvolution 已测；本函数需模板图或相关核，未纳入同一协议。"
            "绑核：未测。"
            "混合精度：未测。"
        )

    if module == "ImageGradient":
        return (
            "混合精度做不了：输出是梯度向量（每个像素两个方向），不是一个灰度值；表里已测标量版 GradientMagnitude。"
            "绑核：未测。"
        )

    if module == "ImageCompare":
        return (
            "未出墙钟：AbsoluteValueDifference/SquaredDifference 已测；CheckerBoard/STAPLE 需双图或标签图。"
            "绑核：未测。"
            "混合精度：STAPLE 输出标签，不适用。"
        )

    if module == "ImageLabel":
        return (
            "无法做混合精度：输出为二值轮廓或标签。"
            "绑核：未测。"
            "测试：未出墙钟。"
        )

    if module == "LabelVoting":
        return (
            "无法做混合精度：投票/标签融合输出为标签或二值。"
            "绑核：未测。"
            "测试：未出墙钟。"
        )

    if module == "ImageCompose":
        return (
            "测不了：是把几张图拼成多通道，或把切片叠成三维，不是单通道灰度滤波。"
            "绑核：未测。"
            "混合精度：不是单通道灰度，不能做单精度 vs 双精度对比。"
        )

    if module == "ImageFrequency":
        return (
            "测不了：这是频谱上的滤波，输入应是傅里叶变换结果，不是脑切片灰度图。"
            "绑核：未测。"
            "混合精度：未测。"
        )

    if module == "Denoising":
        return (
            "无法在本协议下测：块匹配去噪需要 3D/patch 参数，2D 切片不是其标准设定。"
            "绑核：未测。"
            "混合精度：未测。"
        )

    if module == "ImageFusion":
        return (
            "混合精度做不了：叠加/伪彩输出是彩色或标签叠加，不是单通道灰度。"
            "绑核：未测。"
        )

    if module == "SpatialFunction":
        return (
            "无法按滤波口径测试：把空间函数写到图像上，没有输入滤波算子。"
            "绑核：未测。"
            "混合精度：不适用。"
        )

    return (
        "未出墙钟：缺专用输入或参数，不能用这张二维灰度脑切片直接跑。"
        "绑核：未测。"
        "混合精度：未测。"
    )


def main() -> None:
    kinds = load_kind()
    wb = load_workbook(XLSX)
    ws = wb["itk"]
    n_meas = n_reason = n_serial = 0
    for r in range(2, ws.max_row + 1):
        name = ws.cell(r, 3).value
        module = ws.cell(r, 2).value
        kind = kinds.get(name, (module, "ok"))[1]
        measured = ws.cell(r, 9).value is not None
        serial = is_serial(name or "", module or "", kind)
        ws.cell(r, 4).value = "串行" if serial else "并行"
        ws.cell(r, 4).alignment = Alignment(horizontal="center", vertical="center")
        if serial:
            n_serial += 1
            # 串行：不填绑核/混合精度时间
            for col in (8, 9, 10, 11, 13, 14):
                ws.cell(r, col).value = None
            cell = ws.cell(r, 15)
            cell.value = SERIAL_NOTE
            cell.fill = GRAY
            cell.font = NOTE_FONT
            cell.alignment = WRAP
            continue
        cell = ws.cell(r, 15)
        if measured:
            cell.value = MEASURED_NOTE
            cell.fill = GREEN
            n_meas += 1
        else:
            cell.value = reason(name, module or "", kind)
            cell.fill = GRAY
            n_reason += 1
        cell.font = NOTE_FONT
        cell.alignment = WRAP

    if "口径说明" in wb.sheetnames:
        w2 = wb["口径说明"]
        w2["A14"] = (
            "串行/并行：标为串行的函数不测绑核、不测混合精度，对应列留空。"
            "并行函数：H=未绑核，I=绑核，K=混合精度时间，J=H/I，M=I/K。"
        )

    out = DOCS / "算法性能测试结果-并行加速_含未绑核.xlsx"
    try:
        wb.save(out)
        saved = out
    except OSError:
        saved = DOCS / "算法性能测试结果-并行加速_备注已写.xlsx"
        wb.save(saved)
    print(f"measured={n_meas} reason={n_reason} serial={n_serial} saved={saved}")


if __name__ == "__main__":
    main()
