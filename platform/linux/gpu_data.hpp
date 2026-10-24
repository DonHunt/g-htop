#include "../../presentation.hpp"
#include "../../gpu/enumerate_gpu.hpp"
#include "../../gpu/nvml_gpu.hpp"
#pragma once

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using GpuDevice = community::gpu::Device;
inline std::vector<GpuDevice> discover_gpus(
    const std::filesystem::path& drm="/sys/class/drm",
    const std::filesystem::path& pci="/sys/bus/pci/devices",
    const std::filesystem::path& database="/usr/share/hwdata/pci.ids") {
  return community::gpu::enumerate(drm,pci,database);
}

inline std::optional<unsigned long long> gpu_value(const std::string& path) {
  return community::gpu::integer(path);
}

inline std::string gpu_text(const std::vector<GpuDevice> &gpus,
                     std::vector<GpuReading> *readings = nullptr,
                     const community::gpu::Nvml* provider = nullptr) {
  if (readings != nullptr)
    readings->clear();
  if (gpus.empty())
    return "GPU: N/A — no readable PCI/DRM devices | VRAM: N/A";

  std::ostringstream oss;
  for (const auto &gpu : gpus) {
        std::string label = Ui::gpu_name(gpu.name) + " (" + gpu.card + ")";
    auto metrics = community::gpu::read(gpu);
    if (provider) {
      // An old NVML UUID must never acquire a replacement's sysfs counters.
      if (!gpu.uuid.empty() && provider->uuid_for(gpu) != gpu.uuid) metrics = {};
      else provider->read_into(gpu, metrics);
    }
    auto busy = metrics.utilization;
    auto vram_total = metrics.total, vram_used = metrics.used;
    if (readings) readings->push_back({label, busy ? *busy : -1.0,
        gpu.name + " | Vendor: " + gpu.vendor + " | Driver: " +
        (gpu.driver.empty() ? "unbound/unavailable" : gpu.driver) + " | PCI: " +
        gpu.pci_id + " | " + gpu.card + " | " + gpu.path,
        community::gpu::identity_id(gpu)});

    oss << label << " | GPU: ";
    if (busy && *busy <= 100) {
      oss << Ui::percent(*busy);
    } else {
      oss << "N/A";
    }
    oss << " | Temperature: ";
    if (metrics.temperature) oss << std::fixed << std::setprecision(1) << *metrics.temperature << " °C";
    else oss << "N/A";
    oss << " | Device/package power: ";
    if (metrics.power) oss << std::fixed << std::setprecision(1) << *metrics.power << " W";
    else oss << "N/A";
    oss << "\n";

    oss << std::fixed << std::setprecision(1);
    if (vram_used.has_value()) {
      oss << "VRAM Used: " << Ui::memory(vram_used.value() / 1048576.0);
    } else {
      oss << "VRAM Used: N/A";
    }
    oss << " | ";
    if (vram_total.has_value()) {
      oss << "Total: " << Ui::memory(vram_total.value() / 1048576.0);
    } else {
      oss << "Total: N/A";
    }
    oss << " | ";
    if (vram_used.has_value() && vram_total.has_value() &&
        vram_total.value() >= vram_used.value()) {
      oss << "Available: " << Ui::memory((*vram_total - *vram_used) / 1048576.0);
    } else {
      oss << "Available: N/A";
    }
    oss << "\n";
  }

  oss << "Per-process GPU / VRAM: N/A";
  return oss.str();
}
