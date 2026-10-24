#pragma once

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

inline std::vector<ServiceRow> collect_services(std::string &error,
                                                const std::atomic<bool> &stop) {
  std::map<std::string, ServiceRow> services;
  const char *scopes[] = {"System", "User"};
  const char *commands[] = {
      "LC_ALL=C SYSTEMD_COLORS=0 timeout 3s systemctl show --all --no-pager "
      "--property=Id,Description,ActiveState,SubState,MainPID '*.service' "
      "2>/dev/null",
      "LC_ALL=C SYSTEMD_COLORS=0 timeout 3s systemctl --user show --all "
      "--no-pager --property=Id,Description,ActiveState,SubState,MainPID "
      "'*.service' 2>/dev/null"};

  for (int i = 0; i < 2 && !stop.load(); ++i) {
    FILE *pipe = popen(commands[i], "r");
    if (!pipe) {
      error +=
          "Failed to open pipe for scope: " + std::string(scopes[i]) + "; ";
      continue;
    }

    char buffer[4096];
    ServiceRow row;
    bool in_block = false;

    while (fgets(buffer, sizeof(buffer), pipe)) {
      if (stop.load())
        break;

      std::istringstream line(buffer);
      std::string key, value;
      if (std::getline(line, key, '=') && std::getline(line, value)) {
        while (!value.empty() &&
               (value.back() == '\n' || value.back() == '\r')) {
          value.pop_back();
        }

        if (key == "Id") {
          row.unit = value;
          in_block = true;
        } else if (key == "Description" && !value.empty()) {
          row.description = value;
        } else if (key == "ActiveState") {
          row.state = value;
        } else if (key == "SubState") {
          row.substate = value;
        } else if (key == "MainPID") {
          try {
            row.pid = std::stoul(value);
          } catch (...) {
            row.pid = 0;
          }
        }
      }

      if (std::strcmp(buffer, "\n") == 0 || std::strcmp(buffer, "\r\n") == 0) {
        if (in_block && row.unit.size() > 8 &&
            row.unit.substr(row.unit.size() - 8) == ".service") {
          if (row.description.empty()) {
            row.description = row.unit;
          }
          if (row.state.empty()) {
            row.state = "unknown";
          }
          row.scope = scopes[i];
          services[std::string(scopes[i]) + "/" + row.unit] = row;
        }
        in_block = false;
        row = ServiceRow{};
      }
    }

    if (in_block && row.unit.size() > 8 &&
        row.unit.substr(row.unit.size() - 8) == ".service") {
      if (row.description.empty()) {
        row.description = row.unit;
      }
      if (row.state.empty()) {
        row.state = "unknown";
      }
      row.scope = scopes[i];
      services[std::string(scopes[i]) + "/" + row.unit] = row;
    }
    int status = pclose(pipe);
    if (status != 0) {
      error +=
          "Failed to close pipe for scope: " + std::string(scopes[i]) + "; ";
    }
  }

  std::vector<ServiceRow> result;
  for (const auto &pair : services) {
    result.push_back(pair.second);
  }

  return result;
}
