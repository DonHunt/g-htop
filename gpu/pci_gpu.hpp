#pragma once

#include "linux_gpu.hpp"
#include <filesystem>
#include <regex>

namespace community::gpu {

static const std::regex pattern(R"([0-9a-fA-F]{4}:[0-9a-fA-F]{2}:[01][0-9a-fA-F]\.[0-7])");

inline std::vector<Device> discover_pci(const std::filesystem::path& root = "/sys/bus/pci/devices") {
    std::vector<Device> devices;
    std::error_code ec;
    std::filesystem::directory_iterator at(root,ec),end;
    for (; !ec && at != end; at.increment(ec)) {
        const auto path = at->path();
        const auto id = path.filename().string();
        if (!std::regex_match(id, pattern)) continue;
        auto cls = line(path / "class");
        std::transform(cls.begin(),cls.end(),cls.begin(),[](unsigned char c){return char(std::tolower(c));});
        if (cls.compare(0, 4, "0x03") != 0) continue;
        Device d;
        d.card = "pci-" + id;
        d.path = path.string();
        d.pci_id = id;
        d.vendor = line(path / "vendor");
        std::error_code driver_error;
        auto target = std::filesystem::read_symlink(path / "driver", driver_error);
        if (!driver_error) d.driver = target.filename().string();
        d.name = line(path / "product_name");
        if (d.name.empty()) {
            const auto vendor = d.vendor;
            d.name = vendor == "0x1002" ? "AMD (" + id + ")" :
                      vendor == "0x10de" ? "NVIDIA (" + id + ")" :
                      vendor == "0x8086" ? "Intel (" + id + ")" :
                      "Display controller (" + id + ")";
        }
        devices.push_back(d);
    }
    std::sort(devices.begin(), devices.end(), [](const Device& a, const Device& b) {
        return a.pci_id < b.pci_id;
    });
    return devices;
}

} // namespace community::gpu
