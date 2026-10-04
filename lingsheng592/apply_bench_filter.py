#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Let suite benches run only names listed in ITK_BENCH_ONLY_FILE."""
import sys
from pathlib import Path

HEADER = r"""
#include <fstream>
#include <unordered_set>

inline bool
WantName(const std::string & name)
{
  const char * path = std::getenv("ITK_BENCH_ONLY_FILE");
  if (path == nullptr || path[0] == '\0')
  {
    return true;
  }
  static std::unordered_set<std::string> names;
  static bool loaded = false;
  if (!loaded)
  {
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line))
    {
      if (!line.empty() && line.back() == '\r')
      {
        line.pop_back();
      }
      if (!line.empty() && line[0] != '#')
      {
        names.insert(line);
      }
    }
    loaded = true;
  }
  return names.find(name) != names.end();
}
"""

GUARD = """  if (!WantName(name))
  {
    return;
  }
"""


def patch_header(path: Path):
    text = path.read_text(encoding="utf-8")
    if "WantName(" in text:
        print("already", path.name)
        return
    anchor = "#include <string>\n"
    if anchor not in text:
        raise SystemExit(f"header anchor missing: {path}")
    path.write_text(text.replace(anchor, anchor + HEADER, 1), encoding="utf-8")
    print("patched", path.name)


def patch_body(path: Path, needle: str):
    text = path.read_text(encoding="utf-8")
    if "WantName(name)" in text:
        print("already", path.name)
        return
    if needle not in text:
        raise SystemExit(f"anchor missing: {path}")
    if text.count(needle) != 1:
        raise SystemExit(f"anchor not unique: {path}")
    path.write_text(text.replace(needle, needle + GUARD, 1), encoding="utf-8")
    print("patched", path.name)


def main():
    src = Path(sys.argv[1] if len(sys.argv) > 1 else ".")
    patch_header(src / "precision_mode.h")
    patch_body(
        src / "precision_pipeline_bench.cxx",
        "TimeAndReport(const std::string & name, FnF runF, FnD runD, int runs)\n{\n",
    )
    brace = "int                                         runs)\n{\n"
    for name in ("precision_conv_bench.cxx", "precision_diffusion_bench.cxx"):
        patch_body(src / name, brace)


if __name__ == "__main__":
    main()
