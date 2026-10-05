# 灵昇 592 核实测 ITK 5.4.0（改过源码）

本分支是从灵昇服务器 `/tmp/itk-src/InsightToolkit-5.4.0` 打包的 **ITK 5.4.0 源码**，即 592 核混合精度测试实际编译、安装、跑数用的那份，**不是** 上游未改动的官方树。

静态安装树（`.a` / 头文件）仍在灵昇：`/home/share/nsls_yyq/yyq/ITK-5.4.0-static/install`。本分支只收源码，不收编译产物。测试脚本在同仓库分支 [`lingsheng-592-qualified`](https://github.com/yyq0210/ITK_ECNU/tree/lingsheng-592-qualified)。

## 相对官方 5.4.0 的改动

- CMake 选项 `ITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS`（默认 ON）
- `ITK_ACCUMULATION_TYPE`：float 像素用 float 累加，double 仍用 `RealType`
- 已改：`itkMeanImageFilter`、`itkBoxUtilities`、`itkDiscreteGaussianImageFilter`、`itkBilateralImageFilter`
- CAD / GAD：float 图先转 double 迭代再写回 float
- 实测配置：`ITK_DEFAULT_MAX_THREADS=592`，FFTW 关闭，静态库，gold 链接

实测生成的 `itkConfigure.h` 副本在 `lingsheng-snapshot/itkConfigure.h`。

## 在灵昇上复现这次库

```bash
cmake -S InsightToolkit-5.4.0 -B /tmp/itk-static \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_TESTING=OFF \
  -DBUILD_EXAMPLES=OFF \
  -DITK_DEFAULT_MAX_THREADS=592 \
  -DITK_USE_FLOAT_ACCUMULATION_FOR_FLOAT_PIXELS=ON \
  -DCMAKE_C_COMPILER=/usr/bin/gcc \
  -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
  -DCMAKE_C_FLAGS='-B/home/share/nsls_yyq/yyq/ld-gold -fno-use-linker-plugin -O3 -march=armv8.2-a+crypto' \
  -DCMAKE_CXX_FLAGS='-B/home/share/nsls_yyq/yyq/ld-gold -fno-use-linker-plugin -O3 -march=armv8.2-a+crypto' \
  -DCMAKE_EXE_LINKER_FLAGS='-B/home/share/nsls_yyq/yyq/ld-gold -fno-use-linker-plugin'

# 只用 /usr/bin/cmake
cmake --build /tmp/itk-static -j "$(nproc)"
cmake --install /tmp/itk-static
```

更完整的流程见 `lingsheng-592-qualified` 里的 `lingsheng592/compile_itk.sh`。
