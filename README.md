# ITK_ECNU

华东师大 / 华为鲲鹏上的 ITK 5.4 移植与混合精度验收。

- **远程代码仓库：** https://github.com/yyq0210/ITK_ECNU
- **鲲鹏机器上的运行副本：** `root@202.120.87.86:/home/pub/yyq/itk_hybrid_precision_demo`  
  （服务器上没有 git，以本仓库为准）

## 已测函数怎么复测

见 **[docs/已测函数复测指南.md](docs/已测函数复测指南.md)**。

| 目录 | 内容 |
|------|------|
| `delivery/ITK-Kunpeng-Validation/` | 全部 bench 源码与编译/跑数脚本 |
| `report/itk_hybrid_precision_demo/` | 鲲鹏上使用的 CMake 工程 |
| `docs/*_bound.csv` `docs/*_unbound.csv` | 已测墙钟 |
| `testdata/` | 官方脑切片副本与造数说明 |
| `release/` | 交付用 ITK 源码包说明与 Validation 快照 |
