#pragma once
#include "common.hpp"
#include <filesystem>
#include <glib.h>
#include <stdexcept>
namespace monitoring::win {
inline void configure_runtime() {
  std::wstring path(32768, L'\0');
  DWORD length = GetModuleFileNameW(nullptr, path.data(), DWORD(path.size()));
  if (!length || length >= path.size())
    throw std::runtime_error("Cannot locate G_htop executable directory");
  path.resize(length);
  auto bin = std::filesystem::path(path).parent_path(),
       root = bin.parent_path();
  // Imports resolve alongside the executable; later module loads search only
  // that trusted directory and Windows system directories, never the cwd.
  SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
  SetDllDirectoryW(bin.c_str());
  auto set = [&](const char *key, const std::filesystem::path &value) {
    g_setenv(key, utf8(value.wstring()).c_str(), TRUE);
  };
  set("GTK_EXE_PREFIX", root);
  set("GTK_DATA_PREFIX", root);
  set("GSETTINGS_SCHEMA_DIR", root / "share/glib-2.0/schemas");
  set("GDK_PIXBUF_MODULE_FILE",
      root / "lib/gdk-pixbuf-2.0/2.10.0/loaders.cache");
  g_setenv("GDK_BACKEND", "win32", TRUE);
  // Wine's Win32 font backend can produce clipped glyphs. Use the packaged
  // Fontconfig configuration there; native Windows keeps its normal backend.
  if (running_under_wine()) {
    g_setenv("PANGOCAIRO_BACKEND", "fontconfig", TRUE);
    set("FONTCONFIG_FILE", root / "etc/fonts/fonts-wine.conf");
  }
}
} // namespace monitoring::win
