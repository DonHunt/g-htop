#pragma once
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <iomanip>

namespace community::gpu {
struct Device {std::string card,path,name,driver,pci_id,vendor,uuid,identity;};
struct Metrics {std::optional<unsigned long long> used,total;std::optional<double> utilization,temperature,power;};
inline std::string line(const std::filesystem::path& path) {
    std::ifstream file(path);std::string text;
    for(char c;file.get(c)&&c!='\n'&&text.size()<512;)text+=c;
    while(!text.empty()&&std::isspace(static_cast<unsigned char>(text.back())))text.pop_back();
    return text;
}
inline bool usable_unique(std::string id){
    std::transform(id.begin(),id.end(),id.begin(),[](unsigned char c){return char(std::tolower(c));});
    if(id.empty()||id=="unknown"||id=="none"||id=="n/a"||id=="default string"||id=="to be filled by o.e.m.")return false;
    if(id.compare(0,2,"0x")==0)id.erase(0,2);
    return std::any_of(id.begin(),id.end(),[](unsigned char c){return c!='0'&&c!='-'&&!std::isspace(c);});
}
// Hardware identity is separate from the PCI location and DRM node. Never hash driver/name.
inline std::string identity_fingerprint(const Device& d) {
    std::string raw;
    auto file_uuid=d.path.empty()?std::string{}:line(std::filesystem::path(d.path)/"uuid");
    if(!usable_unique(file_uuid)&&!d.path.empty())file_uuid=line(std::filesystem::path(d.path)/"gpu_uuid");
    if(usable_unique(d.uuid)) raw="uuid:"+d.uuid;
    else if(usable_unique(file_uuid))raw="uuid:"+file_uuid;
    else {
        const auto unique=d.path.empty()?std::string{}:line(std::filesystem::path(d.path)/"unique_id");
        const auto serial=d.path.empty()?std::string{}:line(std::filesystem::path(d.path)/"serial");
        const auto discriminator=usable_unique(unique)?unique:(usable_unique(serial)?serial:std::string{});
        raw=discriminator.empty()?"location:"+d.pci_id:"serial:"+discriminator;
    }
    auto numeric=[](std::string value){std::transform(value.begin(),value.end(),value.begin(),[](unsigned char c){return char(std::tolower(c));});return value;};
    raw+="|"+numeric(d.vendor);
    if(!d.path.empty())for(const auto* key:{"device","subsystem_vendor","subsystem_device","revision"}) raw+="|"+numeric(line(std::filesystem::path(d.path)/key));
    std::uint64_t hash=14695981039346656037ULL;
    for(unsigned char c:raw){hash^=c;hash*=1099511628211ULL;}
    std::ostringstream text;text<<"gpu-"<<std::hex<<std::setfill('0')<<std::setw(16)<<hash;return text.str();
}
inline std::string identity_id(const Device& d){return d.identity.empty()?identity_fingerprint(d):d.identity;}
inline bool bound_id(const std::string& s){return s.size()==20&&s.compare(0,4,"gpu-")==0&&std::all_of(s.begin()+4,s.end(),[](unsigned char c){return (c>='0'&&c<='9')||(c>='a'&&c<='f');});}
inline bool matches(const Device& d,const std::string& selected){return selected.empty()||selected==identity_id(d)||selected==d.card||selected=="pci-"+d.pci_id||(!d.uuid.empty()&&selected=="nvml-"+d.uuid);}
inline std::optional<unsigned long long> integer(const std::filesystem::path& path) {
    const auto text=line(path);const auto first=text.find_first_not_of(" \t\r\n");
    if(first==std::string::npos)return {};
    const auto digits=text.substr(first);
    if(!std::all_of(digits.begin(),digits.end(),[](unsigned char c){return c>='0'&&c<='9';}))return {};
    try {return std::stoull(digits);}catch(...){return {};}
}
inline bool card_id(const std::string& name) {
    return name.size()>4&&name.size()<=9&&name.compare(0,4,"card")==0&&std::all_of(name.begin()+4,name.end(),[](unsigned char c){return c>='0'&&c<='9';});
}
inline bool selection_id(const std::string& name) {
    if(name.empty()||card_id(name)||bound_id(name))return true;
    if(name.size()==16&&name.compare(0,4,"pci-")==0){const auto id=name.substr(4);for(size_t i=0;i<id.size();++i){if(i==4||i==7){if(id[i]!=':')return false;}else if(i==10){if(id[i]!='.')return false;}else if(!std::isxdigit(static_cast<unsigned char>(id[i])))return false;}return id.back()>='0'&&id.back()<='7';}
    return name.size()>9&&name.size()<128&&name.compare(0,9,"nvml-GPU-")==0&&std::all_of(name.begin()+9,name.end(),[](unsigned char c){return std::isxdigit(c)||c=='-';});
}
inline std::vector<Device> discover(const std::filesystem::path& root="/sys/class/drm") {
    std::vector<Device> devices;std::set<std::string> seen;std::error_code ec;
    std::filesystem::directory_iterator at(root,ec),end;
    std::vector<std::filesystem::path> paths;
    for(;!ec&&at!=end;at.increment(ec))if(card_id(at->path().filename().string()))paths.push_back(at->path());
    std::sort(paths.begin(),paths.end());
    for(const auto& card:paths) {
        std::error_code error;const auto path=std::filesystem::canonical(card/"device",error);
        if(error||!seen.insert(path.string()).second)continue;
        Device d;d.card=card.filename().string();d.path=path.string();d.pci_id=path.filename().string();
        const auto driver=std::filesystem::read_symlink(path/"driver",error);if(!error)d.driver=driver.filename().string();
        d.vendor=line(path/"vendor");d.name=line(path/"product_name");
        if(d.name.empty()) {
            const std::string vendor=d.vendor=="0x1002"?"AMD":d.vendor=="0x10de"?"NVIDIA":d.vendor=="0x8086"?"Intel":"GPU";
            d.name=vendor+(d.driver.empty()?"":" "+d.driver)+" ("+d.pci_id+")";
        }
        devices.push_back(std::move(d));
    }
    std::sort(devices.begin(),devices.end(),[](const auto& a,const auto& b){return a.card<b.card;});return devices;
}
inline Metrics read(const Device& d) {
    Metrics m;const std::filesystem::path path=d.path;
    if(path.empty())return m;
    // A captured device replaced before this read must never acquire its successor's counters.
    if(!d.identity.empty()){auto current=d;current.vendor=line(path/"vendor");if(identity_fingerprint(current)!=d.identity)return m;}
    const auto busy=integer(path/"gpu_busy_percent");if(busy&&*busy<=100)m.utilization=*busy;
    m.used=integer(path/"mem_info_vram_used");m.total=integer(path/"mem_info_vram_total");
    if(m.used&&m.total&&*m.used>*m.total)m.used.reset();
    std::error_code ec;std::filesystem::directory_iterator at(path/"hwmon",ec),end;
    for(;!ec&&at!=end;at.increment(ec)) {
        const auto name=at->path().filename().string();if(name.compare(0,5,"hwmon")!=0)continue;
        const auto value=integer(at->path()/"temp1_input");
        if(value&&*value<=200000&&!m.temperature)m.temperature=*value/1000.;
        auto power=integer(at->path()/"power1_average");if(!power)power=integer(at->path()/"power1_input");if(power&&!m.power)m.power=static_cast<double>(*power)/1000000.;
        if(m.temperature&&m.power)break;
    }
    return m;
}
}
