#ifndef SERVICES_VIEW_HPP
#define SERVICES_VIEW_HPP

#include "service_data.hpp"
#include "presentation.hpp"
#include <atomic>
#include <cstdlib>
#include <chrono>
#include <condition_variable>
#include <glibmm/main.h>
#include <gtkmm.h>
#include <mutex>
#include <thread>

class ServicesView : public Gtk::Box {
public:
  ServicesView() : Gtk::Box(Gtk::ORIENTATION_VERTICAL), displayed_count(0) {
    filter.append("All");
    filter.append("Active");
    filter.append("Inactive");
    filter.append("Failed");
    filter.set_active_text("All");

    tree.get_selection()->set_mode(Gtk::SELECTION_SINGLE);
    store = Gtk::ListStore::create(columns);
    tree.set_model(store);

    tree.append_column("Description", columns.description);
    tree.get_column(0)->set_sort_column(columns.description);
    tree.get_column(0)->set_expand(true);

    tree.append_column("Unit", columns.unit);
    tree.get_column(1)->set_sort_column(columns.unit);

    tree.append_column("State", columns.state);
    tree.get_column(2)->set_sort_column(columns.state);

    tree.append_column("Scope", columns.scope);
    tree.get_column(3)->set_sort_column(columns.scope);

    tree.append_column("PID", columns.pid);
    tree.get_column(4)->set_sort_column(columns.pid);

    for (int i = 0; i < 5; ++i) {
      auto col = tree.get_column(i);
      if (col) {
        col->set_resizable(true);
        col->set_fixed_width(
            (i == 0)
                ? 280
                : ((i == 1) ? 260 : ((i == 2) ? 110 : ((i == 3) ? 80 : 90))));
      }
    }

Ui::table(tree, {4});
tree.set_tooltip_column(columns.description.index());
store->set_sort_func(columns.pid, [this](const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b) {
    Glib::ustring sa = (*a)[columns.pid], sb = (*b)[columns.pid];
    int pa = sa == "N/A" ? 0 : std::strtol((sa).c_str(), nullptr, 10), pb = sb == "N/A" ? 0 : std::strtol((sb).c_str(), nullptr, 10);
    return (pa > pb) - (pa < pb);
});
Gtk::TreeViewColumn* description = tree.get_column(0);
Gtk::TreeViewColumn* unit = tree.get_column(1);
Gtk::TreeViewColumn* state = tree.get_column(2);
Gtk::TreeViewColumn* scope = tree.get_column(3);
Gtk::TreeViewColumn* pid = tree.get_column(4);
description->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
description->set_fixed_width(180);
description->set_expand(true);
unit->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
unit->set_fixed_width(180);
unit->set_expand(false);
state->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
state->set_fixed_width(90);
state->set_expand(false);
scope->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
scope->set_fixed_width(70);
scope->set_expand(false);
pid->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
pid->set_fixed_width(74);
pid->set_expand(false);
dynamic_cast<Gtk::CellRendererText*>(description->get_first_cell())->property_ellipsize() = Pango::ELLIPSIZE_END;
tree.move_column_after(*state, *description);
tree.move_column_after(*scope, *state);
store->set_sort_column(columns.description, Gtk::SORT_ASCENDING);
    footer.set_ellipsize(Pango::ELLIPSIZE_END); scroll.add(tree);
    pack_start(filter, Gtk::PACK_SHRINK);
    pack_start(scroll, Gtk::PACK_EXPAND_WIDGET);
    pack_start(footer, Gtk::PACK_SHRINK);

    filter.signal_changed().connect(
        sigc::mem_fun(*this, &ServicesView::render));

    start_worker();

    timer = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &ServicesView::poll), 250);

    show_all_children();
  }

  ~ServicesView() {
    stop_worker = true;
    timer.disconnect();
    worker_cv.notify_all();
    if (worker_thread.joinable()) {
      worker_thread.join();
    }
  }

public:
  std::string monitoring_status() const { if (refresh_counter == 0) return "Failed services: pending"; unsigned int count = 0; for (const auto& row : cached) { if (row.state == "failed") count++; } std::string status = "Failed services: " + std::to_string(count) + " (detected)"; if (!cached_error.empty()) status += " | Services: " + cached_error; return status; }
public:
    bool has_warnings() const { return !cached_error.empty() || std::any_of(cached.begin(), cached.end(), [](const ServiceRow& row) { return row.state == "failed"; }); }
private:
    void render() {

    store->clear();

    displayed_count = 0;
    auto selected = filter.get_active_text();
    for (const auto &row : cached) {

      if ((selected == "Active" && row.state != "active") ||
          (selected == "Inactive" && row.state != "inactive") ||
          (selected == "Failed" && row.state != "failed")) {
        continue;
      }

      ++displayed_count;
      Gtk::TreeModel::Row new_row = *(store->append());
      new_row[columns.description] = sanitize_string(row.description);
      new_row[columns.unit] = sanitize_string(row.unit);
      new_row[columns.state] = sanitize_string(row.state);
      new_row[columns.scope] = sanitize_string(row.scope);
      new_row[columns.pid] = row.pid > 0 ? std::to_string(row.pid) : "N/A";
    }

    footer.set_text(Glib::ustring::compose(
        "%1 visible of %2 services, updated %3, refresh5s, %4;",
        displayed_count, cached.size(), ++refresh_counter, cached_error));
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
        try {
          std::string error;
          auto rows = collect_services(error, stop_worker);

          std::lock_guard<std::mutex> lock(worker_mutex);
          pending = std::move(rows);
          pending_error = error;
          ready = true;
          worker_cv.notify_all();
        } catch (const std::exception &e) {
          std::lock_guard<std::mutex> lock(worker_mutex);
          pending_error = e.what();
        }

        std::unique_lock<std::mutex> lock(worker_mutex);
        worker_cv.wait_for(lock, std::chrono::seconds(5),
                           [this] { return stop_worker.load(); });
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
      add(description);
      add(unit);
      add(state);
      add(scope);
      add(pid);
    }

    Gtk::TreeModelColumn<Glib::ustring> description;
    Gtk::TreeModelColumn<Glib::ustring> unit;
    Gtk::TreeModelColumn<Glib::ustring> state;
    Gtk::TreeModelColumn<Glib::ustring> scope;
    Gtk::TreeModelColumn<Glib::ustring> pid;
  };

  Glib::RefPtr<Gtk::ListStore> store;
  Columns columns;

  std::atomic<bool> stop_worker{false};
  std::mutex worker_mutex;
  std::condition_variable worker_cv;
  bool ready = false;
  std::vector<ServiceRow> pending;
  std::string pending_error;
  std::string cached_error;
  std::vector<ServiceRow> cached;
  int displayed_count;
  unsigned int refresh_counter = 0;

  std::thread worker_thread;
  sigc::connection timer;
};

#endif // SERVICES_VIEW_HPP
