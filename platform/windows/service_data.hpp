#pragma once
#include "common.hpp"
#include <vector>
#include <winsvc.h>
inline std::vector<ServiceRow> collect_services(std::string &error,
                                                const std::atomic<bool> &stop) {
  std::vector<ServiceRow> result;
  SC_HANDLE manager =
      OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
  if (!manager) {
    error = monitoring::win::error("OpenSCManager");
    return result;
  }
  struct Close {
    SC_HANDLE value;
    ~Close() { CloseServiceHandle(value); }
  } close{manager};
  DWORD resume = 0;
  do {
    DWORD bytes = 0, count = 0;
    std::vector<unsigned char> storage(256 * 1024);
    BOOL ok = EnumServicesStatusExW(manager, SC_ENUM_PROCESS_INFO,
                                    SERVICE_WIN32, SERVICE_STATE_ALL,
                                    storage.data(), DWORD(storage.size()),
                                    &bytes, &count, &resume, nullptr);
    DWORD status = ok ? ERROR_SUCCESS : GetLastError();
    if (!ok && status != ERROR_MORE_DATA) {
      error = monitoring::win::error("EnumServicesStatusEx", status);
      break;
    }
    auto services =
        reinterpret_cast<ENUM_SERVICE_STATUS_PROCESSW *>(storage.data());
    for (DWORD i = 0; i < count && !stop.load(); ++i) {
      auto &service = services[i];
      ServiceRow row;
      row.unit = monitoring::win::utf8(service.lpServiceName);
      row.description = monitoring::win::utf8(service.lpDisplayName);
      row.scope = "System";
      row.pid = service.ServiceStatusProcess.dwProcessId;
      switch (service.ServiceStatusProcess.dwCurrentState) {
      case SERVICE_RUNNING:
        row.state = "active";
        row.substate = "running";
        break;
      case SERVICE_STOPPED:
        row.state = service.ServiceStatusProcess.dwWin32ExitCode ? "failed"
                                                                 : "inactive";
        row.substate = "stopped";
        break;
      case SERVICE_PAUSED:
        row.state = "inactive";
        row.substate = "paused";
        break;
      case SERVICE_START_PENDING:
        row.state = "activating";
        row.substate = "start pending";
        break;
      case SERVICE_STOP_PENDING:
        row.state = "deactivating";
        row.substate = "stop pending";
        break;
      case SERVICE_CONTINUE_PENDING:
        row.state = "activating";
        row.substate = "continue pending";
        break;
      case SERVICE_PAUSE_PENDING:
        row.state = "deactivating";
        row.substate = "pause pending";
        break;
      default:
        row.state = "unknown";
        row.substate = "unknown";
      }
      result.push_back(std::move(row));
    }
    if (ok || count == 0)
      break;
  } while (!stop.load());
  return result;
}
