#ifndef CONNECTIONS_VIEW_HPP
#define CONNECTIONS_VIEW_HPP

#include "connection_data.hpp"
#include "presentation.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <glibmm/main.h>
#include <gtkmm.h>
#include <mutex>
#include <thread>

class ConnectionsView : public Gtk::Box {
public:
  ConnectionsView() : Gtk::Box(Gtk::ORIENTATION_VERTICAL), displayed_count(0) {
    filter.append("All");
    filter.append("Listening");
    filter.append("Established");
    filter.append("UDP");
    filter.set_active_text("All");

    tree.get_selection()->set_mode(Gtk::SELECTION_SINGLE);
    store = Gtk::ListStore::create(columns);
    tree.set_model(store);

    tree.append_column("Application", columns.application);
    tree.append_column("PID", columns.pid);
    tree.append_column("Protocol", columns.protocol);
    tree.append_column("Local address", columns.local_address);
    tree.append_column("Local port", columns.local_port);
    tree.append_column("Remote address", columns.remote_address);
    tree.append_column("Remote port", columns.remote_port);
    tree.append_column("State", columns.state);
    tree.get_column(0)->set_sort_column(columns.application);
    tree.get_column(1)->set_sort_column(columns.pid_key);
    tree.get_column(2)->set_sort_column(columns.protocol);
    tree.get_column(3)->set_sort_column(columns.local_address);
    tree.get_column(4)->set_sort_column(columns.local_key);
    tree.get_column(5)->set_sort_column(columns.remote_address);
    tree.get_column(6)->set_sort_column(columns.remote_key);
    tree.get_column(7)->set_sort_column(columns.state);
    for (int i = 0; i < 8; ++i) {
      tree.get_column(i)->set_resizable(true);
      tree.get_column(i)->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
    }
    tree.get_column(0)->set_expand(true);
    const int widths[8] = {150, 70, 70, 180, 85, 200, 90, 110};
    for (int i = 0; i < 8; ++i)
      tree.get_column(i)->set_fixed_width(widths[i]);
    tree.set_model(store);
Ui::table(tree, {1, 4, 6});
tree.set_tooltip_column(columns.application.index());
store->set_sort_column(columns.application, Gtk::SORT_ASCENDING);
    footer.set_line_wrap(true);
    scroll.add(tree);
    pack_start(filter, Gtk::PACK_SHRINK);
    pack_start(scroll, Gtk::PACK_EXPAND_WIDGET);
    pack_start(footer, Gtk::PACK_SHRINK);

    filter.signal_changed().connect(
        sigc::mem_fun(*this, &ConnectionsView::render));

    start_worker();

    timer = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &ConnectionsView::poll), 250);

    show_all_children();
  }

  ~ConnectionsView() {
    stop_worker = true;
    timer.disconnect();
    worker_cv.notify_all();
    if (worker_thread.joinable()) {
      worker_thread.join();
    }
  }

private:
  void render() {
    store->clear();
    displayed_count = 0;
    auto selected = filter.get_active_text();

    for (const auto &c : cached) {
      if ((selected == "Listening" && c.state != "LISTEN") ||
          (selected == "Established" && c.state != "ESTAB") ||
          (selected == "UDP" && c.protocol != "UDP" && c.protocol != "UDP6")) {
        continue;
      }

      Gtk::TreeModel::Row row = *(store->append());
      row[columns.application] = sanitize_string(c.application);
      row[columns.pid_key] = c.pid;
      row[columns.pid] = c.pid > 0 ? std::to_string(c.pid) : "N/A";
      row[columns.protocol] = sanitize_string(c.protocol);
      row[columns.local_address] = sanitize_string(c.local_address);
      row[columns.local_key] = c.local_key;
      row[columns.local_port] = sanitize_string(c.local_port);
      row[columns.remote_address] = sanitize_string(c.remote_address);
      row[columns.remote_key] = c.remote_key;
      row[columns.remote_port] = sanitize_string(c.remote_port);
      row[columns.state] = sanitize_string(c.state);

      ++displayed_count;
    }

    footer.set_text(Glib::ustring::compose(
        "%1/%2 connections | Sample %3 | 2s | %4", displayed_count,
        cached.size(), ++refresh_counter, cached_error));
  }

  bool poll() {
    std::lock_guard<std::mutex> lock(worker_mutex);
    if (!ready)
      return true;

    cached = std::move(pending);
    cached_error = pending_error;
    ready = false;

    render();
    return true;
  }

  Glib::ustring sanitize_string(const std::string &input) {
    char *valid_str = g_utf8_make_valid(input.c_str(), -1);
    Glib::ustring result(valid_str);
    g_free(valid_str);
    return result;
  }

  void start_worker() {
    worker_thread = std::thread([this]() {
      while (!stop_worker) {
        const auto next =
            std::chrono::steady_clock::now() + std::chrono::seconds(2);
        try {
          std::string error;
          auto rows = collect_connections(error, stop_worker);

          std::lock_guard<std::mutex> lock(worker_mutex);
          pending = std::move(rows);
          pending_error = error;
          ready = true;
          worker_cv.notify_all();
        } catch (const std::exception &e) {
          std::lock_guard<std::mutex> lock(worker_mutex);
          pending_error = e.what();
          ready = true;
        }

        std::unique_lock<std::mutex> lock(worker_mutex);
        worker_cv.wait_until(lock, next, [this] { return stop_worker.load(); });
      }
    });
  }

  Gtk::ComboBoxText filter;
  Gtk::ScrolledWindow scroll;
  Gtk::TreeView tree;
  Gtk::Label footer;

  class Columns : public Gtk::TreeModel::ColumnRecord {
  public:
    Columns() {
      add(application);
      add(pid_key);
      add(protocol);
      add(local_address);
      add(local_key);
      add(local_port);
      add(remote_address);
      add(remote_key);
      add(remote_port);
      add(state);
      add(pid);
    }

    Gtk::TreeModelColumn<Glib::ustring> application;
    Gtk::TreeModelColumn<int> pid_key;
    Gtk::TreeModelColumn<Glib::ustring> protocol;
    Gtk::TreeModelColumn<Glib::ustring> local_address;
    Gtk::TreeModelColumn<int> local_key;
    Gtk::TreeModelColumn<Glib::ustring> local_port;
    Gtk::TreeModelColumn<Glib::ustring> remote_address;
    Gtk::TreeModelColumn<int> remote_key;
    Gtk::TreeModelColumn<Glib::ustring> remote_port;
    Gtk::TreeModelColumn<Glib::ustring> state;
    Gtk::TreeModelColumn<Glib::ustring> pid;
  };

  Glib::RefPtr<Gtk::ListStore> store;
  Columns columns;

  std::atomic<bool> stop_worker{false};
  std::mutex worker_mutex;
  std::condition_variable worker_cv;
  bool ready = false;
  std::vector<ConnectionRow> pending;
  std::string pending_error;
  std::string cached_error;
  std::vector<ConnectionRow> cached;
  int displayed_count;
  unsigned int refresh_counter = 0;

  std::thread worker_thread;
  sigc::connection timer;
};

#endif // CONNECTIONS_VIEW_HPP
