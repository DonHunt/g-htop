#pragma once
#include "pdh.hpp"
#include <algorithm>
#include <cstdio>
namespace monitoring {
class CpuCollector {
  win::Counter counter{L"\\Processor Information(*)\\% Processor Time"};

public:
  std::map<unsigned, double> sample() {
    std::map<unsigned, double> result;
    for (WORD group = 0; group < GetActiveProcessorGroupCount(); ++group)
      for (DWORD core = 0; core < GetActiveProcessorCount(group); ++core)
        result[group * 64u + core] = -1;
    for (const auto &[name, value] : counter.sample()) {
      unsigned group = 0, core = 0;
      wchar_t extra = 0;
      if (swscanf(name.c_str(), L"%u,%u%c", &group, &core, &extra) == 2 &&
          core < 64 && result.count(group * 64 + core))
        result[group * 64 + core] = std::clamp(value, 0.0, 100.0);
    }
    return result;
  }
};
} // namespace monitoring
