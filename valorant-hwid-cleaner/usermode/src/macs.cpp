// macs.cpp — NIC enumeration + registry MAC spoof + adapter restart.
#include "macs.hpp"
#include "random_gen.hpp"
#include "logger.hpp"
#include <iphlpapi.h>
#include <sstream>

namespace jc::macs {

static const wchar_t* k_class_net = L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}";

static std::string hex_mac(const uint8_t* p) {
    char b[24];
    _snprintf_s(b, sizeof b, _TRUNCATE, "%02X-%02X-%02X-%02X-%02X-%02X", p[0],p[1],p[2],p[3],p[4],p[5]);
    return b;
}

static std::wstring reg_read_wstr(HKEY k, const wchar_t* name) {
    DWORD type = 0, sz = 0;
    if (RegQueryValueExW(k, name, nullptr, &type, nullptr, &sz) != ERROR_SUCCESS) return {};
    if (type != REG_SZ && type != REG_EXPAND_SZ) return {};
    std::wstring v(sz / sizeof(wchar_t) + 1, 0);
    RegQueryValueExW(k, name, nullptr, &type, (LPBYTE)v.data(), &sz);
    while (!v.empty() && v.back() == 0) v.pop_back();
    return v;
}

static bool reg_write_wstr(HKEY k, const wchar_t* name, const std::wstring& v) {
    return RegSetValueExW(k, name, 0, REG_SZ, (const BYTE*)v.c_str(),
                          (DWORD)((v.size()+1) * sizeof(wchar_t))) == ERROR_SUCCESS;
}

std::vector<nic> enumerate() {
    std::vector<nic> out;
    HKEY root = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, k_class_net, 0, KEY_READ, &root) != ERROR_SUCCESS)
        return out;

    for (DWORD i = 0; ; ++i) {
        wchar_t sub[16]{};
        DWORD sublen = 16;
        if (RegEnumKeyExW(root, i, sub, &sublen, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
            break;
        if (wcslen(sub) != 4) continue; // only NNNN subkeys
        HKEY k{};
        if (RegOpenKeyExW(root, sub, 0, KEY_READ, &k) != ERROR_SUCCESS) continue;

        nic n;
        n.driver_subkey = sub;
        n.instance_id   = reg_read_wstr(k, L"NetCfgInstanceId");
        auto desc       = reg_read_wstr(k, L"DriverDesc");
        n.friendly_name = narrow(desc);
        auto char_set   = reg_read_wstr(k, L"Characteristics");
        auto bus_type   = reg_read_wstr(k, L"BusType");
        (void)char_set; (void)bus_type;
        auto net_addr   = reg_read_wstr(k, L"NetworkAddress");
        if (!net_addr.empty()) n.mac_registry = narrow(net_addr);

        // skip WAN miniports / loopback / tunnel pseudo-adapters
        std::string lo = n.friendly_name;
        for (auto& c : lo) c = (char)tolower((unsigned char)c);
        if (lo.find("wan miniport") != std::string::npos
         || lo.find("loopback")     != std::string::npos
         || lo.find("tunnel")       != std::string::npos
         || lo.find("teredo")       != std::string::npos
         || lo.find("virtual")      != std::string::npos
         || lo.find("hyper-v")      != std::string::npos
         || lo.find("vmware")       != std::string::npos
         || lo.find("vbox")         != std::string::npos) {
            RegCloseKey(k);
            continue;
        }
        n.wireless  = (lo.find("wi-fi") != std::string::npos) || (lo.find("wireless") != std::string::npos);
        n.bluetooth = (lo.find("bluetooth") != std::string::npos);
        n.physical  = !n.instance_id.empty();
        RegCloseKey(k);
        if (n.physical) out.push_back(n);
    }

    // merge runtime MACs
    ULONG sz = 32 * 1024;
    std::vector<uint8_t> buf(sz);
    if (GetAdaptersAddresses(AF_UNSPEC,
            GAA_FLAG_SKIP_MULTICAST|GAA_FLAG_SKIP_ANYCAST|GAA_FLAG_SKIP_DNS_SERVER,
            nullptr, (PIP_ADAPTER_ADDRESSES)buf.data(), &sz) == NO_ERROR) {
        for (auto a = (PIP_ADAPTER_ADDRESSES)buf.data(); a; a = a->Next) {
            if (a->PhysicalAddressLength != 6) continue;
            std::wstring aname = widen(a->AdapterName);
            for (auto& n : out) {
                if (n.instance_id == aname) {
                    n.mac_runtime = hex_mac(a->PhysicalAddress);
                    break;
                }
            }
        }
    }
    RegCloseKey(root);
    return out;
}

static result restart_adapter(const std::wstring& instance_id) {
    // Win32 SetupDi property change restarts the adapter and makes NDIS reload NetworkAddress.
    HDEVINFO dev = SetupDiGetClassDevsW(&GUID_DEVCLASS_NET, nullptr, nullptr, DIGCF_PRESENT);
    if (dev == INVALID_HANDLE_VALUE) return fail(status::api_fail, "SetupDiGetClassDevs");

    SP_DEVINFO_DATA did{sizeof did};
    for (DWORD i = 0; SetupDiEnumDeviceInfo(dev, i, &did); ++i) {
        wchar_t drv_sub[256]{};
        DWORD sz = sizeof drv_sub;
        if (!SetupDiGetDeviceRegistryPropertyW(dev, &did, SPDRP_DRIVER, nullptr,
                                               (PBYTE)drv_sub, sz, &sz)) continue;
        // compare NetCfgInstanceId by loading driver subkey from registry
        std::wstring drv_key = std::wstring(k_class_net) + L"\\" + drv_sub;
        HKEY k{};
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, drv_key.c_str(), 0, KEY_READ, &k) != ERROR_SUCCESS) continue;
        auto id = reg_read_wstr(k, L"NetCfgInstanceId");
        RegCloseKey(k);
        if (id != instance_id) continue;

        SP_PROPCHANGE_PARAMS pcp{{sizeof(SP_CLASSINSTALL_HEADER), DIF_PROPERTYCHANGE}};
        pcp.StateChange = DICS_DISABLE;
        pcp.Scope       = DICS_FLAG_GLOBAL;
        SetupDiSetClassInstallParamsW(dev, &did, &pcp.ClassInstallHeader, sizeof pcp);
        SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, dev, &did);

        pcp.StateChange = DICS_ENABLE;
        SetupDiSetClassInstallParamsW(dev, &did, &pcp.ClassInstallHeader, sizeof pcp);
        SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, dev, &did);

        SetupDiDestroyDeviceInfoList(dev);
        return ok();
    }
    SetupDiDestroyDeviceInfoList(dev);
    return fail(status::partial, "adapter not found by instance id");
}

