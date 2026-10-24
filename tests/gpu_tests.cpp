#include "gpu_data.hpp"
#include "gpu_fixture.hpp"
#include <dlfcn.h>
#include <iostream>
using namespace community::gpu;
int main(int argc,char** argv){assert(argc>1);std::string test=argv[1];GpuFixture f;
 if(test=="amd-w5500"||test=="amd-wx3200"){
  const bool w=test=="amd-w5500";auto p=f.add("0003:e7:1f.7","0x1002",w?"0x7341":"0x6981","amdgpu","card11");put(p/"gpu_busy_percent","0");put(p/"mem_info_vram_total",w?"8589934592":"4294967296");put(p/"mem_info_vram_used","536870912");put(p/"hwmon/hwmon42/temp1_input","41500");put(p/"hwmon/hwmon42/power1_average","16500000");auto d=f.devices();assert(d.size()==1&&d[0].name.find(w?"W5500":"WX 3200")!=std::string::npos);auto m=read(d[0]);assert(m.utilization&&*m.utilization==0&&m.temperature&&*m.temperature==41.5&&m.power&&*m.power==16.5);std::vector<GpuReading> readings;auto text=gpu_text(d,&readings);assert(text.find("AMD Radeon")!=std::string::npos&&text.find("0.0%")!=std::string::npos&&text.find("512.0 MiB")!=std::string::npos&&readings.size()==1&&readings[0].identity==identity_id(d[0]));
 }else if(test=="intel-integrated"||test=="intel-discrete"){
  auto p=f.add("0000:00:02.0","0x8086",test=="intel-integrated"?"0x5912":"0x56a0",test=="intel-integrated"?"i915":"xe");auto d=f.devices();assert(d.size()==1&&d[0].vendor=="0x8086");std::vector<GpuReading> r;auto text=gpu_text(d,&r);assert(text.find("Intel")!=std::string::npos&&text.find("AMD")==std::string::npos&&text.find("VRAM Used: N/A")!=std::string::npos&&r[0].usage==-1);put(p/"hwmon/hwmon0/temp1_input","55000");assert(read(d[0]).temperature==55.);
 }else if(test=="nvidia-nouveau"||test=="nvidia-no-driver"||test=="nvidia-professional"){
  auto p=f.add("0002:fe:08.1","0x10de",test=="nvidia-professional"?"0x1bb0":"0x1c81",test=="nvidia-nouveau"?"nouveau":"","");auto d=f.devices();assert(d.size()==1&&d[0].name.find(test=="nvidia-professional"?"Quadro":"GTX 1050")!=std::string::npos);auto text=gpu_text(d);assert(text.find("NVIDIA")!=std::string::npos&&text.find("AMD")==std::string::npos&&text.find("GPU: N/A")!=std::string::npos);put(p/"hwmon/hwmon9/temp1_input","47000");assert(read(d[0]).temperature==47.);
 }else if(test=="missing-invalid"){
  auto p=f.add("0000:80:01.0","0x1002","0x7341","amdgpu");put(p/"gpu_busy_percent","101");put(p/"mem_info_vram_total","100");put(p/"mem_info_vram_used","200");put(p/"hwmon/hwmon0/temp1_input","999999");auto m=read(f.devices()[0]);assert(!m.utilization&&!m.used&&!m.temperature&&m.total==100);put(p/"gpu_busy_percent","0");put(p/"mem_info_vram_used","0");auto z=read(f.devices()[0]);assert(z.utilization==0&&z.used==0);assert(gpu_text({}).find("AMD")==std::string::npos);
 }else if(test=="replacement"){
  auto p=f.add("0000:03:00.0","0x1002","0x7341","amdgpu");put(p/"gpu_busy_percent","25");auto a=f.devices()[0];put(p/"vendor","0x8086");put(p/"device","0x5912");fs::remove(p/"driver");fs::create_directories(f.root/"drivers/i915");fs::create_directory_symlink(f.root/"drivers/i915",p/"driver");assert(!read(a).utilization);auto b=f.devices()[0];assert(identity_id(a)!=identity_id(b)&&b.name.find("Intel")!=std::string::npos&&gpu_text({b}).find("W5500")==std::string::npos);
 }else if(test=="identity"){
  auto p=f.add("0000:03:00.0","0x1002","0x7341","amdgpu");put(p/"serial","serial-A");auto a=f.devices()[0];auto moved=a;moved.pci_id="0002:80:05.0";assert(identity_id(a)==identity_fingerprint(moved));put(p/"serial","serial-B");assert(identity_id(a)!=identity_id(f.devices()[0]));put(p/"serial","0000000");assert(identity_id(a)!=identity_id(f.devices()[0]));
 }else if(test=="drm-pci-merge"){
  auto p=f.add("0001:ff:1e.0","0x1002","0x6981","amdgpu","card29");fs::create_directories(f.drm/"card30");fs::create_directory_symlink(p,f.drm/"card30/device");auto d=f.devices();assert(d.size()==1);fs::remove_all(f.drm/"card29");fs::remove_all(f.drm/"card30");assert(f.devices().size()==1);fs::remove_all(p);assert(f.devices().empty());
 }else if(test=="nvml"){
  assert(argc>2);Nvml provider(argv[2]);f.add("0000:ab:07.0","0x10de","0x1c81","nvidia","");auto d=f.devices();provider.augment(d);assert(d.size()==1&&d[0].uuid.find("GPU-")==0&&d[0].name=="NVIDIA GeForce GTX 1050");std::vector<GpuReading> r;auto text=gpu_text(d,&r,&provider);assert(r[0].usage==0&&text.find("512.0 MiB")!=std::string::npos&&text.find("42.0")!=std::string::npos);auto lib=dlopen(argv[2],RTLD_NOW);assert(lib);reinterpret_cast<void(*)()>(dlsym(lib,"community_fixture_unsupported"))();text=gpu_text(d,&r,&provider);assert(r[0].usage==-1&&text.find("VRAM Used: N/A")!=std::string::npos);reinterpret_cast<void(*)()>(dlsym(lib,"community_fixture_replace"))();Metrics m;provider.read_into(d[0],m);assert(!m.used&&!m.utilization);auto newer=f.devices();provider.augment(newer);assert(identity_id(d[0])!=identity_id(newer[0]));dlclose(lib);
 }else assert(false);
 std::cout<<"PASS fixture "<<test<<"\n";
}
