#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <deque>
#include <gtkmm/drawingarea.h>
#include <map>
#include <pango/pangocairo.h>
#include <string>
#include <vector>
#include "gpu_data.hpp"

// One shared utilization plot; series belong to physical identities, not names.
class GpuHistory : public Gtk::DrawingArea {
public:
  GpuHistory() {
    set_size_request(280, 160);
    set_hexpand(true);
    set_vexpand(true);
  }

  static std::string series_key(const GpuReading& r) {
    return r.identity.empty() ? r.name : r.identity;
  }

  void append_sample(const std::vector<GpuReading>& readings) {
    const auto now = Clock::now();
    std::map<std::string, Series> next;
    for (const auto& r : readings) {
      const auto key = series_key(r);
      auto found = series_.find(key);
      Series s = found == series_.end() ? Series{} : std::move(found->second);
      if (!colors_.count(key)) colors_[key] = colors_.size();
      s.reading = r;
      s.points.push_back({now, r.usage});
      while (!s.points.empty() &&
             (std::chrono::duration<double>(now - s.points.front().at).count() > 300 ||
              s.points.size() > 152)) s.points.pop_front();
      next.emplace(key, std::move(s));
    }
    series_ = std::move(next);
    // Only the legend grows, never one chart per GPU.
    set_size_request(280, std::max(160, 80 + static_cast<int>(series_.size()) * 20));
    queue_draw();
  }

  size_t sample_count() const {
    size_t count = 0;
    for (const auto& entry : series_) count = std::max(count, entry.second.points.size());
    return count;
  }
  size_t series_count() const { return series_.size(); }
  size_t series_sample_count(const std::string& key) const {
    auto it = series_.find(key);
    return it == series_.end() ? 0 : it->second.points.size();
  }
  std::array<double, 3> series_color(const std::string& key) const {
    // Golden-angle hues continue beyond the explicit high-contrast palette.
    static constexpr std::array<std::array<double, 3>, 8> palette{{
      {{0.0, 0.45, 0.70}}, {{0.90, 0.40, 0.0}}, {{0.0, 0.62, 0.45}},
      {{0.80, 0.35, 0.65}}, {{0.55, 0.45, 0.0}}, {{0.30, 0.65, 0.85}},
      {{0.80, 0.25, 0.20}}, {{0.50, 0.40, 0.80}}
    }};
    const auto index = colors_.at(key);
    if (index < palette.size()) return palette[index];
    const double hue = std::fmod((index - palette.size()) * 0.61803398875 + 0.08, 1.0) * 6;
    const double x = 0.65 * (1 - std::abs(std::fmod(hue, 2.0) - 1));
    const double m = 0.15;
    switch (static_cast<int>(hue)) {
      case 0: return {0.8, x + m, m};
      case 1: return {x + m, 0.8, m};
      case 2: return {m, 0.8, x + m};
      case 3: return {m, x + m, 0.8};
      case 4: return {x + m, m, 0.8};
      default: return {0.8, m, x + m};
    }
  }

protected:
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override {
    const int w = get_allocated_width(), h = get_allocated_height();
    get_style_context()->render_background(cr, 0, 0, w, h);
    text(cr, "GPU utilization — last 5 minutes", 10, 6);
    const double top = 32 + 20 * series_.size(), bottom = h - 26;
    int row = 30;
    unsigned gpu_number = 0;
    for (const auto& entry : series_) {
      const auto color = series_color(entry.first);
      cr->set_source_rgb(color[0], color[1], color[2]);
      cr->set_line_width(3);
      cr->move_to(10, row + 8); cr->line_to(30, row + 8); cr->stroke();
      const auto& r = entry.second.reading;
      text(cr, "GPU " + std::to_string(++gpu_number) + ": " + r.name + " | " +
           (valid(r.usage) ? Ui::percent(r.usage) : "N/A"), 38, row);
      row += 20;
    }
    if (series_.empty()) {
      text(cr, "GPU: N/A — no detected devices", 10, 32);
      return true;
    }
    if (w < 100 || bottom - top < 20) return true;
    auto y = [&](double usage) { return bottom - (bottom - top) * usage / 100.0; };
    const auto fg = get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
    cr->set_source_rgba(fg.get_red(), fg.get_green(), fg.get_blue(), 0.18);
    cr->set_line_width(1);
    for (int i = 0; i <= 100; i += 25) {
      cr->move_to(48, y(i)); cr->line_to(w - 12, y(i));
    }
    for (int i = 0; i <= 5; ++i) {
      double x = 48 + (w - 60) * i / 5.0;
      cr->move_to(x, top); cr->line_to(x, bottom);
    }
    cr->stroke();
    for (int i = 0; i <= 100; i += 25) text(cr, std::to_string(i) + "%", 2, y(i) - 8);
    for (int i = 0; i <= 5; ++i) {
      double x = 48 + (w - 60) * i / 5.0;
      text(cr, i == 5 ? "Now" : "-" + std::to_string(5 - i) + "m", x - (i == 5 ? 26 : 12), bottom + 5);
    }
    cr->save(); cr->rectangle(47, top - 2, w - 58, bottom - top + 4); cr->clip();
    const auto now = Clock::now();
    for (const auto& entry : series_) {
      const auto color = series_color(entry.first);
      cr->set_source_rgb(color[0], color[1], color[2]); cr->set_line_width(2);
      bool started = false;
      for (const auto& point : entry.second.points) {
        double age = std::chrono::duration<double>(now - point.at).count();
        if (!valid(point.usage) || age > 300 || age < 0) {
          cr->stroke(); started = false; continue;
        }
        double x = 48 + (w - 60) * (1 - age / 300.0);
        if (!started) {
          // A lone measurement is visible, including a genuine zero.
          cr->arc(x, y(point.usage), 2, 0, 6.283185307); cr->fill();
          cr->move_to(x, y(point.usage)); started = true;
        } else cr->line_to(x, y(point.usage));
      }
      cr->stroke();
    }
    cr->restore();
    return true;
  }

private:
  using Clock = std::chrono::steady_clock;
  struct Point { Clock::time_point at; double usage; };
  struct Series { GpuReading reading; std::deque<Point> points; };
  std::map<std::string, Series> series_;
  std::map<std::string, size_t> colors_;
  static bool valid(double usage) { return std::isfinite(usage) && usage >= 0 && usage <= 100; }
  void text(const Cairo::RefPtr<Cairo::Context>& cr, const std::string& value, int x, int y) {
    const auto fg = get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
    cr->set_source_rgba(fg.get_red(), fg.get_green(), fg.get_blue(), fg.get_alpha());
    auto layout = create_pango_layout(value);
    layout->set_width(std::max(1, get_allocated_width() - x - 6) * Pango::SCALE);
    layout->set_ellipsize(Pango::ELLIPSIZE_END);
    cr->move_to(x, y); layout->show_in_cairo_context(cr);
  }
};
