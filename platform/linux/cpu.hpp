#pragma once
#include <fstream>
#include <sstream>
namespace monitoring {
class CpuCollector {
  struct Counters {
    unsigned long long total = 0;
    unsigned long long idle = 0;
  };
  std::map<unsigned, Counters> previous;
  static auto read_cpus() -> std::map<unsigned, Counters> {
    std::map<unsigned, Counters> result;
    std::ifstream file("/proc/stat");
    if (!file.is_open())
      return result;

    std::string line;
    while (std::getline(file, line)) {
      if (line.find("cpu ") == 0)
        continue; // Skip aggregate cpu
      if (line.find("cpu") != 0 || !isdigit(line[3]))
        break; // Stop on non-cpu line

      unsigned id = std::stoi(line.substr(3));
      Counters counters;
      unsigned long long array[8] = {0};
      std::istringstream iss(line);
      std::string ignored;
      iss >> ignored;
      bool valid = true;
      for (int i = 0; i < 8; ++i) {
        if (!(iss >> array[i])) {
          valid = false;
          break;
        }
      }
      if (!valid)
        continue;
      counters.total = 0;
      for (int i = 0; i < 8; ++i) {
        counters.total += array[i];
      }
      counters.idle = array[3] + array[4];

      result[id] = counters;
    }

    return result;
  }

public:
 std::map<unsigned,double> sample() {
  auto current=read_cpus(); std::map<unsigned,double> result;
  for (const auto& [id,c]:current) {
   double percent=-1;
   if(previous.count(id) && c.total>=previous[id].total && c.idle>=previous[id].idle) {
    auto dt=c.total-previous[id].total, di=c.idle-previous[id].idle;
    if(dt>0 && di<=dt) percent=100.0*(dt-di)/dt;
   }
   result[id]=percent;
  }
  previous=std::move(current); return result;
 }
};
}