result spoof_all(std::vector<std::pair<std::string,std::string>>* audit) {
    auto nics = enumerate();
    if (nics.empty()) return fail(status::partial, "no physical NICs found");
    size_t ok_count = 0;
    for (auto& n : nics) {
        std::wstring drv_key = std::wstring(k_class_net) + L"\\" + n.driver_subkey;
        HKEY k{};
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, drv_key.c_str(), 0, KEY_SET_VALUE, &k) != ERROR_SUCCESS) {
            JC_LOG(warn, "mac: cannot open %ls", drv_key.c_str());
            continue;
        }
        std::string new_mac = rng::gen_mac_plausible();
        // NDIS honors "NetworkAddress" as 12 hex chars, no separators.
        std::wstring wmac = widen(new_mac);
        if (!reg_write_wstr(k, L"NetworkAddress", wmac)) {
            JC_LOG(warn, "mac: write failed for %s", n.friendly_name.c_str());
            RegCloseKey(k);
            continue;
        }
        RegCloseKey(k);
        auto r = restart_adapter(n.instance_id);
        if (r) {
            if (audit) audit->push_back({ n.mac_runtime, new_mac });
            JC_LOG(ok, "mac: %s %s -> %s",
                   n.friendly_name.c_str(), n.mac_runtime.c_str(), new_mac.c_str());
            ++ok_count;
        } else {
            JC_LOG(warn, "mac: %s set but restart failed: %s",
                   n.friendly_name.c_str(), r.what.c_str());
        }
    }
    if (!ok_count) return fail(status::partial, "no NICs got new MAC");
    return ok();
}

std::string dump(const nic& n) {
    std::ostringstream o;
    o << "nic [" << n.friendly_name << "] runtime=" << n.mac_runtime
      << " reg=" << n.mac_registry
      << (n.wireless  ? " wifi" : "")
      << (n.bluetooth ? " bt"   : "");
    return o.str();
}

} // namespace jc::macs
