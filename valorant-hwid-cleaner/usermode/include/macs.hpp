// macs.hpp — enumerate adapters, write NetworkAddress, restart binding, verify.
#pragma once
#include "common.hpp"

namespace jc::macs {

struct nic {
    std::wstring instance_id;   // NetCfgInstanceId GUID
    std::wstring driver_subkey; // {4d36e972-...}\0000 etc under HKLM...Class
    std::string  friendly_name;
    std::string  mac_registry;  // last written
    std::string  mac_runtime;   // GetAdaptersAddresses
    bool         physical{};
    bool         wireless{};
    bool         bluetooth{};
};

std::vector<nic> enumerate();

// generates plausible per-vendor MAC per NIC, writes HKLM\SYSTEM\...\{4d36e972..}\NNNN\NetworkAddress,
// then restarts the adapter via netsh / SetupDiSetClassInstallParams DI_PROPCHANGE.
result spoof_all(std::vector<std::pair<std::string,std::string>>* audit = nullptr);

std::string dump(const nic& n);

} // namespace jc::macs
