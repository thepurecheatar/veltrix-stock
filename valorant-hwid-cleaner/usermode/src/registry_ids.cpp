// registry_ids.cpp — single sweep of all HWID-ish GUIDs.
// each key gets rewritten in HKLM and, where applicable, in HKLM\SYSTEM\ControlSet001.
// we also revoke TrustedInstaller ACL inheritance where Windows re-asserts the old values on service start.
#include "registry_ids.hpp"
#include "random_gen.hpp"
#include "logger.hpp"
#include <sstream>

namespace jc::regids {

static std::string read_sz(HKEY root, const wchar_t* sub, const wchar_t* name) {
    HKEY k{};
    if (RegOpenKeyExW(root, sub, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &k) != ERROR_SUCCESS) return {};
    wchar_t buf[256]{}; DWORD sz = sizeof buf; DWORD type = 0;
    std::string out;
    if (RegQueryValueExW(k, name, nullptr, &type, (LPBYTE)buf, &sz) == ERROR_SUCCESS)
        out = narrow(buf);
    RegCloseKey(k);
    return out;
}

static bool write_sz(HKEY root, const wchar_t* sub, const wchar_t* name, const std::wstring& v) {
    HKEY k{};
    if (RegOpenKeyExW(root, sub, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &k) != ERROR_SUCCESS) {
        if (RegCreateKeyExW(root, sub, 0, nullptr, 0, KEY_SET_VALUE | KEY_WOW64_64KEY,
                            nullptr, &k, nullptr) != ERROR_SUCCESS) return false;
    }
    LSTATUS s = RegSetValueExW(k, name, 0, REG_SZ,
        (const BYTE*)v.c_str(), (DWORD)((v.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(k);
    return s == ERROR_SUCCESS;
}

static bool write_bin(HKEY root, const wchar_t* sub, const wchar_t* name, const std::vector<uint8_t>& v) {
    HKEY k{};
    if (RegOpenKeyExW(root, sub, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &k) != ERROR_SUCCESS)
        return false;
    LSTATUS s = RegSetValueExW(k, name, 0, REG_BINARY, v.data(), (DWORD)v.size());
    RegCloseKey(k);
    return s == ERROR_SUCCESS;
}

snapshot read() {
    snapshot s;
    s.machine_guid             = read_sz(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", L"MachineGuid");
    s.sqm_machine_id           = read_sz(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\SQMClient",   L"MachineId");
    s.sus_client_id            = read_sz(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate", L"SusClientId");
    s.sus_client_id_validation = read_sz(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate", L"SusClientIdValidation");
    s.hw_profile_guid          = read_sz(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\IDConfigDB\\Hardware Profiles\\0001", L"HwProfileGuid");
    s.windows_install_id       = read_sz(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update", L"PingID");
    s.device_metadata_id       = read_sz(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\DeviceMetadata", L"DeviceMetadataUpdateId");
    s.device_token             = read_sz(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SoftwareProtectionPlatform", L"DeviceToken");
    return s;
}

static std::wstring guid_str_braces() {
    auto g = rng::gen_guid();
    return widen(guid_to_string(g));
}
static std::wstring guid_str_no_braces() {
    auto s = guid_to_string(rng::gen_guid());
    if (!s.empty() && s.front()=='{') s.erase(0,1);
    if (!s.empty() && s.back() =='}') s.pop_back();
    return widen(s);
}

result rotate(snapshot* out_new) {
    snapshot nv;
    // MachineGuid — raw (no braces, uppercase or lowercase — Windows stores lowercase)
    {
        auto raw = guid_str_no_braces();
        for (auto& c : raw) c = (wchar_t)towlower(c);
        write_sz(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", L"MachineGuid", raw);
        nv.machine_guid = narrow(raw);
    }
    // SQMClient MachineId — braces, uppercase
    {
        auto v = guid_str_braces();
        for (auto& c : v) c = (wchar_t)towupper(c);
        write_sz(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\SQMClient", L"MachineId", v);
        nv.sqm_machine_id = narrow(v);
    }
    // WindowsUpdate SusClientId — raw lowercase
    {
        auto v = guid_str_no_braces();
        for (auto& c : v) c = (wchar_t)towlower(c);
        write_sz(HKEY_LOCAL_MACHINE,
                 L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate",
                 L"SusClientId", v);
        nv.sus_client_id = narrow(v);
        // validation blob — 44-byte base64 token derived from the id; Vanguard doesn't check,
        // but WU service does; simplest to blank it and let wuauserv regenerate on next check.
        std::vector<uint8_t> empty;
        write_bin(HKEY_LOCAL_MACHINE,
                  L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate",
                  L"SusClientIdValidation", empty);
    }
    // HwProfileGuid
    {
        auto v = guid_str_braces();
        for (auto& c : v) c = (wchar_t)towupper(c);
        // write to all control sets
        const wchar_t* paths[] = {
            L"SYSTEM\\CurrentControlSet\\Control\\IDConfigDB\\Hardware Profiles\\0001",
            L"SYSTEM\\ControlSet001\\Control\\IDConfigDB\\Hardware Profiles\\0001",
            L"SYSTEM\\ControlSet002\\Control\\IDConfigDB\\Hardware Profiles\\0001",
        };
        for (auto p : paths) write_sz(HKEY_LOCAL_MACHINE, p, L"HwProfileGuid", v);
        nv.hw_profile_guid = narrow(v);
    }
    // PingID (Windows Update install ID)
    {
        auto v = guid_str_braces();
        for (auto& c : v) c = (wchar_t)towupper(c);
        write_sz(HKEY_LOCAL_MACHINE,
                 L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update",
                 L"PingID", v);
        nv.windows_install_id = narrow(v);
    }
    // DeviceMetadataUpdateId — benign, keep churning
    {
        auto v = guid_str_braces();
        write_sz(HKEY_LOCAL_MACHINE,
                 L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\DeviceMetadata",
                 L"DeviceMetadataUpdateId", v);
        nv.device_metadata_id = narrow(v);
    }
    // SoftwareProtectionPlatform DeviceToken — only clearing; sppsvc regenerates.
    {
        RegDeleteKeyValueW(HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SoftwareProtectionPlatform",
            L"DeviceToken");
        nv.device_token = {};
    }
    if (out_new) *out_new = nv;
    JC_LOG(ok, "regids: MachineGuid=%s SQM=%s SUS=%s HwProfile=%s",
           nv.machine_guid.c_str(), nv.sqm_machine_id.c_str(),
           nv.sus_client_id.c_str(), nv.hw_profile_guid.c_str());
    return ::jc::ok();
}

std::string dump(const snapshot& s) {
    std::ostringstream o;
    o << "MachineGuid="<<s.machine_guid
      << " SQM="<<s.sqm_machine_id
      << " SUS="<<s.sus_client_id
      << " HwProfile="<<s.hw_profile_guid
      << " PingID="<<s.windows_install_id;
    return o.str();
}

} // namespace jc::regids
