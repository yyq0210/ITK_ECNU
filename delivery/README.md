# ITK 5.4 鲲鹏平台 — 结项交付工具

本目录用于从 ITK 5.4 源码生成 **正式交付包**，替代 `2026-0507/itk_float_accum_patches/` 下的 apply/revert 脚本。

## 交付物

| 目录/文件 | 说明 |
|-----------|------|
| `ITK-5.4.0-Huawei-Kunpeng/` | 完整 ITK 源码（优化已合入） |
| `ITK-Kunpeng-Validation/` | 验收 benchmark 程序 |
| `README-KUNPENG.md` | 写入 ITK 根目录的平台说明 |
| `CMake/KunpengToolchain.cmake` | 鲲鹏推荐编译选项 |

**不交付：** `itk_float_accum_patches/`、apply/revert 脚本。

## 一键生成（远端）

```bash
cd /path/to/ITK_huawei
bash delivery/prepare_kunpeng_itk_release.sh \
  /home/pub/yyq/ITK-5.4.0 \
  /home/pub/yyq/release/ITK-5.4.0-Huawei-Kunpeng \
  -t
```

输出：

- `/home/pub/yyq/release/ITK-5.4.0-Huawei-Kunpeng/`
- `/home/pub/yyq/release/ITK-Kunpeng-Validation/`
- `/home/pub/yyq/release/ITK-5.4.0-Huawei-Kunpeng.tar.gz`

## 手动调用 Python

```bash
python3 delivery/apply_kunpeng_to_itk.py \
  /home/pub/yyq/ITK-5.4.0 \
  /home/pub/yyq/release/ITK-5.4.0-Huawei-Kunpeng \
  -t \
  --validation-dir /home/pub/yyq/release/ITK-Kunpeng-Validation
```

开发期原地合入（**勿用于交付**，仅本地试验）：

```bash
python3 delivery/apply_kunpeng_to_itk.py /home/pub/yyq/ITK-5.4.0 --in-place
```

## 合入的源码改动

见 [`../report/ITK鲲鹏交付代码说明.md`](../report/ITK鲲鹏交付代码说明.md) 第 3 节。CAD/GAD 有限差分 **不** 改为 float。

## 验收方编译 ITK

```bash
cd ITK-5.4.0-Huawei-Kunpeng
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=../CMake/KunpengToolchain.cmake \
  -DITK_BUILD_DEFAULT_MODULES=ON
cmake --build . -j$(nproc)
```

无需任何 patch 步骤。
