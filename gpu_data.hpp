#pragma once
#include <string>
#include <vector>
struct GpuReading {
    std::string name;
    double usage;
    std::string detail;
    std::string identity;
};
#ifdef _WIN32
#include "platform/windows/gpu_data.hpp"
#else
#include "platform/linux/gpu_data.hpp"
#endif
