#pragma once
#include "pci_gpu.hpp"
#include <map>
namespace community::gpu {
inline std::string lower_id(std::string s){for(char& c:s)c=char(std::tolower(static_cast<unsigned char>(c)));return s;}
inline std::string stable_id(const Device& d){return std::regex_match(d.pci_id,pattern)?"pci-"+lower_id(d.pci_id):d.card;}
inline std::string pci_model(const Device& d,const std::filesystem::path& database){
 auto vendor=lower_id(line(std::filesystem::path(d.path)/"vendor"));auto device=lower_id(line(std::filesystem::path(d.path)/"device"));if(vendor.size()!=6||device.size()!=6||vendor.substr(0,2)!="0x"||device.substr(0,2)!="0x")return {};vendor.erase(0,2);device.erase(0,2);std::ifstream file(database);std::string text,manufacturer;size_t bytes=0;unsigned lines=0;bool selected=false;
 auto next=[&]{text.clear();char c;bool long_line=false;while(bytes<10*1024*1024&&file.get(c)){++bytes;if(c=='\n'){if(++lines>250000)return false;if(long_line)text.clear();return true;}if(text.size()<512)text+=c;else long_line=true;}return !long_line&&!text.empty()&&++lines<=250000;};
 while(next()){if(text.empty()||text[0]=='#')continue;
  if(text[0]!='\t'){if(text.size()<6||text[4]!=' ')continue;selected=lower_id(text.substr(0,4))==vendor;if(selected){const auto start=text.find_first_not_of(" \t",4);manufacturer=start==text.npos?"":text.substr(start);}else if(!manufacturer.empty())break;}
  else if(selected&&text.size()>7&&text[1]!='\t'&&text[5]==' '&&lower_id(text.substr(1,4))==device){auto at=text.find_first_not_of(" \t",5);if(at==text.npos)return {};return manufacturer+" "+text.substr(at);}
 }return {};
}
inline std::vector<Device> enumerate(const std::filesystem::path& drm="/sys/class/drm",const std::filesystem::path& pci="/sys/bus/pci/devices",const std::filesystem::path& database="/usr/share/hwdata/pci.ids"){
 auto devices=discover(drm);for(auto d:discover_pci(pci)){auto found=std::find_if(devices.begin(),devices.end(),[&](const auto& a){return lower_id(a.pci_id)==lower_id(d.pci_id);});if(found==devices.end()){devices.push_back(std::move(d));}else{if(found->driver.empty())found->driver=d.driver;if(found->vendor.empty())found->vendor=d.vendor;}}
 for(auto& d:devices){d.vendor=lower_id(d.vendor);const auto product=line(std::filesystem::path(d.path)/"product_name");if(!product.empty())d.name=product;else{auto name=pci_model(d,database);if(!name.empty())d.name=name;else {const auto vendor=d.vendor=="0x1002"?"AMD":d.vendor=="0x10de"?"NVIDIA":d.vendor=="0x8086"?"Intel":"GPU";auto device=line(std::filesystem::path(d.path)/"device");d.name=std::string(vendor)+" GPU (model unavailable; "+d.vendor+":"+(device.empty()?"unknown":device)+"; "+d.pci_id+")";}}d.pci_id=lower_id(d.pci_id);d.identity=identity_fingerprint(d);}
 std::sort(devices.begin(),devices.end(),[](const auto& a,const auto& b){return a.pci_id==b.pci_id?a.card<b.card:a.pci_id<b.pci_id;});return devices;
}
}
