@echo off
REM 投递 CAD/GAD double 有限差分修复到鲲鹏 202.120.87.86 并编译实测
set HOST=root@202.120.87.86
scp -o BatchMode=yes -o ConnectTimeout=20 "%~dp0optimize_cad_gad_double_fd.sh" %HOST%:/tmp/optimize_cad_gad_double_fd.sh
ssh -o BatchMode=yes -o ConnectTimeout=20 %HOST% "sed -i 's/\r$//' /tmp/optimize_cad_gad_double_fd.sh && chmod +x /tmp/optimize_cad_gad_double_fd.sh && bash /tmp/optimize_cad_gad_double_fd.sh"
