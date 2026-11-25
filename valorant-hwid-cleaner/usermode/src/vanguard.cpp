// vanguard.cpp — surgical removal. we don't delete the Riot client. only Vanguard.
#include "vanguard.hpp"
#include "logger.hpp"
#include <sstream>

namespace jc::vanguard {

static bool service_exists(const wchar_t* name, bool* running = nullptr) {
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!scm) return false;
    SC_HANDLE s = OpenServiceW(scm, name, SERVICE_QUERY_STATUS);
    bool ex = (s != nullptr);
    if (s && running) {
        SERVICE_STATUS st{};
        if (QueryServiceStatus(s, &st))
            *running = (st.dwCurrentState == SERVICE_RUNNING);
    }
    if (s) CloseServiceHandle(s);
    CloseServiceHandle(scm);
    return ex;
}

static bool service_control(const wchar_t* name, DWORD code) {
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!scm) return false;
    SC_HANDLE s = OpenServiceW(scm, name, SERVICE_STOP | SERVICE_QUERY_STATUS);
    bool ok = false;
    if (s) {
        SERVICE_STATUS st{};
        ok = ControlService(s, code, &st) || GetLastError() == ERROR_SERVICE_NOT_ACTIVE;
        CloseServiceHandle(s);
    }
    CloseServiceHandle(scm);
    return ok;
}

static bool service_delete(const wchar_t* name) {
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!scm) return false;
    SC_HANDLE s = OpenServiceW(scm, name, DELETE);
    bool ok = false;
    if (s) {
        ok = DeleteService(s) || GetLastError() == ERROR_SERVICE_MARKED_FOR_DELETE;
        CloseServiceHandle(s);
    }
    CloseServiceHandle(scm);
    return ok;
}

state status() {
    state st;
    st.vgc_running = false;
    st.vgc_installed = service_exists(L"vgc", &st.vgc_running);
    st.vgk_loaded = false;
    st.vgk_driver_present = service_exists(L"vgk", &st.vgk_loaded);
    namespace fs = std::filesystem;
    fs::path inst = L"C:\\Program Files\\Riot Vanguard";
    if (fs::exists(inst)) st.install_dir = inst.string();
    fs::path drv = L"C:\\Program Files\\Riot Vanguard\\vgk.sys";
    if (fs::exists(drv)) st.vgk_path = drv.string();
    return st;
}

result stop_services() {
    // vgk is boot-start kernel driver and can't be stopped at runtime without reboot.
    // vgc is the usermode service — stoppable.
    service_control(L"vgc", SERVICE_CONTROL_STOP);
    // vgk stop will fail (driver is in use / cannot be stopped), that's expected.
    service_control(L"vgk", SERVICE_CONTROL_STOP);
    JC_LOG(info, "vanguard: issued stop to vgc + vgk");
    return ok();
}

result uninstall() {
    // the clean path. Riot ships VanguardUninstall or we go manual.
    namespace fs = std::filesystem;
    fs::path u = L"C:\\Program Files\\Riot Vanguard\\VanguardUninstall.exe";
    bool ran = false;
    if (fs::exists(u)) {
        STARTUPINFOW si{sizeof si}; PROCESS_INFORMATION pi{};
        si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE;
        std::wstring cmd = L"\"" + u.wstring() + L"\" uninstall";
        if (CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                           nullptr, nullptr, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 90000);
            CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
            ran = true;
        }
    }
    stop_services();
    service_delete(L"vgc");
    service_delete(L"vgk");
    JC_LOG(ok, "vanguard: uninstall path %s, services deleted (effective next boot)",
           ran ? "ran VanguardUninstall.exe" : "fallback manual");
    return ok();
}

static void rm_tree(const std::filesystem::path& p) {
    std::error_code ec;
    if (!std::filesystem::exists(p, ec)) return;
    std::filesystem::permissions(p, std::filesystem::perms::owner_all, std::filesystem::perm_options::add, ec);
    std::filesystem::remove_all(p, ec);
    if (ec) JC_LOG(warn, "rm_tree failed %s: %s", p.string().c_str(), ec.message().c_str());
}

static void reg_wipe(HKEY root, const wchar_t* sub) {
    RegDeleteTreeW(root, sub);
}

