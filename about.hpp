#pragma once

#include <gtkmm/aboutdialog.h>
#include <gtkmm/window.h>
#include <string>

class CommunityAboutDialog : public Gtk::AboutDialog {
public:
  CommunityAboutDialog(Gtk::Window& parent, const std::string& version) {
    set_transient_for(parent);
    set_modal(true);
    set_program_name("G_htop Community Edition");
    set_version(version);
    set_copyright("Copyright © 2026 Donald E. Hunt");
    set_comments("Free, open-source system monitoring.\n"
                 "G_htop Pro is the separately purchased professional edition.");
    set_license_type(Gtk::LICENSE_CUSTOM);
    set_license("Apache License, Version 2.0\n"
                "https://www.apache.org/licenses/LICENSE-2.0\n\n"
                "See LICENSE in the source or installed package for the complete text.");
    set_website("https://don-hunt.com/G_htopPro/");
    set_website_label("G_htop Pro — separately purchased professional edition");
    set_logo_icon_name("G_htop");
  }
};
