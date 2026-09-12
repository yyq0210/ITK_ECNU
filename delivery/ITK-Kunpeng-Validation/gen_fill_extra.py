# -*- coding: utf-8 -*-
"""Generate precision_fill_extra.inc — 2-template ImageToImageFilter calls."""
from pathlib import Path

out = Path(r"D:\ECNU_HPC\ITK_huawei\delivery\ITK-Kunpeng-Validation\precision_fill_extra.inc")

def r2(mod, cls, sf="(void)0", sd="(void)0"):
    return f"""  Run2<itk::{cls}<FImg, FImg>, itk::{cls}<DImg, DImg>>(
    "{mod}", "{cls}", input, runs,
    [&](auto * f) {{ {sf}; }},
    [&](auto * f) {{ {sd}; }});
"""

def r1(mod, cls, sf="(void)0", sd="(void)0"):
    return f"""  Run1<itk::{cls}<FImg>, itk::{cls}<DImg>>(
    "{mod}", "{cls}", input, runs,
    [&](auto * f) {{ {sf}; }},
    [&](auto * f) {{ {sd}; }});
"""

def rbin(mod, cls, sf="(void)0", sd="(void)0"):
    return f"""  RunBin<itk::{cls}<FImg, FImg>, itk::{cls}<DImg, DImg>>(
    "{mod}", "{cls}", binF, binD, runs,
    [&](auto * f) {{ {sf}; }},
    [&](auto * f) {{ {sd}; }});
"""

def rbin1(mod, cls, sf="(void)0", sd="(void)0"):
    return f"""  RunBin1<itk::{cls}<FImg>, itk::{cls}<DImg>>(
    "{mod}", "{cls}", binF, binD, runs,
    [&](auto * f) {{ {sf}; }},
    [&](auto * f) {{ {sd}; }});
"""

def rmorph(mod, cls):
    return f"""  RunMorph<itk::{cls}<FImg, FImg, BallF>, itk::{cls}<DImg, DImg, BallD>>(
    "{mod}", "{cls}", input, kf, kd, runs);
"""

def rbinmorph2(mod, cls):
    return f"""  RunBinMorph<itk::{cls}<FImg, BallF>, itk::{cls}<DImg, BallD>>(
    "{mod}", "{cls}", binF, binD, kf, kd, runs);
"""

thr_setup_f = "f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f)"
thr_setup_d = "f->SetInsideValue(1.0); f->SetOutsideValue(0.0)"
# skip filters already in precision_fill_missing_bench / evening official
SKIP = {
    "ShiftScaleImageFilter",
    "RescaleIntensityImageFilter",
    "BoxSigmaImageFilter",
    "LaplacianRecursiveGaussianImageFilter",
    "BilateralImageFilter",
    "CurvatureFlowImageFilter",
    "CurvatureAnisotropicDiffusionImageFilter",
    "GradientAnisotropicDiffusionImageFilter",
    "NotImageFilter",  # bitwise, needs integer
    "HistogramThresholdImageFilter",
    "KappaSigmaThresholdImageFilter",
    "BinaryThresholdProjectionImageFilter",
    "ThresholdLabelerImageFilter",
    "BinaryProjectionImageFilter",
    "AccumulateImageFilter",
    "MeanProjectionImageFilter",
    "MaximumProjectionImageFilter",
    "MinimumProjectionImageFilter",
    "SumProjectionImageFilter",
    "StandardDeviationProjectionImageFilter",
    "SLICImageFilter",
    "MorphologicalWatershedImageFilter",
    "ScalarConnectedComponentImageFilter",
    "ChangeLabelImageFilter",
    "AdaptiveHistogramEqualizationImageFilter",  # 1 template
}

thr = [
    "HuangThresholdImageFilter",
    "IntermodesThresholdImageFilter",
    "IsoDataThresholdImageFilter",
    "KittlerIllingworthThresholdImageFilter",
    "LiThresholdImageFilter",
    "MaximumEntropyThresholdImageFilter",
    "MomentsThresholdImageFilter",
    "RenyiEntropyThresholdImageFilter",
    "ShanbhagThresholdImageFilter",
    "TriangleThresholdImageFilter",
    "YenThresholdImageFilter",
]

