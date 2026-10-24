#include "gpu_history.hpp"
#include <gtkmm.h>
#include <cassert>
#include <filesystem>
#include <iostream>
int main(int argc, char** argv) {
  assert(argc == 2);
  const std::filesystem::path out = argv[1];
  std::filesystem::create_directories(out);
  auto app = Gtk::Application::create("org.ghtop.Community.SharedGpuTest", Gio::APPLICATION_NON_UNIQUE);
  GpuHistory graph;
  GpuReading a{"AMD Radeon Pro W5500 (card0)", 0, "", "pci-amd-a"};
  GpuReading b{"AMD Radeon Pro W5500 (card0)", 75, "", "pci-amd-b"};
  GpuReading c{"NVIDIA GeForce GTX 1050 (card2)", 42, "", "pci-nvidia"};
  GpuReading d{"Intel HD Graphics 630 (card3)", -1, "", "pci-intel"};
  graph.append_sample({a,b,c,d});
  assert(graph.series_count() == 4);
  const auto ca = graph.series_color(a.identity), cb = graph.series_color(b.identity);
  assert(ca != cb && ca != graph.series_color(c.identity) && cb != graph.series_color(c.identity));
  graph.append_sample({d,c,b,a});
  assert(graph.sample_count() == 2 && graph.series_color(a.identity) == ca);
  graph.append_sample({a,c,d});
  assert(graph.series_sample_count(a.identity) == 3 && graph.series_sample_count(b.identity) == 0);
  graph.append_sample({b,a,c,d});
  assert(graph.series_sample_count(b.identity) == 1 && graph.series_sample_count(a.identity) == 4);
  assert(graph.series_color(b.identity) == cb);
  b.identity = "replacement";
  graph.append_sample({a,b,c,d});
  assert(graph.series_sample_count(b.identity) == 1 && graph.series_sample_count(a.identity) == 5);
  // Exercise the palette beyond three colors and both missing and non-finite data.
  auto extra = c;
  extra.identity = "extra"; extra.name = "Additional GPU"; extra.usage = std::nan("");
  graph.append_sample({a,b,c,d,extra});
  Gtk::Window window;
  window.set_title("Community shared GPU panel — synthetic mixed-vendor fixture");
  window.set_default_size(600,330);
  window.add(graph); window.show_all_children();
  int stage = 0;
  auto timer = Glib::signal_timeout().connect([&] {
    auto surface = window.get_window();
    assert(surface);
    if (!g_getenv("G_HTOP_TEST_NO_SCREENSHOT")) Gdk::Pixbuf::create(surface,0,0,window.get_allocated_width(),window.get_allocated_height())->save((out/(stage == 0 ? "mixed-gpus.png" : "no-gpu.png")).string(),"png");
    if (stage++ == 0) {graph.append_sample({}); assert(graph.series_count() == 0); return true;}
    window.hide(); return false;
  },300);
  app->run(window); timer.disconnect();
  std::cout << "PASS shared GPU panel: identity, reorder, add/remove, replacement, distinct colors, missing/NaN, rendering, no GPU\n";
}
