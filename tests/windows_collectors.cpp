#include "connection_data.hpp"
#include "gpu_data.hpp"
#include "platform/cpu.hpp"
#include "platform/process.hpp"
#include "service_data.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <thread>
int main() {
  std::atomic<bool> stop{false};
  std::string errors;
  monitoring::ProcessCollector processes;
  auto first = processes.sample();
  assert(!first.empty());
  bool self = false;
  for (const auto &row : first)
    if (row.pid == int(GetCurrentProcessId()))
      self = true;
  assert(self);
  std::this_thread::sleep_for(std::chrono::milliseconds(250));
  auto second = processes.sample();
  self = false;
  for (const auto &row : second)
    if (row.pid == int(GetCurrentProcessId())) {
      self = true;
      assert(std::isfinite(row.cpu) && row.cpu >= 0);
      assert(row.memory > 0);
    }
  assert(self);
  assert(monitoring::uptime_seconds().value_or(-1) >= 0);
  monitoring::CpuCollector cpus;
  auto readings = cpus.sample();
  assert(!readings.empty());
  for (const auto &[id, value] : readings)
    assert(value == -1 || (std::isfinite(value) && value >= 0 && value <= 100));
  auto services = collect_services(errors, stop);
  std::cout << "services=" << services.size() << " warnings=" << errors << "\n";
  errors.clear();
  WSADATA winsock{};
  assert(WSAStartup(MAKEWORD(2, 2), &winsock) == 0);
  SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  SOCKET datagram = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  assert(listener != INVALID_SOCKET && datagram != INVALID_SOCKET);
  sockaddr_in endpoint{};
  endpoint.sin_family = AF_INET;
  endpoint.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  assert(bind(listener, reinterpret_cast<sockaddr *>(&endpoint),
              sizeof(endpoint)) == 0);
  assert(listen(listener, 1) == 0);
  int address_length = sizeof(endpoint);
  assert(getsockname(listener, reinterpret_cast<sockaddr *>(&endpoint),
                     &address_length) == 0);
  int tcp_port = ntohs(endpoint.sin_port);
  endpoint.sin_port = 0;
  assert(bind(datagram, reinterpret_cast<sockaddr *>(&endpoint),
              sizeof(endpoint)) == 0);
  address_length = sizeof(endpoint);
  assert(getsockname(datagram, reinterpret_cast<sockaddr *>(&endpoint),
                     &address_length) == 0);
  int udp_port = ntohs(endpoint.sin_port);
  auto connections = collect_connections(errors, stop);
  for (const auto &row : connections) {
    assert(row.local_key >= 0 && row.local_key <= 65535);
    assert(!row.protocol.empty());
  }
  bool found_tcp = false, found_udp = false;
  for (const auto &row : connections) {
    if (row.pid != int(GetCurrentProcessId()))
      continue;
    if (row.protocol == "TCP" && row.local_key == tcp_port &&
        row.state == "LISTEN")
      found_tcp = true;
    if (row.protocol == "UDP" && row.local_key == udp_port &&
        row.state == "UNCONN")
      found_udp = true;
  }
  closesocket(listener);
  closesocket(datagram);
  WSACleanup();
  assert(found_tcp && found_udp);
  std::cout << "connections=" << connections.size() << " warnings=" << errors
            << "\n";
  LUID luid{0x12345678, LONG(0xabcdef01)};
  assert(gpu_identity(luid) == "luid_0xabcdef01_0x12345678");
  auto identity = monitoring::win::adapter_identity(
      L"pid_123_luid_0xABCDEF01_0x12345678_phys_0_eng_0_engtype_3D");
  assert(identity && *identity == gpu_identity(luid));
  assert(!monitoring::win::adapter_identity(L"malformed"));
  auto aggregated = monitoring::win::GpuCounters::aggregate(
      {{L"pid_1_luid_0xABCDEF01_0x12345678_phys_0_eng_0_engtype_3D", 30},
       {L"pid_2_luid_0xABCDEF01_0x12345678_phys_0_eng_0_engtype_3D", 40},
       {L"pid_3_luid_0xABCDEF01_0x12345678_phys_0_eng_1_engtype_Copy", 60},
       {L"pid_4_luid_0x00000000_0x00000002_phys_0_eng_0_engtype_3D", 0}},
      {{L"luid_0xABCDEF01_0x12345678_phys_0", 1024},
       {L"luid_0xABCDEF01_0x12345678_phys_1", 2048}});
  assert(aggregated.at(*identity).utilization.value() == 70);
  assert(aggregated.at(*identity).used.value() == 3072);
  assert(aggregated.at("luid_0x00000000_0x00000002").utilization.value() == 0);
  assert(!aggregated.at("luid_0x00000000_0x00000002").used);
  assert(monitoring::win::GpuCounters::aggregate({}, {}).empty());
  auto devices = discover_gpus();
  std::vector<GpuReading> samples;
  auto text = gpu_text(devices, &samples);
  assert(!text.empty());
  assert(samples.size() == devices.size());
  std::cout << "processes=" << second.size()
            << " logical_cpus=" << readings.size()
            << " dxgi_adapters=" << devices.size() << "\n";
  std::cout << "PASS Windows collector smoke and adapter identity parsing; "
               "hardware accuracy requires native validation\n";
}