chunks = ["// auto-generated extra 2D filter benches\n"]
for c in thr:
    chunks.append(r2("Thresholding", c, thr_setup_f, thr_setup_d))
chunks.append(r2("Thresholding", "OtsuMultipleThresholdsImageFilter", "f->SetNumberOfThresholds(2)", "f->SetNumberOfThresholds(2)"))

chunks.append(r2("ImageIntensity", "ClampImageFilter", "f->SetBounds(0.0f, 255.0f)", "f->SetBounds(0.0, 255.0)"))
chunks.append(r2("ImageIntensity", "SigmoidImageFilter", "f->SetAlpha(10.0); f->SetBeta(100.0)", "f->SetAlpha(10.0); f->SetBeta(100.0)"))
chunks.append(r2("ImageIntensity", "IntensityWindowingImageFilter", "f->SetWindowMinimum(0.0f); f->SetWindowMaximum(255.0f)", "f->SetWindowMinimum(0.0); f->SetWindowMaximum(255.0)"))
chunks.append(r2("ImageIntensity", "NormalizeToConstantImageFilter", "f->SetConstant(1.0f)", "f->SetConstant(1.0)"))
chunks.append(r2("ImageIntensity", "Log10ImageFilter"))
chunks.append(r2("ImageIntensity", "LogImageFilter"))

chunks.append(r2("Smoothing", "BinomialBlurImageFilter", "f->SetRepetitions(2)", "f->SetRepetitions(2)"))
chunks.append(r2("ImageFeature", "UnsharpMaskImageFilter", "f->SetAmount(0.5)", "f->SetAmount(0.5)"))
chunks.append(r2("ImageFeature", "ZeroCrossingBasedEdgeDetectionImageFilter", "f->SetVariance(2.0)", "f->SetVariance(2.0)"))
chunks.append(r2("ImageFeature", "DerivativeImageFilter", "f->SetOrder(1); f->SetDirection(0)", "f->SetOrder(1); f->SetDirection(0)"))
chunks.append(r2("ImageFeature", "DiscreteGaussianDerivativeImageFilter", "f->SetVariance(2.0)", "f->SetVariance(2.0)"))
chunks.append(r2("ImageFeature", "CannyEdgeDetectionImageFilter", "f->SetVariance(2.0); f->SetUpperThreshold(10.0); f->SetLowerThreshold(5.0)", "f->SetVariance(2.0); f->SetUpperThreshold(10.0); f->SetLowerThreshold(5.0)"))
chunks.append(r2("ImageFeature", "SimpleContourExtractorImageFilter"))
chunks.append(r2("ImageGradient", "GradientMagnitudeRecursiveGaussianImageFilter", "f->SetSigma(1.5)", "f->SetSigma(1.5)"))

