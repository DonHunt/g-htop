#include "presentation.hpp"
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

struct GpuDevice {
  std::string card, path, name;
};

std::vector<GpuDevice> discover_gpus() {
  std::vector<GpuDevice> gpus;
  std::error_code ec;
  if (!std::filesystem::exists("/sys/class/drm", ec))
    return {};

  std::regex card_regex(R"(card\d+$)");
  std::set<std::string> seen_paths;

  for (const auto &entry :
       std::filesystem::directory_iterator("/sys/class/drm", ec)) {
    if (ec)
      continue;
    const auto &path = entry.path();
    if (!std::filesystem::is_symlink(path, ec))
      continue;
    if (!std::regex_match(path.filename().string(), card_regex))
      continue;

    std::error_code symlink_ec;
    auto driver_path =
        std::filesystem::read_symlink(path / "device" / "driver", symlink_ec);
    if (symlink_ec || driver_path.filename() != "amdgpu")
      continue;

    auto canonical_path = std::filesystem::canonical(path / "device", ec);
    if (ec)
      continue;
    if (!seen_paths.insert(canonical_path.string()).second)
      continue;

    GpuDevice gpu;
    gpu.path = canonical_path.string();
    gpu.card = path.filename().string();

    std::string bdf = canonical_path.filename().string();
    std::smatch match;
    if (std::regex_match(
            bdf, match,
            std::regex(
                R"([0-9a-fA-F]{4}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}\.[0-7])"))) {
      FILE *pipe =
          popen(("LC_ALL=C timeout 2s lspci -D -s " + bdf).c_str(), "r");
      if (pipe) {
        std::string line;
        char buffer[512];
        while (fgets(buffer, sizeof(buffer), pipe)) {
          line += buffer;
        }
        auto pos = line.find(": ");
        if (pos != std::string::npos) {
          gpu.name = line.substr(pos + 2);
          while (!gpu.name.empty() &&
                 (gpu.name.back() == '\n' || gpu.name.back() == '\r')) {
            gpu.name.pop_back();
          }
        }
        pclose(pipe);
      }
    }

    if (gpu.name.empty()) {
      gpu.name = "AMD GPU";
    }

    gpus.push_back(gpu);
  }

  return gpus;
}

std::optional<unsigned long long> gpu_value(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open() || !file.good())
    return std::nullopt;

  std::string value_str;
  file >> value_str;
  if (value_str.empty() ||
      !std::all_of(value_str.begin(), value_str.end(), ::isdigit))
    return std::nullopt;

  try {
    return std::stoull(value_str);
  } catch (...) {
    return std::nullopt;
  }
}

struct GpuReading {
    std::string name;
    double usage;
    std::string detail;
};
std::string gpu_text(const std::vector<GpuDevice> &gpus,
                     std::vector<GpuReading> *readings = nullptr) {
  if (readings != nullptr)
    readings->clear();
  if (gpus.empty())
    return "AMD GPU: N/A | VRAM: N/A";

  std::ostringstream oss;
  for (const auto &gpu : gpus) {
        std::string label = Ui::gpu_name(gpu.name) + " (" + gpu.card + ")";
    auto busy = gpu_value(gpu.path + "/gpu_busy_percent");
    if (readings) {
      if (busy && *busy <= 100)
        readings->push_back(
            {label, static_cast<double>(*busy), gpu.name + " | " + gpu.card + " | " + gpu.path});
      else
        readings->push_back({label, -1.0, gpu.name + " | " + gpu.card + " | " + gpu.path});
    }
    auto vram_total = gpu_value(gpu.path + "/mem_info_vram_total");
    auto vram_used = gpu_value(gpu.path + "/mem_info_vram_used");

    oss << label << " | GPU: ";
    if (busy && *busy <= 100) {
      oss << Ui::percent(*busy);
    } else {
      oss << "N/A";
    }
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
