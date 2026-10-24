#pragma once
#include "linux_gpu.hpp"
#include <dlfcn.h>
#include <optional>
#include <atomic>
#include <mutex>

namespace community::gpu {
// Read-only optional NVML v1 ABI, documented by NVIDIA. No SDK or driver is required to launch.
// Memory ABI: total/free/used bytes. Utilization ABI: gpu/memory unsigned percentages.
class Nvml {
    using Handle=void*;using Result=int;
    struct Memory {unsigned long long total,free,used;};
    struct Utilization {unsigned gpu,memory;};
    void* library_=nullptr;mutable std::atomic<bool> ready_{false};mutable std::mutex init_mutex_;
    Result (*init_)()=nullptr;Result (*shutdown_)()=nullptr;
    Result (*count_)(unsigned*)=nullptr;Result (*index_)(unsigned,Handle*)=nullptr;
    Result (*pci_)(const char*,Handle*)=nullptr;Result (*uuid_handle_)(const char*,Handle*)=nullptr;
    Result (*uuid_)(Handle,char*,unsigned)=nullptr;Result (*name_)(Handle,char*,unsigned)=nullptr;
    Result (*memory_)(Handle,Memory*)=nullptr;Result (*utilization_)(Handle,Utilization*)=nullptr;
    Result (*temperature_)(Handle,int,unsigned*)=nullptr;
    Result (*power_)(Handle,unsigned*)=nullptr;
    template<class T>void bind(T& fn,const char* name){fn=reinterpret_cast<T>(::dlsym(library_,name));}
    bool ensure_ready()const {if(ready_)return true;std::lock_guard<std::mutex> lock(init_mutex_);if(!ready_&&init_&&shutdown_)ready_=init_()==0;return ready_;}
    Handle handle(const Device& d) const {
        Handle h=nullptr;if(!ensure_ready())return h;
        if(!d.uuid.empty()){if(uuid_handle_&&uuid_handle_(d.uuid.c_str(),&h)==0)return h;return nullptr;}
        if(!d.pci_id.empty()&&pci_&&pci_(d.pci_id.c_str(),&h)==0)return h;
        return nullptr;
    }
public:
    explicit Nvml(const char* library="libnvidia-ml.so.1") {
        library_=::dlopen(library,RTLD_LOCAL|RTLD_LAZY);if(!library_)return;
        bind(init_,"nvmlInit_v2");bind(shutdown_,"nvmlShutdown");bind(count_,"nvmlDeviceGetCount_v2");bind(index_,"nvmlDeviceGetHandleByIndex_v2");
        bind(pci_,"nvmlDeviceGetHandleByPciBusId_v2");bind(uuid_handle_,"nvmlDeviceGetHandleByUUID");bind(uuid_,"nvmlDeviceGetUUID");bind(name_,"nvmlDeviceGetName");
        bind(memory_,"nvmlDeviceGetMemoryInfo");bind(utilization_,"nvmlDeviceGetUtilizationRates");bind(temperature_,"nvmlDeviceGetTemperature");
        bind(power_,"nvmlDeviceGetPowerUsage");
        ready_=init_&&shutdown_&&init_()==0;
    }
    ~Nvml(){if(ready_)shutdown_();if(library_)::dlclose(library_);}
    Nvml(const Nvml&)=delete;Nvml& operator=(const Nvml&)=delete;
    std::string uuid_for(const Device& d) const {
        const auto h=handle(d);char id[256]={};if(h&&uuid_&&uuid_(h,id,sizeof(id))==0&&id[0]&&id[255]==0)return id;return {};
    }
    void augment(std::vector<Device>& devices) const {
        if(!ensure_ready()||!count_||!index_||!uuid_)return;
        std::set<std::string> seen;
        for(auto& d:devices)if(d.vendor=="0x10de"){d.uuid=uuid_for(d);if(!d.uuid.empty())seen.insert(d.uuid);auto h=handle(d);char name[256]={};if(h&&name_&&name_(h,name,sizeof(name))==0&&name[0]&&name[255]==0)d.name=name;d.identity=identity_fingerprint(d);}
        unsigned count=0;if(count_(&count)!=0)return;
        for(unsigned i=0;i<std::min(count,64U);++i) {
            Handle h=nullptr;char id[256]={},name[256]={};
            if(index_(i,&h)!=0||!h||uuid_(h,id,sizeof(id))!=0||!id[0]||id[255]!=0)continue;
            if(!seen.insert(id).second)continue;
            Device d;d.uuid=id;d.card="nvml-"+d.uuid;if(!selection_id(d.card))continue;
            d.vendor="0x10de";d.driver="nvidia";
            if(name_&&name_(h,name,sizeof(name))==0&&name[255]==0)d.name=name;
            if(d.name.empty())d.name="NVIDIA "+d.uuid;
            d.identity=identity_fingerprint(d);devices.push_back(std::move(d));
        }
    }
    void read_into(const Device& d,Metrics& m) const {
        if(d.vendor!="0x10de")return;
        const auto h=handle(d);if(!h)return;
        Memory mem{};if(memory_&&memory_(h,&mem)==0){if(!m.total)m.total=mem.total;if(!m.used&&mem.used<=mem.total)m.used=mem.used;}
        Utilization util{};if(!m.utilization&&utilization_&&utilization_(h,&util)==0&&util.gpu<=100)m.utilization=util.gpu;
        unsigned temperature=0;if(!m.temperature&&temperature_&&temperature_(h,0,&temperature)==0&&temperature<=200)m.temperature=temperature;
        unsigned power=0;if(!m.power&&power_&&power_(h,&power)==0)m.power=power/1000.;
    }
};
inline Nvml& nvml(){static Nvml adapter;return adapter;}
}
