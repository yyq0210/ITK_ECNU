# ITK Filter float 累加 — 开发期工具（已废弃，不交付）

> **结项交付请使用 [`../../delivery/`](../delivery/)**，不再交付本目录下的 apply/revert 脚本。

本目录保留仅供 **开发期 A/B 对比**（float 累加前后性能）。正式交付由 `delivery/apply_kunpeng_to_itk.py` 将改动烘焙进完整 ITK 源码树。

## 正式交付（推荐）

```bash
bash delivery/prepare_kunpeng_itk_release.sh \
  /home/pub/yyq/ITK-5.4.0 \
  /home/pub/yyq/release/ITK-5.4.0-Huawei-Kunpeng \
  -t
```

## 开发期对比（可选，勿交付）

以下脚本修改远端 ITK 源码树，并留下 `yyq:` 标记，**仅用于实验**：

```bash
export ITK_SRC=/home/pub/yyq/ITK-5.4.0
bash apply_patches.sh
bash apply_bilateral_full_float.sh
bash apply_patches_topn.sh
# 还原
bash revert_patches_topn.sh
bash revert_bilateral_full_float.sh
bash revert_patches.sh
```

合入内容与交付烘焙脚本等价，见 [`ITK鲲鹏交付代码说明.md`](../ITK鲲鹏交付代码说明.md)。
