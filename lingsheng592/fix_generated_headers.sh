#!/bin/bash
# collect2 aborts during a later cmake can mark DICOM/GDCM checks wrong.
# Repair the generated headers. Do not re-run cmake to "fix" them.
set -euo pipefail
source "$(cd "$(dirname "$0")" && pwd)/env.sh"
python3 - "$ITK_BUILD" <<'PY'
import sys
from pathlib import Path
root = Path(sys.argv[1])
n = 0
for path in root.rglob("DICOMCMakeConfig.h"):
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines(True)
    changed = False
    for i, line in enumerate(lines):
        if line.strip() == "#define DICOM_NO_STD_NAMESPACE":
            lines[i] = line.replace(
                "#define DICOM_NO_STD_NAMESPACE",
                "/* #define DICOM_NO_STD_NAMESPACE */",
                1,
            )
            changed = True
    if changed:
        path.write_text("".join(lines), encoding="utf-8")
        print("fixed", path)
        n += 1
for path in root.rglob("gdcmConfigure.h"):
    text = path.read_text(encoding="utf-8", errors="replace")
    if "#define GDCM_HAVE_BYTESWAP_H" in text:
        continue
    if "#undef GDCM_HAVE_BYTESWAP_H" in text:
        text = text.replace("#undef GDCM_HAVE_BYTESWAP_H", "#define GDCM_HAVE_BYTESWAP_H", 1)
    else:
        text += "\n#define GDCM_HAVE_BYTESWAP_H\n"
    path.write_text(text, encoding="utf-8")
    print("fixed", path)
    n += 1
print("FIX_HEADERS", n)
PY
