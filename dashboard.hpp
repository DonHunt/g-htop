#ifndef DASHBOARD_HPP
#define DASHBOARD_HPP

#include <gtkmm.h>
#include "status_strip.hpp"
#include <glibmm/miscutils.h>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <sys/stat.h>

class Dashboard : public Gtk::Box {
public:
    Dashboard();
    void reset_layout();
    void save_layout();

public:
    Gtk::Box processes, services, gpu, cpu;
    Gtk::Button reset_button{"Reset Layout"}, fullscreen_button{"Full Screen (F11)"}, about_button{"About"};
    StatusStrip status;
    int normal_width = 1200, normal_height = 800;

private:
    Gtk::Box toolbar{Gtk::ORIENTATION_HORIZONTAL};
    Gtk::Paned columns{Gtk::ORIENTATION_HORIZONTAL}, left{Gtk::ORIENTATION_VERTICAL}, right{Gtk::ORIENTATION_VERTICAL};
    Gtk::Frame process_frame{"Processes"}, service_frame{"Services"}, gpu_frame{"GPU History / VRAM"}, cpu_frame{"CPU Threads"};

    double horizontal_fraction = 0.5, left_fraction = 0.5, right_fraction = 0.5;
    bool restored = false;

    void read_layout();
    void apply_positions();

    std::string config_dir;
    std::string layout_path;
};

Dashboard::Dashboard()
    : Gtk::Box(Gtk::ORIENTATION_VERTICAL), processes(Gtk::ORIENTATION_VERTICAL),
      services(Gtk::ORIENTATION_VERTICAL), gpu(Gtk::ORIENTATION_VERTICAL),
      cpu(Gtk::ORIENTATION_VERTICAL) {

process_frame.add(processes);
gpu_frame.add(gpu);
cpu_frame.add(cpu);
service_frame.add(services);
for (auto* box : {&processes, &services, &gpu, &cpu}) {
    box->set_margin_start(6);
    box->set_margin_end(6);
    box->set_margin_top(6);
    box->set_margin_bottom(6);
}
for (auto* frame : {&process_frame, &service_frame, &gpu_frame, &cpu_frame}) {
    frame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    if (frame->get_label_widget()) {
        frame->get_label_widget()->set_margin_start(6);
        frame->get_label_widget()->set_margin_top(3);
        frame->get_label_widget()->set_margin_bottom(3);
    }
}
set_margin_start(6);
set_margin_end(6);
set_spacing(6);
toolbar.set_spacing(8);
toolbar.set_margin_top(4);
toolbar.set_margin_bottom(4);toolbar.pack_start(reset_button, Gtk::PACK_SHRINK);
    toolbar.pack_start(fullscreen_button, Gtk::PACK_SHRINK);
    toolbar.pack_end(about_button, Gtk::PACK_SHRINK);

    left.pack1(process_frame, true, false);
    left.pack2(service_frame, true, false);
    right.pack1(gpu_frame, true, false);
    right.pack2(cpu_frame, true, false);

    columns.pack1(left, true, false);
    columns.pack2(right, true, false);

    pack_start(toolbar, Gtk::PACK_SHRINK);
    pack_start(columns, Gtk::PACK_EXPAND_WIDGET);
    pack_start(status, Gtk::PACK_SHRINK);




    columns.set_wide_handle(true);
    left.set_wide_handle(true);
    right.set_wide_handle(true);

    process_frame.set_size_request(280, 160);
    service_frame.set_size_request(280, 160);
    gpu_frame.set_size_request(280, 160);
    cpu_frame.set_size_request(280, 160);

    config_dir = Glib::get_user_config_dir() + "/G_htop";
    layout_path = config_dir + "/layout";

    g_mkdir_with_parents(config_dir.c_str(), 0700);

    columns.signal_size_allocate().connect([this](const Gtk::Allocation& allocation) {
        if (!restored && allocation.get_width() >= 560 &&
            left.get_allocation().get_height() >= 320 &&
            right.get_allocation().get_height() >= 320) {
            apply_positions();
        }
    });

    read_layout();
}

void Dashboard::reset_layout() {
    horizontal_fraction = 0.5;
    left_fraction = 0.5;
    right_fraction = 0.5;

    columns.set_position(columns.get_allocation().get_width() * horizontal_fraction);
    left.set_position(left.get_allocation().get_height() * left_fraction);
    right.set_position(right.get_allocation().get_height() * right_fraction);

    apply_positions();
}

void Dashboard::save_layout() {
    if (!restored) return;

    double h = static_cast<double>(columns.get_position()) / columns.get_allocation().get_width();
    double l = static_cast<double>(left.get_position()) / left.get_allocation().get_height();
    double r = static_cast<double>(right.get_position()) / right.get_allocation().get_height();

    std::ofstream out(layout_path);
    if (out.is_open()) {
        out << normal_width << " " << normal_height << " "
            << h << " " << l << " " << r;
    }
}

void Dashboard::read_layout() {
    std::ifstream in(layout_path);
    if (in.is_open()) {
        int w, h;
        double hf, lf, rf;
        if (in >> w >> h >> hf >> lf >> rf) {
            normal_width = std::clamp(w, 640, 10000);
            normal_height = std::clamp(h, 480, 10000);

            Gdk::Rectangle workarea = Gdk::Screen::get_default()->get_monitor_workarea(
                Gdk::Screen::get_default()->get_primary_monitor());
            int max_width = std::max(640, workarea.get_width() - 32);
            int max_height = std::max(480, workarea.get_height() - 32);

            normal_width = std::clamp(normal_width, 640, max_width);
            normal_height = std::clamp(normal_height, 480, max_height);

            horizontal_fraction = std::clamp(hf, 0.2, 0.8);
            left_fraction = std::clamp(lf, 0.2, 0.8);
            right_fraction = std::clamp(rf, 0.2, 0.8);
        }
    }
}

void Dashboard::apply_positions() {
    restored = true;
    columns.set_position(columns.get_allocation().get_width() * horizontal_fraction);
    left.set_position(left.get_allocation().get_height() * left_fraction);
    right.set_position(right.get_allocation().get_height() * right_fraction);
}

#endif // DASHBOARD_HPP
