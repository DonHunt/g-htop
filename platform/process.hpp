#pragma once
#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>
namespace monitoring {
struct ProcessRow {
  std::string name;
  int pid = 0;
  double cpu = -1, memory = -1;
};
struct ProcessTimes {
  std::uint64_t creation = 0, ticks = 0;
};
} // namespace monitoring
#ifdef _WIN32
#include "windows/process.hpp"
#else
#include "linux/process.hpp"
#endif
