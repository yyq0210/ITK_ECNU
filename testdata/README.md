# 测试数据

送测打包入口见仓库 **[送测/](../送测/README.md)**（图 + 脚本 + bench 源码在同一目录）。

## 官方示例图（滤波 / 度量主图）

ITK 5.4 自带，不重新发明。本目录已放副本：

| 文件 | 用途 |
|------|------|
| `BrainProtonDensitySlice.png` | 官方脑切片（与 ITK Examples/Data 相同） |
| `images/BrainProtonDensitySlice.png` | 同上 |
| `images/BrainProtonDensitySliceBorder20.png` | 配准固定图原图 |
| `images/BrainProtonDensitySliceShifted13x17y.png` | 配准移动图原图 |
| `images/BrainProtonDensitySliceBorder20Mask.png` | 掩膜 |
| `images/BrainProtonDensitySlice256x256.png` | 256² 切片 |
| `images/BrainProtonDensity1024.png` | 主测 1024×1024 |
| `images/BrainProtonDensity1024_fixed.png` | 配准固定图 1024 |
| `images/BrainProtonDensity1024_moving.png` | 配准移动图 1024 |

若需自行生成 1024：

```bash
python3 送测/scripts/prepare_data.py
```

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

## 分批脚本

`testdata/scripts/` 是鲲鹏上实际用过的分批 `run_*_1024.sh` 副本（内部路径写死 `/home/pub/yyq/...`）。新测请用 `送测/scripts/run_all.sh`。
