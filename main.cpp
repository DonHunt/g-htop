#include <unistd.h>
#include <cmath>
#include <iterator>
#include <vector>
#include <cctype>
#include <cstdio>
#include <gtkmm/application.h>
#include <gtkmm/notebook.h>
#include "services_view.hpp"
#include "gpu_summary.hpp"
#include "cpu_panel.hpp"
#include "connections_view.hpp"
#include <gtkmm/notebook.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>
#include <gdk/gdkkeysyms.h>
#include "gpu_history.hpp"
#include "dashboard.hpp"
#include <glibmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/liststore.h>
#include <gtkmm/label.h>
#include <glibmm/ustring.h>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <map>
#include <cstring>

class ProcessWindow : public Gtk::Window {
public:
    ProcessWindow() : m_vbox(Gtk::ORIENTATION_VERTICAL, false), m_scrolled_window(),  m_footer_label("Processes: 0, Sample: 0") {
        set_title("G_htop");
set_default_size(m_dashboard.normal_width, m_dashboard.normal_height);
        m_vbox.pack_start(m_scrolled_window, true, true);
        m_scrolled_window.add(m_tree_view);

        m_refTreeModel = Gtk::ListStore::create(m_columns);
        m_tree_view.set_model(m_refTreeModel);

        m_tree_view.append_column("Name", m_columns.name);
        m_tree_view.get_column(0)->set_sort_column(m_columns.name);
        m_tree_view.append_column("PID", m_columns.pid);
        m_tree_view.get_column(1)->set_sort_column(m_columns.pid);
        m_tree_view.append_column("CPU %", m_columns.cpu);
        m_tree_view.get_column(2)->set_sort_column(m_columns.cpu);
        m_tree_view.append_column("Memory MiB", m_columns.memory);
        m_tree_view.get_column(3)->set_sort_column(m_columns.memory);

        for (int i = 0; i < 4; ++i) {
            auto column = m_tree_view.get_column(i);
            column->set_resizable(true);
            column->set_expand(i == 0);
        }

// Assuming the constructor is already defined and columns are configured
Ui::table(m_tree_view, {1, 2, 3});
Ui::number(m_tree_view, 2, m_columns.cpu, true);
Ui::number(m_tree_view, 3, m_columns.memory, false);
m_tree_view.get_column(2)->set_title("CPU");
m_tree_view.get_column(3)->set_title("Memory");
m_tree_view.set_tooltip_column(m_columns.name.index());
m_tree_view.get_column(0)->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
m_tree_view.get_column(1)->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
m_tree_view.get_column(2)->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
m_tree_view.get_column(3)->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
m_tree_view.get_column(0)->set_fixed_width(160);
m_tree_view.get_column(1)->set_fixed_width(76);
m_tree_view.get_column(2)->set_fixed_width(76);
m_tree_view.get_column(3)->set_fixed_width(110);
m_tree_view.get_column(0)->set_min_width(100);
m_tree_view.get_column(1)->set_min_width(60);
m_tree_view.get_column(2)->set_min_width(60);
m_tree_view.get_column(3)->set_min_width(96);
m_tree_view.get_column(0)->set_expand(true);
m_tree_view.get_column(1)->set_expand(false);
m_tree_view.get_column(2)->set_expand(false);
m_tree_view.get_column(3)->set_expand(false);
auto renderer = dynamic_cast<Gtk::CellRendererText*>(m_tree_view.get_column(0)->get_first_cell());
renderer->property_ellipsize() = Pango::ELLIPSIZE_END;

m_refTreeModel->set_sort_column(m_columns.cpu, Gtk::SORT_DESCENDING);

        m_vbox.pack_start(m_footer_label, false, false);
// Append pages to notebook
m_notebook.append_page(m_dashboard, "Dashboard");
m_notebook.append_page(m_process_page, "Processes");
m_notebook.append_page(m_service_page, "Services");
m_notebook.append_page(m_cpu_page, "CPU Threads");
m_notebook.append_page(m_connections, "Connections");
m_notebook.append_page(m_history_page, "GPU History");

// Make notebook scrollable
m_notebook.set_scrollable(true);

// Pack notebook into root box
m_root.pack_start(m_notebook, Gtk::PACK_EXPAND_WIDGET);

// Add root box to the window
add(m_root);

// Connect notebook signal_switch_page
m_notebook.signal_switch_page().connect([this](Gtk::Widget* widget, guint page) {
  route_views(page);
});

// Call route_views for initial setup
route_views(0);

// Set footer label ellipsize mode
m_footer_label.set_ellipsize(Pango::ELLIPSIZE_END);

// Connect m_gpu.signal_sample to GpuHistory::append_sample
m_gpu.signal_sample().connect(sigc::mem_fun(m_history, &GpuHistory::append_sample));

// Connect m_gpu.signal_sample to lambda for status update
m_gpu.signal_sample().connect([this](const std::vector<GpuReading>& values) {
  m_gpu_seen = true;
  m_gpu_valid = !values.empty() && std::all_of(values.begin(), values.end(), [](const GpuReading& r) { return r.usage >= 0 && r.usage <= 100; });
  update_status();
});

// Connect reset_button clicked to m_dashboard.reset_layout
m_dashboard.reset_button.signal_clicked().connect([this]() {
  m_dashboard.reset_layout();
});

// Connect fullscreen_button clicked to ProcessWindow::toggle_fullscreen
m_dashboard.fullscreen_button.signal_clicked().connect([this]() {
  toggle_fullscreen();
});show_all_children();

        refresh();
        m_timeout_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &ProcessWindow::refresh), 2000);
    }

    ~ProcessWindow() {
    m_dashboard.save_layout();
        if (m_timeout_connection.connected()) {
            m_timeout_connection.disconnect();
        }
    }

