#pragma once

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct ConnectionRow {
  std::string application = "Unknown", protocol, state, local_address,
              local_port, remote_address, remote_port;
  int pid = 0, local_key = 0, remote_key = 0;
};

inline void split_endpoint(const std::string &input, bool ipv6,
                           std::string &address, std::string &port, int &key) {
  size_t last_colon = input.rfind(':');
  if (last_colon != std::string::npos) {
    address = input.substr(0, last_colon);
    port = input.substr(last_colon + 1);

    if (address.front() == '[' && address.back() == ']') {
      address = address.substr(1, address.size() - 2);
    }

    if (port == "*") {
      key = 0;
    } else {
      try {
        int port_num = std::stoi(port);
        if (port_num < 0 || port_num > 65535) {
          throw std::out_of_range("Port out of range");
        }
        port = std::to_string(port_num);
        key = port_num;
      } catch (...) {
        key = 0;
      }
    }

    if (address == "*") {
      address = ipv6 ? "::" : "0.0.0.0";
    }
  } else {
    address = input;
    port = "*";
    key = 0;
  }
}

inline std::vector<ConnectionRow>
collect_connections(std::string &error, const std::atomic<bool> &stop) {
  std::vector<ConnectionRow> connections;
  const char *commands[] = {
      "LC_ALL=C timeout 3s ss -H -a -n -t -u -p -O -4 2>/dev/null",
      "LC_ALL=C timeout 3s ss -H -a -n -t -u -p -O -6 2>/dev/null"};
  const std::string suffixes[] = {"", "6"};

  for (size_t i = 0; i < 2 && !stop.load(); ++i) {
    FILE *pipe = popen(commands[i], "r");
    if (!pipe) {
      error += "Failed to execute ss command\n";
      continue;
    }

    char *buffer = nullptr;
    size_t capacity = 0;
    while (::getline(&buffer, &capacity, pipe) > 0 && !stop.load()) {
      if (stop.load())
        break;

      std::istringstream line(buffer);
      std::string netid, state, recvq, sendq, local, remote;
      line >> netid >> state >> recvq >> sendq >> local >> remote;

      if (netid != "tcp" && netid != "udp")
        continue;

      ConnectionRow row;
      row.protocol = netid == "tcp" ? "TCP" : "UDP";
      row.protocol += suffixes[i];
      row.state = state;
      split_endpoint(local, i == 1, row.local_address, row.local_port,
                     row.local_key);
      split_endpoint(remote, i == 1, row.remote_address, row.remote_port,
                     row.remote_key);

      std::string owners;
      std::getline(line, owners);
      static const std::regex owner_regex(R"OWNER("([^"]*)",pid=(\d+))OWNER");
      std::set<int> seen_pids;
      for (auto it =
               std::sregex_iterator(owners.begin(), owners.end(), owner_regex);
           it != std::sregex_iterator(); ++it) {
        if (stop.load())
          break;
        const auto &match = *it;
        try {
          int pid = std::stoi(match[2].str());
          if (pid > 0 && seen_pids.insert(pid).second) {
            ConnectionRow owned = row;
            owned.pid = pid;
            owned.application =
                match[1].str().empty() ? "Unknown" : match[1].str();
            connections.push_back(owned);
          }
        } catch (...) {
        }
      }
      if (seen_pids.empty()) {
        connections.push_back(row);
      }
    }

    free(buffer);
    int status = pclose(pipe);
    if (status != 0) {
      error += "ss command failed\n";
    }
  }

  return connections;
}