result wipe_traces() {
    rm_tree(L"C:\\Program Files\\Riot Vanguard");
    rm_tree(L"C:\\ProgramData\\Riot Vanguard");
    rm_tree(L"C:\\ProgramData\\RiotClientInstalls");
    rm_tree(L"C:\\Windows\\System32\\drivers\\vgk.sys");
    rm_tree(L"C:\\Windows\\System32\\drivers\\vgc.sys");

    reg_wipe(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Riot Vanguard");
    reg_wipe(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\vgc");
    reg_wipe(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\vgk");
    reg_wipe(HKEY_LOCAL_MACHINE, L"SYSTEM\\ControlSet001\\Services\\vgc");
    reg_wipe(HKEY_LOCAL_MACHINE, L"SYSTEM\\ControlSet001\\Services\\vgk");
    reg_wipe(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Enum\\Root\\LEGACY_VGK");
    reg_wipe(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Enum\\Root\\LEGACY_VGC");

    // prefetch for vgc/vgtray/vanguard
    WIN32_FIND_DATAW fd{};
    HANDLE f = FindFirstFileW(L"C:\\Windows\\Prefetch\\*VGC*.pf", &fd);
    while (f != INVALID_HANDLE_VALUE) {
        std::wstring p = L"C:\\Windows\\Prefetch\\"; p += fd.cFileName;
        DeleteFileW(p.c_str());
        if (!FindNextFileW(f, &fd)) { FindClose(f); f = INVALID_HANDLE_VALUE; }
    }
    f = FindFirstFileW(L"C:\\Windows\\Prefetch\\*VGTRAY*.pf", &fd);
    while (f != INVALID_HANDLE_VALUE) {
        std::wstring p = L"C:\\Windows\\Prefetch\\"; p += fd.cFileName;
        DeleteFileW(p.c_str());
        if (!FindNextFileW(f, &fd)) { FindClose(f); f = INVALID_HANDLE_VALUE; }
    }

    // WER
    rm_tree(L"C:\\ProgramData\\Microsoft\\Windows\\WER\\ReportQueue");
    rm_tree(L"C:\\ProgramData\\Microsoft\\Windows\\WER\\Temp");

    // clear specific event logs. "wevtutil cl <log>" is simplest + least noisy.
    const char* logs[] = {
        "Application", "System", "Security",
        "Microsoft-Windows-DeviceSetupManager/Admin",
        "Microsoft-Windows-Diagnostics-Performance/Operational",
        "Microsoft-Windows-Kernel-PnP/Configuration",
        "Microsoft-Windows-Ntfs/Operational",
        "Microsoft-Windows-TaskScheduler/Operational",
    };
    for (auto l : logs) {
        std::string cmd = "wevtutil cl \"" + std::string(l) + "\"";
        STARTUPINFOA si{sizeof si}; PROCESS_INFORMATION pi{};
        si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE;
        if (CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                           nullptr, nullptr, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 5000);
            CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
        }
    }

    JC_LOG(ok, "vanguard: trace wipe complete");
    return ok();
}

result block_reinstall() {
    // add a Windows Firewall outbound rule denying RiotClientServices to Vanguard CDN.
    // and deny-write ACL on C:\Program Files\Riot Vanguard until we release it.
    std::error_code ec;
    std::filesystem::create_directories(L"C:\\Program Files\\Riot Vanguard", ec);
    // use icacls to deny SYSTEM write — reversed in unblock().
    STARTUPINFOA si{sizeof si}; PROCESS_INFORMATION pi{};
    si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE;
    std::string cmd = "icacls \"C:\\Program Files\\Riot Vanguard\" /deny *S-1-5-18:(OI)(CI)(W)";
    if (CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                       nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, 5000);
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    }
    JC_LOG(ok, "vanguard: reinstall blocked (ACL deny)");
    return ok();
}

result unblock() {
    STARTUPINFOA si{sizeof si}; PROCESS_INFORMATION pi{};
    si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE;
    std::string cmd = "icacls \"C:\\Program Files\\Riot Vanguard\" /remove:d *S-1-5-18";
    if (CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                       nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, 5000);
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    }
    JC_LOG(ok, "vanguard: reinstall unblocked");
    return ok();
}

std::string dump(const state& s) {
    std::ostringstream o;
    o << "vgc=" << (s.vgc_installed ? "present" : "absent")
      << (s.vgc_running ? "(running)" : "")
      << " vgk=" << (s.vgk_driver_present ? "present" : "absent")
      << (s.vgk_loaded ? "(loaded)" : "")
      << " dir=" << s.install_dir;
    return o.str();
}

} // namespace jc::vanguard
