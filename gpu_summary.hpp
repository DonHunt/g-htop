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

class GpuSummary : public Gtk::Label {
public:
  GpuSummary()
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

    worker_ = std::thread([this] {
      try {
        auto devices = discover_gpus();
        while (!stop_.load()) {
          try {
            std::vector<GpuReading> readings;
            std::string text = gpu_text(devices, &readings);
            {
              std::lock_guard<std::mutex> lock(mutex_);
              pending_ = text;
              pending_readings_ = readings;
              ready_ = true;
            }
          } catch (...) {
            // Failure, do nothing
          }
          std::unique_lock<std::mutex> lock(mutex_);
          cv_.wait_for(lock, std::chrono::seconds(2),
                       [this] { return stop_.load(); });
        }
      } catch (...) {
        // Discovery failure, use empty vector
        auto devices = std::vector<GpuDevice>{};
        while (!stop_.load()) {
          try {
            std::vector<GpuReading> readings;
            std::string text = gpu_text(devices, &readings);
            {
              std::lock_guard<std::mutex> lock(mutex_);
              pending_ = text;
              pending_readings_ = readings;
              ready_ = true;
            }
          } catch (...) {
            // Failure, do nothing
          }
          std::unique_lock<std::mutex> lock(mutex_);
          cv_.wait_for(lock, std::chrono::seconds(2),
                       [this] { return stop_.load(); });
        }
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
set_tooltip_text(details);
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
