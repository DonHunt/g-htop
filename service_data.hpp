#pragma once
#include <atomic>
#include <string>
#include <vector>
struct ServiceRow {
  std::string description;
  std::string unit;
  std::string state;
  std::string substate;
  std::string scope;
  unsigned long pid = 0;
};


#ifdef _WIN32
#include "platform/windows/service_data.hpp"
#else
#include "platform/linux/service_data.hpp"
#endif
