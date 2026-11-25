// gpu_edid.hpp — GPU adapter LUID + monitor EDID rewrite.
#pragma once
#include "common.hpp"

namespace jc::gpu {

struct gpu_adapter {
    std::wstring device_instance_id;  // PCI\VEN_10DE&DEV_...
    std::wstring driver_subkey;       // {4d36e968..}\NNNN
    std::string  model;
    std::string  uuid_registry;
};

struct monitor {
    std::wstring device_instance_id;  // DISPLAY\XXX\5&...
    std::wstring driver_subkey;
    std::vector<uint8_t> edid;
    std::string edid_sha256;
};

std::vector<gpu_adapter> enum_gpus();
std::vector<monitor>     enum_monitors();

result rotate_monitor_edid();
result rotate_gpu_registry_ids();

std::string dump(const gpu_adapter& g);
std::string dump(const monitor& m);

} // namespace jc::gpu
