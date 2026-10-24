#pragma once
#include <atomic>
#include <string>
#include <vector>
struct ConnectionRow {
  std::string application = "Unknown", protocol, state, local_address,
              local_port, remote_address, remote_port;
  int pid = 0, local_key = 0, remote_key = 0;
};


#ifdef _WIN32
#include "platform/windows/connection_data.hpp"
#else
#include "platform/linux/connection_data.hpp"
#endif