chunks.append(r1("ImageGrid", "FlipImageFilter", "auto a=f->GetFlipAxes(); a[0]=true; f->SetFlipAxes(a)", "auto a=f->GetFlipAxes(); a[0]=true; f->SetFlipAxes(a)"))
chunks.append(r2("ImageGrid", "ShrinkImageFilter", "f->SetShrinkFactors(2)", "f->SetShrinkFactors(2)"))
chunks.append(r2("ImageGrid", "ExpandImageFilter", "f->SetExpandFactors(2)", "f->SetExpandFactors(2)"))
chunks.append(r2("ImageGrid", "ConstantPadImageFilter", "f->SetPadLowerBound(pad); f->SetPadUpperBound(pad); f->SetConstant(0.0f)", "f->SetPadLowerBound(pad); f->SetPadUpperBound(pad); f->SetConstant(0.0)"))
chunks.append(r2("ImageGrid", "MirrorPadImageFilter", "f->SetPadLowerBound(pad); f->SetPadUpperBound(pad)", "f->SetPadLowerBound(pad); f->SetPadUpperBound(pad)"))
chunks.append(r2("ImageGrid", "WrapPadImageFilter", "f->SetPadLowerBound(pad); f->SetPadUpperBound(pad)", "f->SetPadLowerBound(pad); f->SetPadUpperBound(pad)"))
chunks.append(r2("ImageGrid", "ZeroFluxNeumannPadImageFilter", "f->SetPadLowerBound(pad); f->SetPadUpperBound(pad)", "f->SetPadLowerBound(pad); f->SetPadUpperBound(pad)"))
chunks.append(r2("ImageGrid", "CropImageFilter", "f->SetLowerBoundaryCropSize(pad); f->SetUpperBoundaryCropSize(pad)", "f->SetLowerBoundaryCropSize(pad); f->SetUpperBoundaryCropSize(pad)"))
chunks.append(r1("ImageGrid", "CyclicShiftImageFilter", "itk::Offset<2> s; s.Fill(5); f->SetShift(s)", "itk::Offset<2> s; s.Fill(5); f->SetShift(s)"))
chunks.append(r2("ImageGrid", "BinShrinkImageFilter", "f->SetShrinkFactors(2)", "f->SetShrinkFactors(2)"))
chunks.append(r1("ImageGrid", "ChangeInformationImageFilter"))
chunks.append(r2("ImageGrid", "RegionOfInterestImageFilter", "f->SetRegionOfInterest(roi)", "f->SetRegionOfInterest(roi)"))

chunks.append(r2("ImageNoise", "AdditiveGaussianNoiseImageFilter", "f->SetStandardDeviation(5.0); f->SetSeed(1)", "f->SetStandardDeviation(5.0); f->SetSeed(1)"))
chunks.append(r2("ImageNoise", "SaltAndPepperNoiseImageFilter", "f->SetProbability(0.01); f->SetSeed(1)", "f->SetProbability(0.01); f->SetSeed(1)"))
chunks.append(r2("ImageNoise", "ShotNoiseImageFilter", "f->SetScale(1.0); f->SetSeed(1)", "f->SetScale(1.0); f->SetSeed(1)"))
chunks.append(r2("ImageNoise", "SpeckleNoiseImageFilter", "f->SetStandardDeviation(0.1); f->SetSeed(1)", "f->SetStandardDeviation(0.1); f->SetSeed(1)"))

chunks.append(r2("MathematicalMorphology", "HMaximaImageFilter", "f->SetHeight(10.0)", "f->SetHeight(10.0)"))
chunks.append(r2("MathematicalMorphology", "HMinimaImageFilter", "f->SetHeight(10.0)", "f->SetHeight(10.0)"))
chunks.append(r2("MathematicalMorphology", "HConvexImageFilter", "f->SetHeight(10.0)", "f->SetHeight(10.0)"))
chunks.append(r2("MathematicalMorphology", "HConcaveImageFilter", "f->SetHeight(10.0)", "f->SetHeight(10.0)"))
chunks.append(r2("MathematicalMorphology", "RegionalMaximaImageFilter"))
chunks.append(r2("MathematicalMorphology", "RegionalMinimaImageFilter"))
chunks.append(r2("MathematicalMorphology", "GrayscaleFillholeImageFilter"))
chunks.append(r2("MathematicalMorphology", "GrayscaleGrindPeakImageFilter"))
chunks.append(r2("MathematicalMorphology", "RankImageFilter", "f->SetRadius(radius)", "f->SetRadius(radius)"))
chunks.append(rmorph("MathematicalMorphology", "OpeningByReconstructionImageFilter"))
chunks.append(rmorph("MathematicalMorphology", "ClosingByReconstructionImageFilter"))

chunks.append(rbinmorph2("BinaryMathematicalMorphology", "BinaryOpeningByReconstructionImageFilter"))
chunks.append(rbinmorph2("BinaryMathematicalMorphology", "BinaryClosingByReconstructionImageFilter"))
chunks.append(rbin1("BinaryMathematicalMorphology", "BinaryFillholeImageFilter", "f->SetForegroundValue(1.0f)", "f->SetForegroundValue(1.0)"))
chunks.append(rbin("BinaryMathematicalMorphology", "BinaryPruningImageFilter", "f->SetIteration(3)", "f->SetIteration(3)"))
chunks.append(rbin("BinaryMathematicalMorphology", "BinaryThinningImageFilter"))

