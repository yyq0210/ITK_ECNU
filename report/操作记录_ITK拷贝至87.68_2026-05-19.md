# ITK-5.4.0 从 87.20 同步至 87.68 — 操作记录

**日期：2026-05-19**  
**源：** `root@202.120.87.20` → `/home/pub/yyq/ITK-5.4.0/`（约 **4.8 GB**，含 `build-yyq`）  
**目标：** `hpc@202.120.87.68` → **`/home/hpc/yyq/ITK/ITK-5.4.0/`**  
**方式：** 在源机上 `rsync -aH` + `expect` 输入目标机密码（源机无 `sshpass`）

---

## 1. 同步结果

| 项 | 结果 |
|----|------|
| 传输量 | 约 **5.06 GB**（日志 `100%`，约 **44 s**，~109 MB/s） |
| 目标目录大小 | **`du -sh` → 4.9G** |
| 示例二进制 | `build-yyq/bin/BilateralImageFilter` 存在（**BIN_OK**） |
| 日志（源机） | `/tmp/sync_itk_to_68.log` |

---

## 2. 重要说明（到新机器后）

1. **`build-yyq` 内 CMake 缓存** 仍记录 **`/home/pub/yyq/ITK-5.4.0`** 等旧路径，在 **87.68** 上若直接 `cmake --build` 可能报错。建议二选一：  
   - **新建构建目录** 重新 `cmake`（推荐）；或  
   - 在 68 上建立相同路径软链接（仅当目录结构可一致时）。  
2. 依赖 **gcc10 / cmake 3.27** 等工具链在 **87.68** 上是否同样安装在 `/home/pub/...`，需自行核对。  
3. 同步脚本在仓库：[`sync_itk_to_68.expect`](../sync_itk_to_68.expect)、[`upload_sync_expect.cmd`](../upload_sync_expect.cmd)；再次同步可在源机执行：  
   `env ITK_SYNC_PASS='…' expect /tmp/sync_itk_to_68.exp`

---

## 3. Cursor 项目文档（`docs/`）

已同步至 **`/home/hpc/yyq/ITK/docs/`**（约 **200 KB**）：

| 子目录 | 内容 |
|--------|------|
| `README.md` | 项目理解摘要、文档与脚本索引 |
| `操作记录/` | 11 篇操作记录（编译、绑核、扫频、MPI、GPU、拷贝等） |
| `scripts/` | 绑核/基准/htop 相关 `.sh` 脚本 |

本地维护目录：`D:\ECNU_HPC\ITK_huawei\docs\`；再次上传：`upload_docs_to_servers.cmd`（需 `ITK_SYNC_PASS`）。

## 4. 登录目标机检查

```bash
ssh hpc@202.120.87.68
ls -la /home/hpc/yyq/ITK/ITK-5.4.0
du -sh /home/hpc/yyq/ITK/ITK-5.4.0
ls -la /home/hpc/yyq/ITK/docs/
```

---

*密码勿写入 Git；本次由操作者在会话中提供，仅用于一次性 `rsync`。*
