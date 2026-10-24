#pragma once
#include "common.hpp"
#include <iphlpapi.h>
#include <map>
#include <vector>
namespace monitoring::win {
inline std::string tcp_state(DWORD state) {
  switch (state) {
  case MIB_TCP_STATE_CLOSED:
    return "CLOSED";
  case MIB_TCP_STATE_LISTEN:
    return "LISTEN";
  case MIB_TCP_STATE_SYN_SENT:
    return "SYN-SENT";
  case MIB_TCP_STATE_SYN_RCVD:
    return "SYN-RECV";
  case MIB_TCP_STATE_ESTAB:
    return "ESTAB";
  case MIB_TCP_STATE_FIN_WAIT1:
    return "FIN-WAIT-1";
  case MIB_TCP_STATE_FIN_WAIT2:
    return "FIN-WAIT-2";
  case MIB_TCP_STATE_CLOSE_WAIT:
    return "CLOSE-WAIT";
  case MIB_TCP_STATE_CLOSING:
    return "CLOSING";
  case MIB_TCP_STATE_LAST_ACK:
    return "LAST-ACK";
  case MIB_TCP_STATE_TIME_WAIT:
    return "TIME-WAIT";
  case MIB_TCP_STATE_DELETE_TCB:
    return "DELETE-TCB";
  default:
    return "UNKNOWN";
  }
}
inline std::string address(const void *bytes, ULONG family, DWORD scope = 0) {
  wchar_t buffer[INET6_ADDRSTRLEN]{};
  if (!InetNtopW(family, const_cast<void *>(bytes), buffer, INET6_ADDRSTRLEN))
    return "N/A";
  auto text = utf8(buffer);
  if (family == AF_INET6 && scope)
    text += "%" + std::to_string(scope);
  return text;
}
} // namespace monitoring::win
inline std::vector<ConnectionRow>
collect_connections(std::string &error, const std::atomic<bool> &stop) {
  std::vector<ConnectionRow> result;
  std::map<DWORD, std::string> names;
  auto owner = [&](ConnectionRow &row, DWORD pid) {
    row.pid = int(pid);
    if (!names.count(pid))
      names[pid] = monitoring::win::process_name(pid);
    row.application = names[pid];
  };
  auto ports = [](ConnectionRow &row, DWORD local, DWORD remote, bool tcp) {
    row.local_key = ntohs(static_cast<u_short>(local));
    row.local_port = std::to_string(row.local_key);
    row.remote_key = tcp ? ntohs(static_cast<u_short>(remote)) : 0;
    row.remote_port = tcp ? std::to_string(row.remote_key) : "*";
  };
  for (ULONG family : {ULONG(AF_INET), ULONG(AF_INET6)})
    for (bool tcp : {true, false}) {
      if (stop.load())
        return result;
      DWORD bytes = 0;
      auto get = [&](void *buffer) {
        return tcp ? GetExtendedTcpTable(buffer, &bytes, FALSE, family,
                                         TCP_TABLE_OWNER_PID_ALL, 0)
                   : GetExtendedUdpTable(buffer, &bytes, FALSE, family,
                                         UDP_TABLE_OWNER_PID, 0);
      };
      DWORD status = get(nullptr);
      if (status != ERROR_INSUFFICIENT_BUFFER && status != NO_ERROR) {
        error += monitoring::win::error("Read connection table", status);
        continue;
      }
      std::vector<unsigned char> storage;
      for (unsigned retry = 0; retry < 4; ++retry) {
        if (bytes > 64 * 1024 * 1024) {
          status = ERROR_NOT_ENOUGH_MEMORY;
          break;
        }
        storage.resize(bytes);
        status = get(storage.data());
        if (status != ERROR_INSUFFICIENT_BUFFER)
          break;
      }
      if (status != NO_ERROR || storage.size() < sizeof(DWORD)) {
        error += monitoring::win::error("Read connection table", status);
        continue;
      }
      if (tcp && family == AF_INET) {
        auto table = reinterpret_cast<MIB_TCPTABLE_OWNER_PID *>(storage.data());
        for (DWORD i = 0; i < table->dwNumEntries && !stop.load(); ++i) {
          const auto &item = table->table[i];
          ConnectionRow row;
          row.protocol = "TCP";
          row.state = monitoring::win::tcp_state(item.dwState);
          row.local_address =
              monitoring::win::address(&item.dwLocalAddr, family);
          row.remote_address =
              monitoring::win::address(&item.dwRemoteAddr, family);
          ports(row, item.dwLocalPort, item.dwRemotePort, true);
          owner(row, item.dwOwningPid);
          result.push_back(std::move(row));
        }
      } else if (tcp) {
        auto table =
            reinterpret_cast<MIB_TCP6TABLE_OWNER_PID *>(storage.data());
        for (DWORD i = 0; i < table->dwNumEntries && !stop.load(); ++i) {
          const auto &item = table->table[i];
          ConnectionRow row;
          row.protocol = "TCP6";
          row.state = monitoring::win::tcp_state(item.dwState);
          row.local_address = monitoring::win::address(item.ucLocalAddr, family,
                                                       item.dwLocalScopeId);
          row.remote_address = monitoring::win::address(
              item.ucRemoteAddr, family, item.dwRemoteScopeId);
          ports(row, item.dwLocalPort, item.dwRemotePort, true);
          owner(row, item.dwOwningPid);
          result.push_back(std::move(row));
        }
      } else if (family == AF_INET) {
        auto table = reinterpret_cast<MIB_UDPTABLE_OWNER_PID *>(storage.data());
        for (DWORD i = 0; i < table->dwNumEntries && !stop.load(); ++i) {
          const auto &item = table->table[i];
          ConnectionRow row;
          row.protocol = "UDP";
          row.state = "UNCONN";
          row.local_address =
              monitoring::win::address(&item.dwLocalAddr, family);
          row.remote_address = "*";
          ports(row, item.dwLocalPort, 0, false);
          owner(row, item.dwOwningPid);
          result.push_back(std::move(row));
        }
      } else {
        auto table =
            reinterpret_cast<MIB_UDP6TABLE_OWNER_PID *>(storage.data());
        for (DWORD i = 0; i < table->dwNumEntries && !stop.load(); ++i) {
          const auto &item = table->table[i];
          ConnectionRow row;
          row.protocol = "UDP6";
          row.state = "UNCONN";
          row.local_address = monitoring::win::address(item.ucLocalAddr, family,
                                                       item.dwLocalScopeId);
          row.remote_address = "*";
          ports(row, item.dwLocalPort, 0, false);
          owner(row, item.dwOwningPid);
          result.push_back(std::move(row));
        }
      }
    }
  return result;
}
