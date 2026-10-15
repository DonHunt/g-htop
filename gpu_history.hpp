#pragma once

#include <chrono>
#include <deque>
#include <gtkmm/drawingarea.h>
#include <pango/pangocairo.h>
#include <set>
#include <string>
#include <vector>

#include "gpu_data.hpp"

class GpuHistory : public Gtk::DrawingArea {
public:
  GpuHistory() {
    set_size_request(320, 240);
    set_hexpand(true);
    set_vexpand(true);
  }

  void append_sample(const std::vector<GpuReading> &readings) {
    auto now = std::chrono::steady_clock::now();
    packets.push_back({now, readings});
    while (!packets.empty() &&
           std::chrono::duration<double>(now - packets.front().at).count() >
               300.0)
      packets.pop_front();
    while (packets.size() > 152)
      packets.pop_front();
    queue_draw();
  }

protected:
  bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override {
    auto allocation = get_allocation();
    int w = allocation.get_width();
    int h = allocation.get_height();

    if (w < 100 || h < 100) {
      draw_text(cr, "GPU utilization — last 5 minutes", 10, 10);
      return true;
    }

    render_background(cr, 0, 0, w, h);

    auto fg_color = get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
Gdk::RGBA accent_color;
if (!get_style_context()->lookup_color("theme_selected_bg_color", accent_color)) {
  accent_color = get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
}
accent_color.set_alpha(1.0);

Gdk::RGBA background;
if (!get_style_context()->lookup_color("theme_bg_color", background)) {
  background = get_style_context()->get_background_color(Gtk::STATE_FLAG_NORMAL);
}

auto luminance = [](const Gdk::RGBA& color) -> double {
  auto channel = [](double v) -> double { return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
  return 0.2126 * channel(color.get_red()) + 0.7152 * channel(color.get_green()) + 0.0722 * channel(color.get_blue());
};

double lumaAccent = luminance(accent_color);
double lumaBg = luminance(background);
double contrast = (std::max(lumaAccent, lumaBg) + 0.05) / (std::min(lumaAccent, lumaBg) + 0.05);

if (contrast < 3.0) {
  accent_color = get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
}    cr->set_source_rgba(fg_color.get_red(), fg_color.get_green(),
                        fg_color.get_blue(), 0.15);
    draw_grid(cr, w, h);

    cr->set_source_rgba(fg_color.get_red(), fg_color.get_green(),
                        fg_color.get_blue(), 1.0);
    draw_text(cr, "GPU utilization — last 5 minutes", 10, 10);

    std::set<std::string> series_names;
    for (const auto &packet : packets) {
      for (const auto &reading : packet.readings) {
        if (reading.usage >= 0 && reading.usage <= 100) {
          series_names.insert(reading.name);
        }
      }
    }

    int legend_y = 34;
    Gdk::RGBA legend_color = accent_color;
    for (const auto &name : series_names) {
      draw_legend(cr, name, legend_color, legend_y);
      legend_y += 20;
      legend_color = get_next_accent_color(legend_color);
    }

    if (packets.empty() ||
        std::none_of(packets.back().readings.begin(),
                     packets.back().readings.end(), [](const GpuReading &r) {
                       return r.usage >= 0 && r.usage <= 100;
                     })) {
      draw_text(cr, "GPU: N/A — telemetry unavailable", w / 2 - 100, h / 2);
      return true;
    }

    cr->set_line_width(2.0);

    {
      cr->save();
      cr->rectangle(58, 60, w - 78, h - 98);
      cr->clip();

      for (const auto &name : series_names) {
        cr->set_source_rgba(accent_color.get_red(), accent_color.get_green(),
                            accent_color.get_blue(), accent_color.get_alpha());
        cr->set_line_width(2);
        cr->begin_new_path();
        bool first = true;

        for (const auto &packet : packets) {
          auto age = std::chrono::duration<double>(
                         std::chrono::steady_clock::now() - packet.at)
                         .count();
          auto it = std::find_if(
              packet.readings.begin(), packet.readings.end(),
              [&name](const GpuReading &r) { return r.name == name; });

          if (it == packet.readings.end() || it->usage < 0 || it->usage > 100 ||
              age < 0 || age > 300) {
            cr->stroke();
            first = true;
            continue;
          }

          double x = map_x(w, packet.at);
          double y = map_y(h, it->usage);

          if (first) {
            cr->move_to(x, y);
            first = false;
          } else {
            cr->line_to(x, y);
          }
        }

        cr->stroke();
        accent_color = get_next_accent_color(accent_color);
      }

      cr->restore();
    }

    return true;
  }

private:
  struct Packet {
    std::chrono::steady_clock::time_point at;
    std::vector<GpuReading> readings;
  };

  std::deque<Packet> packets;

  void render_background(const Cairo::RefPtr<Cairo::Context> &cr, int x, int y,
                         int w, int h) {
    get_style_context()->render_background(cr, x, y, w, h);
  }

  void draw_grid(const Cairo::RefPtr<Cairo::Context> &cr, int w, int h) {
    auto grid_color = get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
    grid_color.set_alpha(0.15);
    cr->set_source_rgba(grid_color.get_red(), grid_color.get_green(),
                        grid_color.get_blue(), grid_color.get_alpha());
    cr->set_line_width(1);

    for (int i = 0; i <= 100; i += 25) {
      double y = map_y(h, i);
      cr->move_to(58, y);
      cr->line_to(w - 20, y);
    }

    for (int i = 0; i <= 5; ++i) {
      double x = 58 + (w - 78) * i / 5.0;
      cr->move_to(x, 60);
      cr->line_to(x, h - 38);
    }

    cr->stroke();

    for (int i = 0; i <= 100; i += 25) {
      double y = map_y(h, i);
      draw_text(cr, std::to_string(i) + "%", 10, y - 8);
    }

    std::vector<std::string> xlabels = {"-5m", "-4m", "-3m",
                                        "-2m", "-1m", "Now"};
    for (int i = 0; i <= 5; ++i) {
      double x = 58 + (w - 78) * i / 5.0;
      draw_text(cr, xlabels[i], (i == 5) ? x - 30 : x - 14, h - 28);
    }
  }

  void draw_legend(const Cairo::RefPtr<Cairo::Context> &cr,
                   const std::string &name, const Gdk::RGBA &color, int y) {
    cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(),
                        color.get_alpha());
    cr->move_to(10, y + 8);
    cr->line_to(30, y + 8);
    cr->stroke();

    if (!packets.empty()) {
      auto it = std::find_if(
          packets.back().readings.begin(), packets.back().readings.end(),
          [&name](const GpuReading &r) { return r.name == name; });
      if (it != packets.back().readings.end() && it->usage >= 0 &&
          it->usage <= 100) {
        draw_text(cr, name + " | " + Ui::percent(it->usage), 40, y);
      } else {
        draw_text(cr, name + " | N/A", 40, y);
      }
    } else {
      draw_text(cr, name + " | N/A", 40, y);
    }
  }

  void draw_text(const Cairo::RefPtr<Cairo::Context> &cr,
                 const std::string &text, int x, int y) {
    auto fg_color = get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
    cr->set_source_rgba(fg_color.get_red(), fg_color.get_green(),
                        fg_color.get_blue(), fg_color.get_alpha());
    auto layout = create_pango_layout(text);
    layout->set_width(std::max(1, get_allocation().get_width() - x - 8) *
                      Pango::SCALE);
    layout->set_ellipsize(Pango::ELLIPSIZE_END);
    cr->move_to(x, y);
    layout->show_in_cairo_context(cr);
  }

  double map_x(int w, std::chrono::steady_clock::time_point at) {
    auto age =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - at)
            .count();
    return 58 + (w - 78) * (1 - age / 300.0);
  }

  double map_y(int h, double usage) {
    return h - 38 - (h - 98) * (usage / 100.0);
  }

  Gdk::RGBA get_next_accent_color(const Gdk::RGBA &current) {
    Gdk::RGBA next;
    next.set_rgba(current.get_green(), current.get_blue(), current.get_red(),
                  1.0);
    return next;
  }
};
