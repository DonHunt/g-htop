#pragma once
#include "common.hpp"
#include <psapi.h>
#include <tlhelp32.h>
namespace monitoring {
inline std::optional<double> uptime_seconds() {
  return GetTickCount64() / 1000.0;
}
class ProcessCollector {
  std::map<int, ProcessTimes> previous;
  std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now();

public:
  std::vector<ProcessRow> sample() {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - last).count();
    std::vector<ProcessRow> result;
    std::map<int, ProcessTimes> current;
    win::Handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (!snapshot)
      return result;
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snapshot.get(), &entry))
      do {
        ProcessRow row{win::utf8(entry.szExeFile), int(entry.th32ProcessID), -1,
                       -1};
        win::Handle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
                                        FALSE, entry.th32ProcessID));
        if (process) {
          FILETIME creation, exit, kernel, user;
          if (GetProcessTimes(process.get(), &creation, &exit, &kernel,
                              &user)) {
            ProcessTimes times{win::ticks(creation),
                               win::ticks(kernel) + win::ticks(user)};
            auto prev = previous.find(row.pid);
            if (prev != previous.end() &&
                prev->second.creation == times.creation &&
                times.ticks >= prev->second.ticks && elapsed > 0)
              row.cpu = 100.0 * (times.ticks - prev->second.ticks) /
                        10000000.0 / elapsed;
            current[row.pid] = times;
          }
          PROCESS_MEMORY_COUNTERS memory{};
          memory.cb = sizeof(memory);
          // Current Windows supports QUERY_LIMITED_INFORMATION; retry the
          // documented QUERY_INFORMATION|VM_READ access for systems/providers
          // requiring it.
          if (GetProcessMemoryInfo(process.get(), &memory, sizeof(memory)))
            row.memory = memory.WorkingSetSize / 1048576.0;
          else {
            win::Handle readable(
                OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
                            entry.th32ProcessID));
            if (readable &&
                GetProcessMemoryInfo(readable.get(), &memory, sizeof(memory)))
              row.memory = memory.WorkingSetSize / 1048576.0;
          }
        }
        result.push_back(std::move(row));
      } while (Process32NextW(snapshot.get(), &entry));
    previous = std::move(current);
    last = now;
    return result;
  }
};
} // namespace monitoring
