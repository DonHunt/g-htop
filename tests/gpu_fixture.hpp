#pragma once
#include "gpu/enumerate_gpu.hpp"
#include <cassert>
#include <unistd.h>
namespace fs=std::filesystem;
inline void put(const fs::path& p,const std::string& value){fs::create_directories(p.parent_path());std::ofstream(p)<<value;}
struct GpuFixture {
 fs::path root,drm,pci,database;
 GpuFixture(){char t[]="/tmp/community-gpu-fixture-XXXXXX";assert(mkdtemp(t));root=t;drm=root/"drm";pci=root/"pci";database=root/"pci.ids";fs::create_directories(drm);fs::create_directories(pci);put(database,"1002  Advanced Micro Devices, Inc. [AMD/ATI]\n\t7341  Navi 14 [Radeon Pro W5500]\n\t6981  Lexa XT [Radeon PRO WX 3200]\n10de  NVIDIA Corporation\n\t1c81  GP107 [GeForce GTX 1050]\n\t1bb0  Quadro P5000\n8086  Intel Corporation\n\t5912  HD Graphics 630\n\t56a0  Arc A770\n");}
 ~GpuFixture(){fs::remove_all(root);}
 fs::path add(const std::string& bdf,const std::string& vendor,const std::string& device,const std::string& driver,const std::string& card="card0"){
  auto p=pci/bdf;put(p/"class","0x030000\n");put(p/"vendor",vendor);put(p/"device",device);put(p/"subsystem_vendor",vendor);put(p/"subsystem_device","0x0001");put(p/"revision","0x01");
  if(!driver.empty()){fs::create_directories(root/"drivers"/driver);fs::create_directory_symlink(root/"drivers"/driver,p/"driver");}
  if(!card.empty()){fs::create_directories(drm/card);fs::create_directory_symlink(p,drm/card/"device");}return p;
 }
 std::vector<community::gpu::Device> devices()const{return community::gpu::enumerate(drm,pci,database);}
};
