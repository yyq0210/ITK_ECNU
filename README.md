# ITK_ECNU（灵昇分支）

本分支 **`lingsheng-592-qualified`** 用于在 **灵昇 HPC** 上编译当前 ITK 5.4，并在计算节点 592 核上重测已合格的混合精度函数。

最终测试入口：**[lingsheng592/README.md](lingsheng592/README.md)**

远程仓库：https://github.com/yyq0210/ITK_ECNU  
灵昇代码目录：`/home/share/nsls_yyq/yyq/ITK_ECNU`

## 在灵昇上测

登录节点准备编译，计算节点跑名单（Donau 队列 `q_hpcapp`，`cpu=592`）：

```bash
cd /home/share/nsls_yyq/yyq/ITK_ECNU/lingsheng592
bash build_pinpreload.sh
bash compile_itk.sh
bash compile_benches.sh
bash submit_592.sh
# 四个遗留接口（向量/频谱/B样条）另交：
bash submit_four_retry.sh
```

合格标准、数据集、排除节点、作业日志见 `lingsheng592/README.md`。584 个接口逐条结果见 [`lingsheng592/interface_status_584.md`](lingsheng592/interface_status_584.md)。不要在登录节点执行 `run_qualified.sh`。不要把 SSH 密码写进仓库。

主测图：`test/data/BrainProtonDensity1024.png`。

## 本分支里其它目录

| 目录 | 用途 |
| --- | --- |
| `lingsheng592/` | **灵昇最终测试脚本、合格名单、复测 CSV** |
| `test/data/` | 官方脑切片与 1024 图 |
| `test/src/` | bench 源码（灵昇编译脚本会补上 fail-retry / remainder） |
| `test/scripts/` | 鲲鹏旧流程，本分支验收不走这里 |

鲲鹏 96 核旧入口仍在 `test/`，与本分支的灵昇 592 核验收分开。
