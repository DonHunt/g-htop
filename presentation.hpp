#ifndef PRESENTATION_HPP
#define PRESENTATION_HPP

#include <gtkmm.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>

namespace Ui {

inline std::string percent(double v) { if (!std::isfinite(v) || v < 0) return "N/A"; std::ostringstream oss; oss << std::fixed << std::setprecision(1) << v << "%"; return oss.str(); }

inline std::string memory(double mib) {
    if (!std::isfinite(mib) || mib < 0) {
        return "N/A";
    } else if (mib >= 1024) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << (mib / 1024.0) << " GiB";
        return oss.str();
    } else {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << mib << " MiB";
        return oss.str();
    }
}

inline std::string gpu_name(const std::string& detailed) {
    size_t pos = detailed.find("Radeon");
    if (pos != std::string::npos) {
        size_t end_pos = detailed.find(']', pos);
        std::string name = detailed.substr(pos, end_pos - pos);
        name.erase(name.find_last_not_of(" ") + 1); // Trim trailing spaces
        return "AMD " + name;
    } else {
        return "AMD GPU";
    }
}

inline void table(Gtk::TreeView& tree, const std::vector<int>& numeric) {
    for (int i = 0; i < tree.get_columns().size(); ++i) {
        auto column = dynamic_cast<Gtk::TreeViewColumn*>(tree.get_columns()[i]);
        if (column) {
            column->set_spacing(8);
            for (auto& cell : column->get_cells()) {
                auto text_cell = dynamic_cast<Gtk::CellRendererText*>(cell);
                if (text_cell) {
                    text_cell->property_xpad() = 6;
                    text_cell->property_ypad() = 3;
                    if (std::find(numeric.begin(), numeric.end(), i) != numeric.end()) {
                        column->set_alignment(1.0);
                        text_cell->property_xalign() = 1.0;
                    }
                }
            }
        }
    }
}

inline void number(Gtk::TreeView& tree, int index, Gtk::TreeModelColumn<double> value, bool percentage) {
    auto column = dynamic_cast<Gtk::TreeViewColumn*>(tree.get_columns()[index]);
    if (column) {
        for (auto& cell : column->get_cells()) {
            auto text_cell = dynamic_cast<Gtk::CellRendererText*>(cell);
            if (text_cell) {
                column->set_cell_data_func(*text_cell, [value, percentage](Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter) {
                    if (iter) {
                        double n = (*iter)[value];
                        dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text() = percentage ? percent(n) : memory(n);
                    }
                });
            }
        }
        column->set_sort_column(value);
        column->set_alignment(1.0);
    }
}

} // namespace Ui

#endif // PRESENTATION_HPP
