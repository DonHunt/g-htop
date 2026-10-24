#include "gpu_summary.hpp"
#include "gpu_history.hpp"
#include "dashboard.hpp"
#include "gpu_fixture.hpp"
#include <gtkmm.h>
#include <iostream>
#include <atomic>
#include <mutex>
int main(int argc,char** argv){assert(argc>=3);const std::string mode=argv[1];const fs::path evidence=argv[2];fs::create_directories(evidence);auto app=Gtk::Application::create("org.ghtop.Community.GpuTest",Gio::APPLICATION_NON_UNIQUE);GpuFixture fixture;auto p=fixture.add("0000:03:00.0","0x1002",mode=="wx3200"?"0x6981":"0x7341","amdgpu");put(p/"gpu_busy_percent","0");put(p/"mem_info_vram_total",mode=="wx3200"?"4294967296":"8589934592");put(p/"mem_info_vram_used","268435456");put(p/"hwmon/hwmon0/temp1_input","40000");std::mutex mutex;std::string expected=mode=="wx3200"?"WX 3200":"W5500";const community::gpu::Nvml* provider=nullptr;std::unique_ptr<community::gpu::Nvml> nvml;
 if(mode=="nvidia"){assert(argc>=4);fs::remove_all(fixture.drm);fs::create_directories(fixture.drm);fs::remove_all(fixture.pci);fs::create_directories(fixture.pci);fixture.add("0000:ab:07.0","0x10de","0x1c81","nvidia");nvml=std::make_unique<community::gpu::Nvml>(argv[3]);provider=nvml.get();expected="GTX 1050";}
 if(mode=="intel"){put(p/"vendor","0x8086");put(p/"device","0x5912");fs::remove(p/"gpu_busy_percent");fs::remove(p/"mem_info_vram_total");fs::remove(p/"mem_info_vram_used");expected="Intel";}
 if(mode=="missing"){fs::remove_all(p);expected="N/A";}
 const bool live=mode=="live";if(live){assert(argc>=4);expected=argv[3];}
 GpuSummary summary([&]{std::lock_guard<std::mutex> guard(mutex);auto devices=live?discover_gpus():fixture.devices();if(live){devices.erase(std::remove_if(devices.begin(),devices.end(),[&](const auto& d){return d.pci_id!=expected;}),devices.end());assert(devices.size()==1);}if(provider)provider->augment(devices);return devices;},provider);GpuHistory history;Gtk::Window window;Gtk::Box box(Gtk::ORIENTATION_VERTICAL,10);Gtk::Label caption(live?"LIVE HARDWARE — single-device view on the current dual-AMD host":"SYNTHETIC FIXTURE — not a physical hardware claim");window.set_title("Community GPU acceptance: "+mode);window.set_default_size(1000,520);window.add(box);box.pack_start(caption,Gtk::PACK_SHRINK);box.pack_start(summary,Gtk::PACK_SHRINK);box.pack_start(history);window.show_all_children();bool done=false,replaced=false;unsigned samples=0;
 // Keep the exact pre-existing layout format and unrelated contents across replacement.
 const auto layout=fs::path(g_get_user_config_dir())/"G_htop/layout";put(layout,"1000 700 0.44 0.33 0.66");const auto original=community::gpu::line(layout);Dashboard dashboard;
 auto timeout=Glib::signal_timeout().connect([&]{std::cerr<<"FAIL GTK sample deadline "<<mode<<"\n";window.hide();return false;},12000);
 summary.signal_sample().connect([&](const std::vector<GpuReading>& rows){++samples;history.append_sample(rows);auto text=summary.get_text().raw();if(mode=="missing")assert(rows.empty());else assert(rows.size()==1);
  if(live){assert(rows[0].detail.find(expected)!=std::string::npos);std::ofstream(evidence/("live-"+expected+".txt"))<<text<<"\n"<<rows[0].detail<<"\n";}
  else assert(text.find(expected)!=std::string::npos);
  if(mode=="intel")assert(rows[0].usage==-1&&text.find("VRAM Used: N/A")!=std::string::npos);
  if(mode=="replacement"&&!replaced){replaced=true;std::lock_guard<std::mutex> guard(mutex);put(p/"vendor","0x8086");put(p/"device","0x5912");fs::remove(p/"driver");fs::create_directories(fixture.root/"drivers/i915");fs::create_directory_symlink(fixture.root/"drivers/i915",p/"driver");expected="Intel";return;}
  if(mode=="replacement"){assert(samples>=2&&text.find("W5500")==std::string::npos&&history.sample_count()==1);assert(community::gpu::line(layout)==original);}
  if(done)return;done=true;Glib::signal_timeout().connect_once([&]{if(auto surface=window.get_window())Gdk::Pixbuf::create(surface,0,0,window.get_allocated_width(),window.get_allocated_height())->save((evidence/(mode+(live?"-"+expected:"")+".png")).string(),"png");window.hide();},300);
 });app->run(window);timeout.disconnect();assert(done);assert(community::gpu::line(layout)==original);std::cout<<"PASS GTK "<<mode<<" samples="<<samples<<"\n";
}
