# ITK-5.4.0-Huawei-Kunpeng 源码包

混精改动已烘焙进本包，**不需要**再跑外挂补丁脚本。

## 编译

```bash
tar -xzf ITK-5.4.0-Huawei-Kunpeng.tar.gz
cmake -S ITK-5.4.0-Huawei-Kunpeng -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_TESTING=OFF \
  -DITK_DEFAULT_MAX_THREADS=592 \
  -DITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS=ON
cmake --build build -j
cmake --install build
```

关键开关：`ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS`

- `ON`（默认）：float 图像内部累加用 float
- `OFF`：退回 `RealType`（double）累加

灵昇一键入口：`lingsheng592/compile_itk.sh`（自动选本包并打开开关）。

重新烘焙（维护者）：

```bash
cd lingsheng592
python3 bake_release_itk.py
```
