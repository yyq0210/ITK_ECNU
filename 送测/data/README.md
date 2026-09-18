# 测试数据

主测图来自 ITK 5.4 官方 `Examples/Data`，不另造灰度切片。

| 文件 | 来源 |
|------|------|
| BrainProtonDensitySlice.png | ITK 官方 |
| BrainProtonDensitySliceBorder20.png | ITK 官方 |
| BrainProtonDensitySliceShifted13x17y.png | ITK 官方 |
| BrainProtonDensity1024.png | 官方图拉成 1024×1024 |
| BrainProtonDensity1024_fixed.png | Border20 拉成 1024×1024 |
| BrainProtonDensity1024_moving.png | Shifted13x17y 拉成 1024×1024 |

其余输入（网格、折线、标签表、水平集初值、点集、位移场、复数频谱等）**没有独立文件**，由 `src/precision_*_bench.cxx` 在运行时按 ITK 官方测试协议生成。
