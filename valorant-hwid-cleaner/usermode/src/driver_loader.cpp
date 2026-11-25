// driver_loader.cpp — write the vuln driver, SCM-start it, exercise its MmMapIoSpace primitive
// to manually map our spoofer driver image into non-paged pool, patch relocations, and jump to entry.
//
// FILL: raw vuln driver blobs go in third_party\\iqvw64e.sys etc; we just read them at runtime,
// nothing is baked into the exe.
#include "driver_loader.hpp"
#include "logger.hpp"
#include "random_gen.hpp"
#include <sstream>

namespace jc::byovd {

static bool enable_priv(const wchar_t* name) {
    HANDLE t{};
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &t)) return false;
    LUID luid{};
    if (!LookupPrivilegeValueW(nullptr, name, &luid)) { CloseHandle(t); return false; }
    TOKEN_PRIVILEGES tp{1, {{luid, SE_PRIVILEGE_ENABLED}}};
    bool ok = AdjustTokenPrivileges(t, FALSE, &tp, sizeof tp, nullptr, nullptr)
           && GetLastError() == ERROR_SUCCESS;
    CloseHandle(t);
    return ok;
}

static std::wstring rand_service_name() {
    std::wstring s = L"svc";
    for (int i = 0; i < 8; ++i) s += (wchar_t)('a' + (rng::u8() % 26));
    return s;
}

static std::filesystem::path src_for(vuln v) {
    namespace fs = std::filesystem;
    fs::path base = fs::current_path() / "tools";
    switch (v) {
        case vuln::iqvw64e_intel: return base / "iqvw64e.sys";
        case vuln::rtcore64_micsai: return base / "RTCore64.sys";
        case vuln::dbutil_dell:   return base / "DBUtil_2_3.sys";
        case vuln::gdrv_gigabyte: return base / "gdrv.sys";
        case vuln::capcom:        return base / "Capcom.sys";
    }
    return {};
}

static std::wstring sys_for(vuln v) {
    switch (v) {
        case vuln::iqvw64e_intel: return L"Nal";
        case vuln::rtcore64_micsai: return L"RTCore64";
        case vuln::dbutil_dell:   return L"DBUtil_2_3";
        case vuln::gdrv_gigabyte: return L"GIO";
        case vuln::capcom:        return L"Htsysm72FB";
    }
    return {};
}

result load_vuln(vuln v, std::wstring* out) {
    if (!enable_priv(SE_LOAD_DRIVER_NAME))
        return fail(status::denied_privilege, "SeLoadDriverPrivilege denied — need admin");

    auto src = src_for(v);
    if (!std::filesystem::exists(src))
        return fail(status::driver_fail, "missing vuln driver file: " + src.string() +
                   " — drop it in tools/, see README");

    wchar_t tmp[MAX_PATH]{}; GetTempPathW(MAX_PATH, tmp);
    std::wstring dst = std::wstring(tmp) + rand_service_name() + L".sys";
    if (!CopyFileW(src.wstring().c_str(), dst.c_str(), FALSE))
        return fail(status::driver_fail, "CopyFile to temp failed");

    std::wstring svc = rand_service_name();
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (!scm) return fail(status::driver_fail, "OpenSCManager");
    SC_HANDLE s = CreateServiceW(scm, svc.c_str(), svc.c_str(),
        SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
        dst.c_str(), nullptr, nullptr, nullptr, nullptr, nullptr);
    if (!s) {
        if (GetLastError() == ERROR_SERVICE_EXISTS) {
            s = OpenServiceW(scm, svc.c_str(), SERVICE_ALL_ACCESS);
        } else {
            CloseServiceHandle(scm);
            DeleteFileW(dst.c_str());
            return fail(status::driver_fail, "CreateService: " + std::to_string(GetLastError()));
        }
    }
    bool started = StartServiceW(s, 0, nullptr) ||
                   GetLastError() == ERROR_SERVICE_ALREADY_RUNNING;
    if (!started) {
        DWORD e = GetLastError();
        DeleteService(s);
        CloseServiceHandle(s);
        CloseServiceHandle(scm);
        DeleteFileW(dst.c_str());
        if (e == ERROR_INVALID_IMAGE_HASH)
            return fail(status::driver_fail, "driver cert revoked by MS driver blocklist — pick a different BYOVD target");
        return fail(status::driver_fail, "StartService: " + std::to_string(e));
    }
    CloseServiceHandle(s);
    CloseServiceHandle(scm);
    if (out) *out = svc;
    JC_LOG(ok, "byovd: loaded %ls as service %ls", sys_for(v).c_str(), svc.c_str());
    return ok();
}

result unload_vuln(const std::wstring& svc) {
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!scm) return fail(status::api_fail, "OpenSCManager");
    SC_HANDLE s = OpenServiceW(scm, svc.c_str(), SERVICE_ALL_ACCESS);
    if (s) {
        SERVICE_STATUS st{};
        ControlService(s, SERVICE_CONTROL_STOP, &st);
        DeleteService(s);
        CloseServiceHandle(s);
    }
    CloseServiceHandle(scm);
    JC_LOG(ok, "byovd: unloaded %ls", svc.c_str());
    return ok();
}

capability probe_capability(vuln v) {
    capability c; c.which = v;
    switch (v) {
        case vuln::iqvw64e_intel:
            c.mm_map_io_space = true; c.arbitrary_phys_rw = true; break;
        case vuln::rtcore64_micsai:
            c.arbitrary_phys_rw = true; c.pte_leak = true; break;
        case vuln::dbutil_dell:
            c.arbitrary_phys_rw = true; c.mdl_write = true; break;
        case vuln::gdrv_gigabyte:
            c.mm_map_io_space = true; break;
        case vuln::capcom:
            c.mm_map_io_space = true; c.arbitrary_phys_rw = true; c.mdl_write = true; break;
    }
    return c;
}

// --- PE manual map helpers ---
// deliberately leaving the heavy loader logic in a dedicated TU that includes the vuln-specific
// IOCTL code tables; here we expose the shape only. The real kernel work is in src/manual_map.cpp.
extern result manual_map_via_iqvw64e(const std::filesystem::path& payload, mapped_driver* out);

result map_our_driver(vuln via, const std::filesystem::path& payload, mapped_driver* out) {
    if (!std::filesystem::exists(payload))
        return fail(status::driver_fail, "spoofer payload not found: " + payload.string());
    if (via == vuln::iqvw64e_intel) return manual_map_via_iqvw64e(payload, out);
    return fail(status::driver_fail, "selected BYOVD not wired for manual map yet");
}

result unmap(mapped_driver& m) {
    if (m.control_device) { CloseHandle(m.control_device); m.control_device = nullptr; }
    // kernel-side cleanup is triggered via IOCTL_JC_UNLOAD before we drop the vuln driver.
    m = {};
    return ok();
}

HANDLE open_control() {
    return CreateFileW(L"\\\\.\\JailCrashSpoof", GENERIC_READ|GENERIC_WRITE,
        0, nullptr, OPEN_EXISTING, 0, nullptr);
}

} // namespace jc::byovd
