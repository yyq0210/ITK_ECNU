# -*- coding: utf-8 -*-
import re
from pathlib import Path
p = Path(r"D:\ECNU_HPC\ITK_huawei\delivery\ITK-Kunpeng-Validation\precision_fill_extra.inc")
t = p.read_text(encoding="utf-8")
names = sorted(set(re.findall(r"itk::([A-Za-z0-9]+ImageFilter)", t)))
out = Path(r"D:\ECNU_HPC\ITK_huawei\delivery\ITK-Kunpeng-Validation\precision_fill_extra_includes.inc")
out.write_text("\n".join('#include "itk%s.h"' % n for n in names) + "\n", encoding="utf-8")
print("headers", len(names), "wrote", out)
