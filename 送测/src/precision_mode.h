#pragma once

#include <cctype>
#include <cstdlib>
#include <string>

enum class BenchPrec
{
  Both,
  FloatOnly,
  DoubleOnly
};

inline BenchPrec
GetBenchPrec()
{
  const char * e = std::getenv("ITK_BENCH_PRECISION");
  if (e == nullptr || e[0] == '\0')
  {
    return BenchPrec::Both;
  }
  std::string s(e);
  for (char & c : s)
  {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  if (s == "float" || s == "fp32" || s == "single")
  {
    return BenchPrec::FloatOnly;
  }
  if (s == "double" || s == "fp64")
  {
    return BenchPrec::DoubleOnly;
  }
  return BenchPrec::Both;
}

inline bool
WantFloat()
{
  return GetBenchPrec() != BenchPrec::DoubleOnly;
}

inline bool
WantDouble()
{
  return GetBenchPrec() != BenchPrec::FloatOnly;
}

template <typename FnF, typename FnD, typename Timer>
inline void
TimePrec(FnF runF, FnD runD, int runs, Timer timer, double & msF, double & msD)
{
  msF = 0.0;
  msD = 0.0;
  if (WantFloat())
  {
    runF();
    msF = timer(runF, runs);
  }
  if (WantDouble())
  {
    runD();
    msD = timer(runD, runs);
  }
}
