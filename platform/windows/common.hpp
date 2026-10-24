#pragma once
// winsock2 must precede windows.h (also when GTK later includes windows.h).
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <cstdint>
#include <string>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
namespace monitoring::win {
class Handle {
  HANDLE value_;

public:
  explicit Handle(HANDLE value = nullptr) : value_(value) {}
  ~Handle() {
    if (value_ && value_ != INVALID_HANDLE_VALUE)
      CloseHandle(value_);
  }
  Handle(const Handle &) = delete;
  Handle &operator=(const Handle &) = delete;
  HANDLE get() const { return value_; }
  explicit operator bool() const {
    return value_ && value_ != INVALID_HANDLE_VALUE;
  }
};
inline std::string utf8(const std::wstring &value) {
  if (value.empty())
    return {};
  int n = WideCharToMultiByte(CP_UTF8, 0, value.data(), int(value.size()),
                              nullptr, 0, nullptr, nullptr);
  if (n <= 0)
    return {};
  std::string out(n, '\0');
  WideCharToMultiByte(CP_UTF8, 0, value.data(), int(value.size()), out.data(),
                      n, nullptr, nullptr);
  return out;
}
inline std::string error(const char *operation, DWORD code = GetLastError()) {
  wchar_t *buffer = nullptr;
  FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                 nullptr, code, 0, reinterpret_cast<wchar_t *>(&buffer), 0,
                 nullptr);
  std::string result = std::string(operation) + ": " +
                       (buffer ? utf8(buffer) : std::to_string(code));
  if (buffer)
    LocalFree(buffer);
  return result;
}
inline std::uint64_t ticks(const FILETIME &time) {
  return (std::uint64_t(time.dwHighDateTime) << 32) | time.dwLowDateTime;
}
inline std::string process_name(DWORD pid) {
  Handle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
  if (!process)
    return "Unknown";
  std::wstring path(32768, L'\0');
  DWORD count = DWORD(path.size());
  if (!QueryFullProcessImageNameW(process.get(), 0, path.data(), &count))
    return "Unknown";
  path.resize(count);
  auto slash = path.find_last_of(L"\\/");
  return utf8(path.substr(slash == std::wstring::npos ? 0 : slash + 1));
}
} // namespace monitoring::win
