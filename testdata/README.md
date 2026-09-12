# 测试数据

## 官方示例图（滤波 / 度量主图）

ITK 5.4 自带，不重新发明：

```text
/home/pub/yyq/ITK-5.4.0/Examples/Data/BrainProtonDensitySlice.png
```

1024 批次由脚本现生成：

```bash
convert "$OFFICIAL" -resize 1024x1024! /tmp/BrainProtonDensity1024.png
```

本目录可放一份官方 PNG 副本（`BrainProtonDensitySlice.png`），方便离线复测。

## 程序内造数（无独立文件）

以下不读外部网格/路径文件，由对应 `precision_*_bench.cxx` 生成，协议对齐 ITK `Modules/**/test`：

- 开边界平面 QuadEdge 盘、`RegularSphereMeshSource` 球面
- 方形折线 → ChainCode → 傅里叶路径 + 正交条带 merit
- Simplex：球面三角网 + `TriangleMeshToSimplexMeshFilter` + 20³ 盒梯度
- 形状先验：圆距离图 + `SphereSignedDistanceFunction`
- SpatialObject：椭圆光栅化
- 标签点集：两类 int 标签三维点
- LevelSetsv4 domain map：重叠 `std::list<int>` 图
- remain/extra35：裁块、位移场、复数频谱、LabelMap、DTI 梯度方向等

官方 `Testing/Data` 的 `.vtk` 在本机只有 `.cid` 哈希、没有实体文件，复测不依赖 ExternalData。