chunks.append(rbin("DistanceMap", "ApproximateSignedDistanceMapImageFilter", "f->SetInsideValue(1.0f); f->SetOutsideValue(0.0f)", "f->SetInsideValue(1.0); f->SetOutsideValue(0.0)"))
chunks.append(rbin("DistanceMap", "FastChamferDistanceImageFilter"))
chunks.append(rbin("DistanceMap", "IsoContourDistanceImageFilter", "f->SetLevelSetValue(0.5)", "f->SetLevelSetValue(0.5)"))
chunks.append(rbin("ImageLabel", "BinaryContourImageFilter", "f->SetForegroundValue(1.0f)", "f->SetForegroundValue(1.0)"))
chunks.append(r1("ImageStatistics", "AdaptiveHistogramEqualizationImageFilter", "f->SetRadius(radius); f->SetAlpha(0.3); f->SetBeta(0.3)", "f->SetRadius(radius); f->SetAlpha(0.3); f->SetBeta(0.3)"))

chunks.append(r2("CurvatureFlow", "MinMaxCurvatureFlowImageFilter", "f->SetTimeStep(0.125); f->SetNumberOfIterations(6); f->SetStencilRadius(1)", "f->SetTimeStep(0.125); f->SetNumberOfIterations(6); f->SetStencilRadius(1)"))
chunks.append(rbin("AntiAlias", "AntiAliasBinaryImageFilter", "f->SetMaximumRMSError(0.01); f->SetNumberOfIterations(6)", "f->SetMaximumRMSError(0.01); f->SetNumberOfIterations(6)"))

chunks.append(r2("RegionGrowing", "ConnectedThresholdImageFilter", "f->SetSeed(seed); f->SetLower(50.0f); f->SetUpper(200.0f); f->SetReplaceValue(1.0f)", "f->SetSeed(seed); f->SetLower(50.0); f->SetUpper(200.0); f->SetReplaceValue(1.0)"))
chunks.append(r2("RegionGrowing", "ConfidenceConnectedImageFilter", "f->SetSeed(seed); f->SetMultiplier(2.5); f->SetNumberOfIterations(2); f->SetReplaceValue(1.0f)", "f->SetSeed(seed); f->SetMultiplier(2.5); f->SetNumberOfIterations(2); f->SetReplaceValue(1.0)"))
chunks.append(r2("RegionGrowing", "NeighborhoodConnectedImageFilter", "f->SetSeed(seed); f->SetLower(50.0f); f->SetUpper(200.0f); f->SetRadius(radius); f->SetReplaceValue(1.0f)", "f->SetSeed(seed); f->SetLower(50.0); f->SetUpper(200.0); f->SetRadius(radius); f->SetReplaceValue(1.0)"))
chunks.append(rbin("LabelVoting", "BinaryMedianImageFilter", "f->SetRadius(radius)", "f->SetRadius(radius)"))
chunks.append(rbin("LabelVoting", "VotingBinaryImageFilter", "f->SetRadius(radius); f->SetForegroundValue(1.0f); f->SetBackgroundValue(0.0f)", "f->SetRadius(radius); f->SetForegroundValue(1.0); f->SetBackgroundValue(0.0)"))
chunks.append(rbin("LabelVoting", "VotingBinaryHoleFillingImageFilter", "f->SetRadius(radius); f->SetForegroundValue(1.0f); f->SetBackgroundValue(0.0f)", "f->SetRadius(radius); f->SetForegroundValue(1.0); f->SetBackgroundValue(0.0)"))
chunks.append(r2("Registration", "MultiResolutionPyramidImageFilter", "f->SetNumberOfLevels(2)", "f->SetNumberOfLevels(2)"))
chunks.append(r2("Registration", "RecursiveMultiResolutionPyramidImageFilter", "f->SetNumberOfLevels(2)", "f->SetNumberOfLevels(2)"))

text = "".join(chunks)
out.write_text(text, encoding="utf-8")
print("wrote", out, "bytes", out.stat().st_size, "calls", text.count("Run"))
