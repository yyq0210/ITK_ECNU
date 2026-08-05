# ITK-Kunpeng-Validation

华为鲲鹏 ITK 5.4 优化版的 **功能与性能验收程序**，依赖已编译安装的 ITK。

## 编译

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/ITK-5.4.0-Huawei-Kunpeng/build
cmake --build . -j
```

## 程序说明

| 程序 | 用途 |
|------|------|
| `precision_pipeline_bench` | 典型 pipeline float vs double |
| `precision_bilateral_bench` | Bilateral 精度与性能 |
| `precision_conv_bench` | 卷积类 Filter 对比 |
| `precision_registration_bench` | Metricsv4 配准 float 存 + double 算 |
| `precision_diffusion_bench` | 各向异性/同性扩散 |

## 运行示例

```bash
export ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=96
./precision_pipeline_bench
./precision_registration_bench
```

测试数据可使用 ITK 官方 Example 数据（如 `BrainProtonDensitySlice256x256.png`）。

## 说明

本目录随 `ITK-5.4.0-Huawei-Kunpeng` 一并交付，**不包含** ITK 源码或补丁脚本。
