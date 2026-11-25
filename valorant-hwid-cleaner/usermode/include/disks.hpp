// disks.hpp — enumerate physical disks, read serials, change volume IDs, drive IOCTL hooks.
#pragma once
#include "common.hpp"

namespace jc::disks {

struct phys_disk {
    uint32_t    index{};
    std::string device_path;      // \\.\PhysicalDriveN
    std::string model;
    std::string serial;
    std::string vendor_id;
    std::string firmware_rev;
    uint64_t    size_bytes{};
    bool        nvme{};
    bool        ssd{};
};

struct volume_info {
    std::wstring guid_path;       // volume guid path
    std::wstring drive_letter;    // drive letter root (e.g. C:\) if assigned
    uint32_t     serial{};
    std::wstring label;
    std::wstring fs_name;         // NTFS / FAT32 / exFAT
};

std::vector<phys_disk>   enumerate_physical();
std::vector<volume_info> enumerate_volumes();

// change every writable volume's serial to a fresh value; returns the old→new mapping count.
result rotate_volume_serials(std::vector<std::pair<uint32_t,uint32_t>>* audit = nullptr);

// per-disk: send ATA/NVMe passthrough to change firmware-visible serial where supported.
// SATA SSDs on many vendors: not supported. nvme: nvme-cli style namespace reinit — risky, we skip by default.
// instead we hand the driver a hook table to lie to Vanguard's IOCTL path.
result stage_driver_hook_table(const std::vector<phys_disk>& disks);

std::string dump(const phys_disk& d);
std::string dump(const volume_info& v);

} // namespace jc::disks
