# 优化优先：误差已 `<1e-5` 但 float 不快（94）

来源：作业 1960127 / `mixable100` 一轮×100。双标已过 343；本表是剩余主战场。

| 优先级 | 条件 | 个数 |
| --- | --- | ---: |
| near_miss | speedup ≥ 0.95 | 59 |
| medium | 0.8 ≤ speedup < 0.95 | 21 |
| hard | speedup < 0.8 | 14 |
| **合计** | | **94** |

策略：先打 near_miss（差一点就过），再看耗时长的 medium；hard 多半是 float 路径本身更重或含 double 度量。

## Top near_miss / 高耗时

| # | 函数 | speedup | float_ms | double_ms | 模块 |
| ---: | --- | ---: | ---: | ---: | --- |
| 1 | `MRIBiasFieldCorrectionFilter` | 1.0000 | 44.027 | 44.029 | Filtering/BiasCorrection |
| 2 | `VectorGradientMagnitudeImageFilter` | 0.9999 | 1428.748 | 1428.594 | Filtering/ImageGradient |
| 3 | `BinaryReconstructionByDilationImageFilter` | 0.9999 | 40.366 | 40.363 | Filtering/LabelMap |
| 4 | `LaplacianDeformationQuadEdgeMeshFilterWithHardConstraints` | 0.9997 | 16.909 | 16.903 | Filtering/QuadEdgeMeshFiltering |
| 5 | `ReconstructionByErosionImageFilter` | 0.9995 | 196.650 | 196.546 | Filtering/MathematicalMorphology |
| 6 | `DiscreteMinimumCurvatureQuadEdgeMeshFilter` | 0.9992 | 27.416 | 27.395 | Filtering/QuadEdgeMeshFiltering |
| 7 | `BinaryThinningImageFilter` | 0.9990 | 3.896 | 3.892 | Filtering/BinaryMathematicalMorphology |
| 8 | `DirectedHausdorffDistanceImageFilter` | 0.9975 | 66.320 | 66.156 | Filtering/DistanceMap |
| 9 | `ANTSNeighborhoodCorrelationImageToImageMetricv4` | 0.9970 | 82.987 | 82.739 | Registration/Metricsv4 |
| 10 | `ExponentialDisplacementFieldImageFilter` | 0.9970 | 11.416 | 11.381 | Filtering/DisplacementField |
| 11 | `DiscreteMeanCurvatureQuadEdgeMeshFilter` | 0.9969 | 26.511 | 26.429 | Filtering/QuadEdgeMeshFiltering |
| 12 | `ChainCodeToFourierSeriesPathFilter` | 0.9966 | 0.178 | 0.177 | Filtering/Path |
| 13 | `TensorRelativeAnisotropyImageFilter` | 0.9966 | 0.153 | 0.152 | Filtering/DiffusionTensorImage |
| 14 | `ReinitializeLevelSetImageFilter` | 0.9965 | 958.923 | 955.552 | Segmentation/LevelSets |
| 15 | `StatisticsLabelMapFilter` | 0.9964 | 0.036 | 0.036 | Filtering/LabelMap |
| 16 | `BorderQuadEdgeMeshFilter` | 0.9962 | 2.274 | 2.266 | Filtering/QuadEdgeMeshFiltering |
| 17 | `BSplineControlPointImageFilter` | 0.9946 | 0.583 | 0.580 | Filtering/ImageGrid |
| 18 | `NormalQuadEdgeMeshFilter` | 0.9943 | 9.002 | 8.951 | Filtering/QuadEdgeMeshFiltering |
| 19 | `MovingHistogramMorphologicalGradientImageFilter` | 0.9937 | 9.421 | 9.361 | Filtering/MathematicalMorphology |
| 20 | `BinaryImageToShapeLabelMapFilter` | 0.9927 | 58.513 | 58.089 | Filtering/LabelMap |
| 21 | `VectorRescaleIntensityImageFilter` | 0.9926 | 11.770 | 11.683 | Filtering/ImageIntensity |
| 22 | `ConvertLabelMapFilter` | 0.9926 | 0.023 | 0.023 | Filtering/LabelMap |
| 23 | `PathToChainCodePathFilter` | 0.9913 | 0.049 | 0.049 | Filtering/Path |
| 24 | `MergeLabelMapFilter` | 0.9912 | 0.028 | 0.028 | Filtering/LabelMap |
| 25 | `GaborImageSource` | 0.9907 | 76.035 | 75.327 | Filtering/ImageSources |

