// smbios.hpp — read + spoof SMBIOS. C++17, Win11 x64.
// reads via GetSystemFirmwareTable('RSMB'), parses DMTF 3.4 structures,
// writes via AMIDEWinx64.exe or H2OSDE64.exe (Insyde) depending on vendor.
#pragma once
#include "common.hpp"

namespace jc::smbios {

struct snapshot {
    std::string bios_vendor;
    std::string bios_version;
    std::string bios_release;
    std::string system_manufacturer;
    std::string system_product;
    std::string system_serial;
    std::string system_uuid;
    std::string baseboard_manufacturer;
    std::string baseboard_product;
    std::string baseboard_serial;
    std::string chassis_serial;
    std::string chassis_asset_tag;
};

enum class oem { unknown, dell, hp, lenovo, asus, msi, gigabyte, asrock, insyde_laptop };

struct plan {
    std::string system_serial;
    std::string system_uuid;
    std::string baseboard_serial;
    std::string chassis_serial;
    std::string asset_tag;
    oem vendor{oem::unknown};
};

snapshot read();                  // GetSystemFirmwareTable + structure walk
oem      detect_vendor(const snapshot& s);
plan     make_plan(const snapshot& s);
result   apply(const plan& p);    // dispatches to amidewin / h2osde / insyde wrapper
result   verify(const plan& p);   // re-read, assert fields changed
std::filesystem::path find_vendor_tool(oem v);

// low-level helpers used by other modules (volume serial hashing etc)
std::vector<uint8_t> raw_smbios_table();
std::string dump(const snapshot& s);

} // namespace jc::smbios
