#pragma once

#include "gpu_data.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <glibmm/main.h>
#include <gtkmm/label.h>
#include <mutex>
#include <string>
#include <thread>
#include <functional>

class GpuSummary : public Gtk::Label {
public:
  using Discovery = std::function<std::vector<GpuDevice>()>;
  explicit GpuSummary(Discovery discover = [] { auto d = discover_gpus(); community::gpu::nvml().augment(d); return d; }, const community::gpu::Nvml* provider = &community::gpu::nvml())
      : Gtk::Label("GPU and VRAM N/A"), mutex_(), cv_(), stop_(false),
        pending_(""), ready_(false) {
    set_xalign(0.0);
    set_line_wrap(true);
    set_selectable(true);
    set_margin_start(8);
    set_margin_end(8);
    set_margin_top(6);
    set_margin_bottom(6);

    timer_ = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &GpuSummary::poll), 250);

    worker_ = std::thread([this, discover=std::move(discover), provider] {
      while (!stop_.load()) {
        std::vector<GpuReading> readings;
        std::string text;
        try { text = gpu_text(discover(), &readings, provider); }
        catch (...) { text = "GPU telemetry unavailable; discovery will retry"; readings.clear(); }
        std::unique_lock<std::mutex> lock(mutex_);
        pending_ = std::move(text); pending_readings_ = std::move(readings); ready_ = true;
        cv_.wait_for(lock, std::chrono::seconds(2), [this] { return stop_.load(); });
      }
    });
  }

  ~GpuSummary() {
    timer_.disconnect();
    stop_ = true;
    cv_.notify_all();
    if (worker_.joinable()) {
      worker_.join();
    }
  }

public:
  sigc::signal<void, const std::vector<GpuReading> &> &signal_sample() {
    return sample_signal_;
  }

private:
  bool poll() {
    std::string text;
    std::vector<GpuReading> readings;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (!ready_)
        return true;
      text = pending_;
      readings = pending_readings_;
      ready_ = false;
    }
    const gchar *valid_text = g_utf8_make_valid(text.c_str(), -1);
set_text(valid_text);

std::string details;
for (const auto& r : readings) {
details += r.detail + "\n";
}
details += "Per-process GPU utilization and VRAM attribution are unavailable.";
gchar* valid_details = g_utf8_make_valid(details.data(), details.size());
set_tooltip_text(valid_details); g_free(valid_details);
    sample_signal_.emit(readings);
    g_free(const_cast<gchar *>(valid_text));
    return true;
  }

  std::mutex mutex_;
  std::condition_variable cv_;
  std::atomic<bool> stop_{false};
  std::string pending_;
  bool ready_ = false;
  sigc::connection timer_;
  std::thread worker_;
  std::vector<GpuReading> pending_readings_;
  sigc::signal<void, const std::vector<GpuReading> &> sample_signal_;
};
