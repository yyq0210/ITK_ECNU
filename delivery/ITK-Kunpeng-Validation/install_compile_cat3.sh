#!/bin/bash
set -euo pipefail
python3 <<'PY'
from pathlib import Path
src = Path("/tmp/precision_cat3_bench.cxx")
dst = Path("/home/pub/yyq/itk_hybrid_precision_demo/precision_cat3_bench.cxx")
dst.write_bytes(src.read_bytes().replace(b"\r", b""))
print("wrote", dst, dst.stat().st_size)
PY
python3 <<'PY'
from pathlib import Path
p = Path("/home/pub/yyq/itk_hybrid_precision_demo/CMakeLists.txt")
t = p.read_text(encoding="utf-8")
need_comp = [
    "ITKDeformableMesh",
    "ITKLevelSetsv4",
    "ITKSpatialObjects",
    "ITKMesh",
    "ITKSignedDistanceFunction",
    "ITKOptimizers",
]
for c in need_comp:
    if c not in t:
        t = t.replace("ITKIOMeta)", f"{c} ITKIOMeta)")
        print("added component", c)
if "precision_cat3_bench" not in t:
    t = t.replace(
        "add_executable(precision_extra35_bench precision_extra35_bench.cxx)",
        "add_executable(precision_extra35_bench precision_extra35_bench.cxx)\n"
        "add_executable(precision_cat3_bench precision_cat3_bench.cxx)",
    )
    t = t.replace(
        "target_link_libraries(precision_extra35_bench PRIVATE ${ITK_LIBRARIES})",
        "target_link_libraries(precision_extra35_bench PRIVATE ${ITK_LIBRARIES})\n"
        "target_link_libraries(precision_cat3_bench PRIVATE ${ITK_LIBRARIES})",
    )
    t = t.replace(
        "target_link_options(precision_extra35_bench PRIVATE -static-libgcc -static-libstdc++)",
        "target_link_options(precision_extra35_bench PRIVATE -static-libgcc -static-libstdc++)\n"
        "  target_link_options(precision_cat3_bench PRIVATE -static-libgcc -static-libstdc++)",
    )
    print("added cat3 target")
p.write_text(t, encoding="utf-8")
print("cmake ok")
PY
bash /tmp/compile_cat3.sh
