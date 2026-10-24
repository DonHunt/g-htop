#pragma once
#include "../../presentation.hpp"
#include "common.hpp"
#include "pdh.hpp"
#include <algorithm>
#include <dxgi1_2.h>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
// Keep the Linux injection seam; no vendor runtime is needed on Windows.
namespace community::gpu {
struct Nvml {};
inline const Nvml &nvml() {
  static Nvml unused;
  return unused;
}
} // namespace community::gpu
struct GpuDevice {
  std::string name, identity;
  LUID luid{};
  SIZE_T dedicated = 0;
};
inline std::string gpu_identity(LUID luid) {
  char text[32];
  std::snprintf(text, sizeof(text), "luid_0x%08x_0x%08x",
                unsigned(luid.HighPart), unsigned(luid.LowPart));
  return text;
}
inline std::vector<GpuDevice> discover_gpus() {
  std::vector<GpuDevice> result;
  IDXGIFactory1 *factory = nullptr;
  if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1),
                                reinterpret_cast<void **>(&factory))))
    return result;
  struct Release {
    IDXGIFactory1 *value;
    ~Release() { value->Release(); }
  } release{factory};
  for (UINT i = 0;; ++i) {
    IDXGIAdapter1 *adapter = nullptr;
    auto status = factory->EnumAdapters1(i, &adapter);
    if (status == DXGI_ERROR_NOT_FOUND)
      break;
    if (FAILED(status))
      break;
    DXGI_ADAPTER_DESC1 description{};
    status = adapter->GetDesc1(&description);
    adapter->Release();
    if (FAILED(status) || (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
      continue;
    result.push_back({monitoring::win::utf8(description.Description),
                      gpu_identity(description.AdapterLuid),
                      description.AdapterLuid,
                      description.DedicatedVideoMemory});
  }
  return result;
}
namespace monitoring::win {
inline std::optional<std::string>
adapter_identity(const std::wstring &instance) {
  auto pos = instance.find(L"luid_0x");
  if (pos == std::wstring::npos)
    return std::nullopt;
  unsigned high = 0, low = 0;
  if (swscanf(instance.c_str() + pos, L"luid_0x%x_0x%x", &high, &low) != 2)
    return std::nullopt;
  return gpu_identity(LUID{DWORD(low), LONG(high)});
}
class GpuCounters {
  Counter engines{L"\\GPU Engine(*)\\Utilization Percentage"};
  Counter memory{L"\\GPU Adapter Memory(*)\\Dedicated Usage"};

public:
  struct Metrics {
    std::optional<double> utilization, used;
  };
  std::map<std::string, Metrics> sample() {
    std::map<std::string, Metrics> result;
    std::map<std::string, std::map<std::wstring, double>> engine_totals;
    for (const auto &[instance, value] : engines.sample()) {
      auto identity = adapter_identity(instance);
      auto engine = instance.find(L"_phys_");
      if (identity && engine != std::wstring::npos)
        engine_totals[*identity][instance.substr(engine)] += value;
    }
    // Sum processes sharing an engine, then report the busiest engine as
    // Windows Task Manager does. Different engines may run simultaneously.
    for (const auto &[id, engines] : engine_totals) {
      double maximum = 0;
      for (const auto &[name, total] : engines)
        maximum = std::max(maximum, total);
      result[id].utilization = std::clamp(maximum, 0.0, 100.0);
    }
    for (const auto &[instance, value] : memory.sample()) {
      auto identity = adapter_identity(instance);
      if (identity)
        result[*identity].used = result[*identity].used.value_or(0) + value;
    }
    return result;
  }
};
} // namespace monitoring::win
inline std::string gpu_text(const std::vector<GpuDevice> &devices,
                            std::vector<GpuReading> *readings = nullptr,
                            const community::gpu::Nvml * = nullptr) {
  if (readings)
    readings->clear();
  if (devices.empty())
    return "GPU: N/A — no readable DXGI hardware adapters | VRAM: N/A";
  // The normal caller is the single GPU worker; thread-local also keeps test
  // collectors independent and guarantees PDH thread ownership.
  thread_local monitoring::win::GpuCounters counters;
  auto metrics = counters.sample();
  std::ostringstream out;
  for (const auto &device : devices) {
    const auto &metric = metrics[device.identity];
    auto name = Ui::gpu_name(device.name);
    out << name << " | GPU: "
        << (metric.utilization ? Ui::percent(*metric.utilization) : "N/A")
        << " | Temperature: N/A | Device/package power: N/A\n";
    out << "VRAM Used: "
        << (metric.used ? Ui::memory(*metric.used / 1048576.0) : "N/A")
        << " | Total: "
        << (device.dedicated ? Ui::memory(device.dedicated / 1048576.0) : "N/A")
        << " | Available: ";
    if (metric.used && device.dedicated && *metric.used <= device.dedicated)
      out << Ui::memory((device.dedicated - *metric.used) / 1048576.0);
    else
      out << "N/A";
    out << "\n";
    if (readings)
      readings->push_back(
          {name, metric.utilization.value_or(-1),
           device.name + " | DXGI: " + device.identity +
               " | Windows WDDM performance counters; temperature/power N/A",
           device.identity});
  }
  out << "Per-process GPU / VRAM: N/A";
  return out.str();
}