protected:
bool on_key_press_event(GdkEventKey* event) override {
    if (event->keyval == GDK_KEY_F11) {
        toggle_fullscreen();
        return true;
    } else if (event->keyval == GDK_KEY_Escape && m_fullscreen) {
        unfullscreen();
        m_fullscreen = false;
        return true;
    }
    return Gtk::Window::on_key_press_event(event);
}

bool on_window_state_event(GdkEventWindowState* event) override {
    GdkWindowState new_window_state = static_cast<GdkWindowState>(event->new_window_state);
    m_fullscreen = (new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;
    update_status();
    return Gtk::Window::on_window_state_event(event);
}

bool on_configure_event(GdkEventConfigure* event) override {
    if (!m_fullscreen && get_window() && !(get_window()->get_state() & (GDK_WINDOW_STATE_FULLSCREEN | GDK_WINDOW_STATE_MAXIMIZED)) && event->width >= 640 && event->height >= 480) {
        m_dashboard.normal_width = event->width;
        m_dashboard.normal_height = event->height;
    }
    return Gtk::Window::on_configure_event(event);
}

private:
static void place(Gtk::Widget& widget, Gtk::Box& target, bool expand = true) {
    if (widget.get_parent() == &target) return;
    auto old_parent = dynamic_cast<Gtk::Container*>(widget.get_parent());
    if (old_parent) old_parent->remove(widget);
    target.pack_start(widget, expand ? Gtk::PACK_EXPAND_WIDGET : Gtk::PACK_SHRINK);
    widget.show_all();
}

void route_views(unsigned page) {
    switch (page) {
        case 0:
            place(m_vbox, m_dashboard.processes);
            place(m_services, m_dashboard.services);
            place(m_cpu, m_dashboard.cpu);
            place(m_gpu, m_dashboard.gpu, false);
            place(m_history, m_dashboard.gpu);
            break;
        default:
            place(m_gpu, m_root, false);
            m_root.reorder_child(m_gpu, 0);
            place(m_vbox, m_process_page);
            place(m_services, m_service_page);
            place(m_cpu, m_cpu_page);
            place(m_history, m_history_page);
    }
}

void toggle_fullscreen() {
    if (m_fullscreen) {
        unfullscreen();
        m_fullscreen = false;
    } else {
        m_notebook.set_current_page(0);
        m_fullscreen = true;
        fullscreen();
    }
}

// C++ source code for the update_status method
void update_status() {
    bool warning = m_services.has_warnings();
    std::string monitoring_status = m_services.monitoring_status();
    std::string text;

    // Read uptime from /proc/uptime
    FILE* uptime_file = fopen("/proc/uptime", "r");
    if (uptime_file) {
        double seconds;
        if (fscanf(uptime_file, "%lf", &seconds) == 1) {
            int days = static_cast<int>(seconds / (60 * 60 * 24));
            seconds -= days * 60 * 60 * 24;
            int hours = static_cast<int>(seconds / (60 * 60));
            seconds -= hours * 60 * 60;
            int minutes = static_cast<int>(seconds / 60);

            text += "Uptime: ";
            if (days > 0) {
                text += std::to_string(days) + " day" + (days == 1 ? "" : "s") + ", ";
            }
            if (hours > 0 || days > 0) {
                text += std::to_string(hours) + " hour" + (hours == 1 ? "" : "s") + ", ";
            }
            text += std::to_string(minutes) + " minute" + (minutes == 1 ? "" : "s");
        } else {
            text = "Uptime: N/A";
            warning = true;
        }
        fclose(uptime_file);
    } else {
        text = "Uptime: N/A";
        warning = true;
    }

    // Append monitoring status
    text += " | " + monitoring_status;

    // Check GPU telemetry
    if (!m_gpu_seen) {
        text += " | GPU telemetry pending";
    } else if (!m_gpu_valid) {
        text += " | GPU telemetry unavailable";
        warning = true;
    } else {
        std::string gpu = m_gpu.get_text();
        if (gpu.find("VRAM Used: N/A") != std::string::npos ||
            gpu.find("Total: N/A") != std::string::npos ||
            gpu.find("Available: N/A") != std::string::npos) {
            text += " | VRAM telemetry unavailable";
            warning = true;
        }
    }

    // Update the status strip
    m_dashboard.status.set_status(text, warning);
}
private:
bool refresh() {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - m_last_refresh).count();
    auto prev_times = m_process_times;
    m_process_times.clear();
    m_refTreeModel->clear();

    DIR *dir = opendir("/proc");
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            pid_t pid;
            if (sscanf(entry->d_name, "%u", &pid) == 1) {
                std::ifstream stat_file("/proc/" + std::string(entry->d_name) + "/stat");
                std::string line;
                if (std::getline(stat_file, line)) {
                    size_t pos = line.rfind(')');
                        std::istringstream iss(line.substr(pos + 1));
                    std::vector<std::string> tokens;
                    std::string token;
                    while (iss >> token) {
                        tokens.push_back(token);
                    }
                    if (tokens.size() >= 20) {
                        size_t start = line.find('(') + 1;
                        size_t end = line.rfind(')');
                        std::string rawName = line.substr(start, end - start);                        gchar* valid = g_utf8_make_valid(rawName.c_str(), -1);
                        Glib::ustring name(valid);
                        g_free(valid);

                        long ticks = std::stol(tokens[11]) + std::stol(tokens[12]);
                        long starttime = std::stol(tokens[19]);

                        auto it = prev_times.find(pid);
                        double cpu = 0.0;
                        if (it != prev_times.end() && it->second.starttime == starttime && ticks >= it->second.ticks && elapsed > 0) {
                            cpu = 100.0 * (ticks - it->second.ticks) / sysconf(_SC_CLK_TCK) / elapsed;
                        }

                        std::ifstream statm_file("/proc/" + std::string(entry->d_name) + "/statm");
                        unsigned long long virtualPages, residentPages;
                        if (statm_file >> virtualPages >> residentPages) {
                            double memory_mib = static_cast<double>(residentPages * sysconf(_SC_PAGESIZE)) / 1048576.0;

                            auto row = *(m_refTreeModel->append());
                            row[m_columns.name] = name;
                            row[m_columns.pid] = pid;
                            row[m_columns.cpu] = std::round(cpu * 10) / 10;
                            row[m_columns.memory] = std::round(memory_mib * 10) / 10;

                            m_process_times[pid] = {starttime, ticks};
                        }
                    }
                }
            }
        }
        closedir(dir);
    }

    update_status();
    ++m_sample_number;
    m_footer_label.set_text("Processes: " + std::to_string(m_refTreeModel->children().size()) + ", Sample: " + std::to_string(m_sample_number));
    m_last_refresh = std::chrono::steady_clock::now();
    return true;
}

    Gtk::Box m_vbox;
    Gtk::ScrolledWindow m_scrolled_window;
    Gtk::TreeView m_tree_view;
    Glib::RefPtr<Gtk::ListStore> m_refTreeModel;
    Gtk::Label m_footer_label;
    sigc::connection m_timeout_connection;
    std::chrono::steady_clock::time_point m_last_refresh = std::chrono::steady_clock::now();
    int m_sample_number = 0;
    struct ProcessData {
        long starttime;
        long ticks;
    };
    std::map<pid_t, ProcessData> m_process_times;

    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        ModelColumns() {
            add(name);
            add(pid);
            add(cpu);
            add(memory);
        }

        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<int> pid;
        Gtk::TreeModelColumn<double> cpu;
        Gtk::TreeModelColumn<double> memory;
    };

    ModelColumns m_columns;
    Gtk::Notebook m_notebook;
ServicesView m_services;
Gtk::Box m_root{Gtk::ORIENTATION_VERTICAL};
GpuSummary m_gpu;
CpuPanel m_cpu;
ConnectionsView m_connections;
GpuHistory m_history;
Dashboard m_dashboard;
Gtk::Box m_process_page;
Gtk::Box m_service_page;
Gtk::Box m_cpu_page;
Gtk::Box m_connections_page;
Gtk::Box m_history_page;
bool m_fullscreen = false;
bool m_gpu_seen = false;
bool m_gpu_valid = false;
};

int main(int argc, char *argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.ghtop.Monitor");
    ProcessWindow window;
    return app->run(window);
}
