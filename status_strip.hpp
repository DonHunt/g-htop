#ifndef STATUS_STRIP_HPP
#define STATUS_STRIP_HPP

#include <gtkmm.h>
#include <string>

class StatusStrip : public Gtk::Box {
public:
    StatusStrip() 
        : Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 8) {
        set_margin_start(8);
        set_margin_end(8);
        set_margin_top(6);
        set_margin_bottom(6);

        label.set_xalign(0.0);
        label.set_ellipsize(Pango::ELLIPSIZE_END);
        label.set_hexpand(true);

        pack_start(icon, Gtk::PACK_SHRINK);
        pack_start(label, Gtk::PACK_EXPAND_WIDGET);

        set_status("Monitoring data pending", false);
    }

    void set_status(const std::string& text, bool warning) {
        label.set_text(text);
        label.set_tooltip_text(text);
        set_tooltip_text(text);

        icon.set_from_icon_name(warning ? "dialog-warning-symbolic" : "dialog-information-symbolic", Gtk::ICON_SIZE_MENU);
        icon.set_tooltip_text(warning ? "Detected monitoring warning" : "Monitoring information");
    }

private:
    Gtk::Image icon;
    Gtk::Label label;
};

#endif // STATUS_STRIP_HPP
