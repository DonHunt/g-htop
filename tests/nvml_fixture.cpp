// Synthetic NVML ABI test double: no physical NVIDIA hardware is implied.
#include <cstring>
namespace {bool replaced=false,unsupported=false;const char* id(){return replaced?"GPU-bbbbbbbb-bbbb-bbbb-bbbb-bbbbbbbbbbbb":"GPU-aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa";}int text(const char* s,char* dst,unsigned n){if(n<=std::strlen(s))return 1;std::strcpy(dst,s);return 0;}}
extern "C" {
int nvmlInit_v2(){return 0;}int nvmlShutdown(){return 0;}
int nvmlDeviceGetCount_v2(unsigned* n){*n=1;return 0;}
int nvmlDeviceGetHandleByIndex_v2(unsigned n,void** h){if(n)return 1;*h=reinterpret_cast<void*>(1);return 0;}
int nvmlDeviceGetHandleByPciBusId_v2(const char* bdf,void** h){if(std::strcmp(bdf,"0000:ab:07.0"))return 1;*h=reinterpret_cast<void*>(1);return 0;}
int nvmlDeviceGetHandleByUUID(const char* uuid,void** h){if(std::strcmp(uuid,id()))return 1;*h=reinterpret_cast<void*>(1);return 0;}
int nvmlDeviceGetUUID(void*,char* dst,unsigned n){return text(id(),dst,n);}
int nvmlDeviceGetName(void*,char* dst,unsigned n){return text("NVIDIA GeForce GTX 1050",dst,n);}
struct Memory{unsigned long long total,free,used;};int nvmlDeviceGetMemoryInfo(void*,Memory* m){if(unsupported)return 3;*m={2147483648ULL,1610612736ULL,536870912ULL};return 0;}
struct Util{unsigned gpu,memory;};int nvmlDeviceGetUtilizationRates(void*,Util* v){if(unsupported)return 3;*v={0,0};return 0;}
int nvmlDeviceGetTemperature(void*,int,unsigned* t){if(unsupported)return 3;*t=42;return 0;}
int nvmlDeviceGetPowerUsage(void*,unsigned* p){if(unsupported)return 3;*p=12000;return 0;}
void community_fixture_replace(){replaced=true;}void community_fixture_unsupported(){unsupported=true;}
}
