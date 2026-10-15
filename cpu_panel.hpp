#pragma once

#include <algorithm>
#include <fstream>
#include <gtkmm.h>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

class CpuPanel : public Gtk::Box {
public:
  CpuPanel() : Gtk::Box(Gtk::ORIENTATION_VERTICAL, 8) {
    header.set_text("Detected: 0, Sample: 0");
    pack_start(header, false, true, 0);

    scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    pack_start(scroll, true, true, 0);

    flow.set_selection_mode(Gtk::SELECTION_NONE);
    flow.set_valign(Gtk::ALIGN_START);
    flow.set_homogeneous(true);
    flow.set_min_children_per_line(1);
    flow.set_max_children_per_line(64);
    flow.set_row_spacing(8);
    flow.set_column_spacing(8);
    flow.set_margin_start(8);
    flow.set_margin_end(8);
    flow.set_margin_top(8);
    flow.set_margin_bottom(8);

    scroll.add(flow);

    auto css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(R"(
            progressbar.ghtop-cpu.ghtop-green progress {
                background-image: none;
                background-color: #2eae4f;
                border-color: #2eae4f;
            }
            progressbar.ghtop-cpu.ghtop-yellow progress {
                background-image: none;
                background-color: #e6c229;
                border-color: #e6c229;
            }
            progressbar.ghtop-cpu.ghtop-red progress {
                background-image: none;
                background-color: #e34b4b;
                border-color: #e34b4b;
            }
        )");

    Gtk::StyleContext::add_provider_for_screen(
        Gdk::Screen::get_default(), css_provider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    refresh();
    timer = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &CpuPanel::refresh), 2000);
  }

  ~CpuPanel() {
    if (timer.connected()) {
      timer.disconnect();
    }
  }

private:
  Gtk::Label header;
  Gtk::ScrolledWindow scroll;
  Gtk::FlowBox flow;
  class Meter;
  std::map<unsigned, Meter *> meters;
  struct Counters {
    unsigned long long total = 0;
    unsigned long long idle = 0;
  };
  std::map<unsigned, Counters> previous;
  sigc::connection timer;
  unsigned sample = 0;

  static auto read_cpus() -> std::map<unsigned, Counters> {
    std::map<unsigned, Counters> result;
    std::ifstream file("/proc/stat");
    if (!file.is_open())
      return result;

    std::string line;
    while (std::getline(file, line)) {
      if (line.find("cpu ") == 0)
        continue; // Skip aggregate cpu
      if (line.find("cpu") != 0 || !isdigit(line[3]))
        break; // Stop on non-cpu line

      unsigned id = std::stoi(line.substr(3));
      Counters counters;
      unsigned long long array[8] = {0};
      std::istringstream iss(line);
      std::string ignored;
      iss >> ignored;
      bool valid = true;
      for (int i = 0; i < 8; ++i) {
        if (!(iss >> array[i])) {
          valid = false;
          break;
        }
      }
      if (!valid)
        continue;
      counters.total = 0;
      for (int i = 0; i < 8; ++i) {
        counters.total += array[i];
      }
      counters.idle = array[3] + array[4];

      result[id] = counters;
    }

    return result;
  }

  bool refresh() {
    auto current = read_cpus();
    ++sample;

    for (const auto &[id, _] : current) {
      if (!meters.count(id)) {
        meters[id] = Gtk::manage(new Meter(id));
        flow.add(*meters[id]);
        meters[id]->show_all();
      }
    }

    for (auto &[id, meter] : meters) {
      double percent = -1.0;
      if (current.count(id) && previous.count(id) &&
          current[id].total >= previous[id].total &&
          current[id].idle >= previous[id].idle) {
        auto dt = current[id].total - previous[id].total;
        auto di = current[id].idle - previous[id].idle;
        if (dt > 0 && di <= dt) {
          percent = 100.0 * (dt - di) / dt;
        }
      }
      meter->update(percent);
    }

    previous = current;

    header.set_text("Detected: " + std::to_string(current.size()) +
                    ", Sample: " + std::to_string(sample));

    return true;
  }

  class Meter : public Gtk::Box {
  public:
// Constructor for Meter class
Meter(unsigned id) : Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 6),
    name("CPU " + std::to_string(id)), value("N/A") {

    // Set properties for the name label
    name.set_width_chars(6);
    name.set_xalign(0.0);

    // Set properties for the progress bar
    bar.set_size_request(80, -1);
    bar.set_hexpand(true);
    bar.set_valign(Gtk::ALIGN_CENTER);
    bar.get_style_context()->add_class("ghtop-cpu");
    bar.set_show_text(false);

    // Set properties for the value label
    value.set_width_chars(6);
    value.set_xalign(1.0);

    // Pack widgets into the box
    pack_start(name, false, true, 0);
    pack_start(bar, true, true, 0);
    pack_start(value, false, true, 0);

    // Set size request for the entire Meter widget
    set_size_request(210, -1);
}
    void update(double percent) {
      bar.set_fraction(0.0);
      value.set_text("N/A");
      bar.get_style_context()->remove_class("ghtop-green");
      bar.get_style_context()->remove_class("ghtop-yellow");
      bar.get_style_context()->remove_class("ghtop-red");

      if (percent >= 0) {
        percent = std::clamp(percent, 0.0, 100.0);
        bar.set_fraction(percent / 100.0);
        std::ostringstream out;
        out << std::fixed << std::setprecision(1) << percent << "%";
        value.set_text(out.str());
        if (percent <= 33) {
          bar.get_style_context()->add_class("ghtop-green");
        } else if (percent < 67) {
          bar.get_style_context()->add_class("ghtop-yellow");
        } else {
          bar.get_style_context()->add_class("ghtop-red");
        }
      }
    }

  private:
    Gtk::Label name;
    Gtk::ProgressBar bar;
    Gtk::Label value;
  };
};
