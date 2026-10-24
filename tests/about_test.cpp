#include "about.hpp"
#include <gtkmm.h>
#include <cassert>
#include <filesystem>
#include <iostream>

static Gtk::Label* find_link(Gtk::Widget& widget, const std::string& uri) {
  if (auto* link = dynamic_cast<Gtk::Label*>(&widget))
    if (link->get_label().find(uri) != Glib::ustring::npos) return link;
  if (auto* container = dynamic_cast<Gtk::Container*>(&widget))
    for (auto* child : container->get_children())
      if (auto* link = find_link(*child, uri)) return link;
  return nullptr;
}
int main(int argc, char** argv) {
  assert(argc == 2);
  auto app = Gtk::Application::create("org.ghtop.Community.AboutTest", Gio::APPLICATION_NON_UNIQUE);
  Gtk::Window parent;
  CommunityAboutDialog dialog(parent, G_HTOP_VERSION);
  const std::string uri = "https://don-hunt.com/G_htopPro/";
  assert(dialog.get_version() == "1.1.0");
  assert(dialog.get_website() == uri);
  assert(dialog.get_website_label().find("separately purchased professional edition") != Glib::ustring::npos);
  assert(dialog.get_license_type() == Gtk::LICENSE_CUSTOM);
  assert(dialog.get_license().find("Apache License, Version 2.0") != Glib::ustring::npos);
  assert(dialog.get_comments().find("Free, open-source") != Glib::ustring::npos);
  // Intercept the standard About link dispatch to verify its URI without launching a browser.
  bool activated = false;
  dialog.signal_activate_link().connect([&](const Glib::ustring& link) {
    assert(link == uri); activated = true; return true;
  }, false);
  dialog.show_all();
  Glib::signal_timeout().connect_once([&] {
    auto* link = find_link(dialog, uri);
    assert(link);
    gboolean handled = false;
    g_signal_emit_by_name(link->gobj(), "activate-link", uri.c_str(), &handled);
    assert(handled);
    assert(activated);
    const std::filesystem::path output(argv[1]);
    std::filesystem::create_directories(output);
    if (!g_getenv("G_HTOP_TEST_NO_SCREENSHOT")) Gdk::Pixbuf::create(dialog.get_window(), 0, 0, dialog.get_allocated_width(), dialog.get_allocated_height())->save((output/"about.png").string(), "png");
    dialog.response(Gtk::RESPONSE_CLOSE);
  }, 350);
  dialog.run();
  assert(activated);
  std::cout << "PASS About: v1.1.0, Apache-2.0, separate paid Pro label, actual link activation URI\n";
}
