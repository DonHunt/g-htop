#pragma once
#include <algorithm>
#include <cctype>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
namespace monitoring {
inline std::optional<double> uptime_seconds() {
  double seconds;
  std::ifstream in("/proc/uptime");
  if (in >> seconds)
    return seconds;
  return std::nullopt;
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
    DIR *dir = opendir("/proc");
    if (!dir)
      return result;
    while (auto entry = readdir(dir)) {
      std::string id = entry->d_name;
      if (id.empty() || !std::all_of(id.begin(), id.end(), [](unsigned char c) {
            return std::isdigit(c);
          }))
        continue;
      try {
        int pid = std::stoi(id);
        std::ifstream stat("/proc/" + id + "/stat");
        std::string line;
        if (!std::getline(stat, line))
          continue;
        auto begin = line.find('('), end = line.rfind(')');
        if (begin == std::string::npos || end == std::string::npos ||
            end <= begin)
          continue;
        std::istringstream in(line.substr(end + 1));
        std::vector<std::string> tokens;
        std::string token;
        while (in >> token)
          tokens.push_back(token);
        if (tokens.size() < 20)
          continue;
        ProcessTimes times{std::stoull(tokens[19]),
                           std::stoull(tokens[11]) + std::stoull(tokens[12])};
        double cpu = 0;
        auto prev = previous.find(pid);
        if (prev != previous.end() && prev->second.creation == times.creation &&
            times.ticks >= prev->second.ticks && elapsed > 0)
          cpu = 100.0 * (times.ticks - prev->second.ticks) /
                sysconf(_SC_CLK_TCK) / elapsed;
        std::ifstream memory("/proc/" + id + "/statm");
        unsigned long long virtual_pages, resident_pages;
        if (!(memory >> virtual_pages >> resident_pages))
          continue;
        result.push_back(
            {line.substr(begin + 1, end - begin - 1), pid, cpu,
             resident_pages * double(sysconf(_SC_PAGESIZE)) / 1048576.0});
        current[pid] = times;
      } catch (const std::exception
                   &) { /* Process exited or its proc entry changed. */
      }
    }
    closedir(dir);
    previous = std::move(current);
    last = std::chrono::steady_clock::now();
    return result;
  }
};
} // namespace monitoring
