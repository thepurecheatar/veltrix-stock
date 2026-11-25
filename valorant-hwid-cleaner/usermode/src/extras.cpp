// extras.cpp — the second-pass sweep. kills every cache the first pass didn't move.
#include "extras.hpp"
#include "logger.hpp"
#include "random_gen.hpp"
#include <sstream>

namespace jc::extras {

static void rm_tree(const std::filesystem::path& p) {
    std::error_code ec;
    std::filesystem::permissions(p, std::filesystem::perms::owner_all, std::filesystem::perm_options::add, ec);
    std::filesystem::remove_all(p, ec);
}

result wipe_bluetooth_bd_addr() {
    // BTHPORT caches the controller's BD_ADDR under each device key and under "Keys".
    // We rewrite only the controller keys, letting paired-device keys die with them (user re-pairs).
    HKEY k{};
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\BTHPORT\\Parameters\\Keys",
        0, KEY_READ, &k) != ERROR_SUCCESS) {
        return fail(status::partial, "no Bluetooth stack present");
    }
    for (DWORD i = 0; ; ++i) {
        wchar_t sub[64]{}; DWORD sl = 64;
        if (RegEnumKeyExW(k, i, sub, &sl, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
        // delete the whole per-controller subtree; Windows re-enumerates on boot.
        std::wstring path = std::wstring(L"SYSTEM\\CurrentControlSet\\Services\\BTHPORT\\Parameters\\Keys\\") + sub;
        RegDeleteTreeW(HKEY_LOCAL_MACHINE, path.c_str());
    }
    RegCloseKey(k);
    JC_LOG(ok, "bt: BTHPORT keys dropped");
    return ok();
}

result wipe_wmi_cim_cache() {
    // Winmgmt caches hardware strings in C:\Windows\System32\wbem\Repository.
    // Nuke it with the service stopped; it rebuilds lazily. Vanguard uses WMI for some reads.
    SERVICE_STATUS st{};
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (scm) {
        SC_HANDLE s = OpenServiceW(scm, L"winmgmt", SERVICE_ALL_ACCESS);
        if (s) {
            ControlService(s, SERVICE_CONTROL_STOP, &st);
            CloseServiceHandle(s);
        }
        CloseServiceHandle(scm);
    }
    rm_tree(L"C:\\Windows\\System32\\wbem\\Repository");
    // start it back up — winmgmt.exe /resetrepository would also work but is slower.
    scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (scm) {
        SC_HANDLE s = OpenServiceW(scm, L"winmgmt", SERVICE_START);
        if (s) { StartServiceW(s, 0, nullptr); CloseServiceHandle(s); }
        CloseServiceHandle(scm);
    }
    JC_LOG(ok, "wmi: CIM repository rebuilt");
    return ok();
}

result wipe_bam_dam_history() {
    // BAM/DAM tracks process usage per-SID and links to HWID. Not a fingerprint, but a correlator.
    const wchar_t* paths[] = {
        L"SYSTEM\\CurrentControlSet\\Services\\bam\\State\\UserSettings",
        L"SYSTEM\\CurrentControlSet\\Services\\dam\\State\\UserSettings",
    };
    for (auto p : paths) {
        HKEY k{};
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, p, 0, KEY_READ, &k) != ERROR_SUCCESS) continue;
        for (DWORD i = 0; ; ++i) {
            wchar_t sub[128]{}; DWORD sl = 128;
            if (RegEnumKeyExW(k, i, sub, &sl, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
            std::wstring full = std::wstring(p) + L"\\" + sub;
            RegDeleteTreeW(HKEY_LOCAL_MACHINE, full.c_str());
        }
        RegCloseKey(k);
    }
    JC_LOG(ok, "bam/dam history cleared");
    return ok();
}

result wipe_riot_client_remnants() {
    wchar_t appdata[MAX_PATH]{}; GetEnvironmentVariableW(L"LOCALAPPDATA", appdata, MAX_PATH);
    if (appdata[0]) {
        rm_tree(std::wstring(appdata) + L"\\Riot Games");
    }
    rm_tree(L"C:\\ProgramData\\Riot Games");
    rm_tree(L"C:\\Riot Games");  // if user installed to root
    // Riot client config carries a machineId hash; nuke it.
    JC_LOG(ok, "riot client remnants cleared (you'll re-login fresh)");
    return ok();
}

result rotate_computer_name() {
    // only do if requested — breaks a lot of local tooling.
    std::wstring nn = L"DESKTOP-";
    for (int i = 0; i < 7; ++i) nn += (wchar_t)('A' + (rng::u8() % 26));
    if (!SetComputerNameExW(ComputerNamePhysicalDnsHostname, nn.c_str()))
        return fail(status::api_fail, "SetComputerNameEx failed");
    if (!SetComputerNameExW(ComputerNameDnsHostname, nn.c_str()))
        return fail(status::partial, "DnsHostname did not update");
    JC_LOG(ok, "computer name -> %ls (effective next reboot)", nn.c_str());
    return ok();
}

result wipe_system_vault_device_id() {
    RegDeleteKeyValueW(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\MoSetup",
        L"DeviceVaultData");
    RegDeleteKeyValueW(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\IdentityCRL\\Immersive\\production",
        L"DeviceId");
    RegDeleteKeyValueW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\IdentityCRL\\Immersive\\production",
        L"DeviceId");
    JC_LOG(ok, "system vault device id caches dropped");
    return ok();
}

result wipe_prefetch_global() {
    WIN32_FIND_DATAW fd{};
    HANDLE f = FindFirstFileW(L"C:\\Windows\\Prefetch\\*.pf", &fd);
    if (f == INVALID_HANDLE_VALUE) return ok();
    size_t n = 0;
    do {
        std::wstring p = L"C:\\Windows\\Prefetch\\"; p += fd.cFileName;
        if (DeleteFileW(p.c_str())) ++n;
    } while (FindNextFileW(f, &fd));
    FindClose(f);
    JC_LOG(ok, "prefetch: dropped %zu .pf files", n);
    return ok();
}

} // namespace jc::extras
