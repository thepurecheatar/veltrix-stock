// evasion.cpp — the quiet layer.
#include "evasion.hpp"
#include "logger.hpp"
#include <sstream>

namespace jc::evasion {

result disable_etw_for_self() {
    HMODULE h = GetModuleHandleW(L"ntdll");
    if (!h) return fail(status::api_fail, "no ntdll");
    auto fn = (void*)GetProcAddress(h, "EtwEventWrite");
    if (!fn) return fail(status::api_fail, "EtwEventWrite not found");
    DWORD old = 0;
    if (!VirtualProtect(fn, 1, PAGE_EXECUTE_READWRITE, &old))
        return fail(status::api_fail, "VirtualProtect");
    *(uint8_t*)fn = 0xC3; // ret
    VirtualProtect(fn, 1, old, &old);
    JC_LOG(ok, "evasion: ETW muted for current process");
    return ok();
}

result smear_timestamps(const std::filesystem::path& p) {
    WIN32_FILE_ATTRIBUTE_DATA ref{};
    if (!GetFileAttributesExW(L"C:\\Windows\\explorer.exe",
                              GetFileExInfoStandard, &ref)) {
        return fail(status::api_fail, "cannot stat explorer.exe");
    }
    handle_t h{ CreateFileW(p.c_str(), FILE_WRITE_ATTRIBUTES,
                            FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr,
                            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr) };
    if (!h.valid()) return fail(status::api_fail, "open for attrs");
    if (!SetFileTime(h.get(), &ref.ftCreationTime, &ref.ftLastAccessTime, &ref.ftLastWriteTime))
        return fail(status::api_fail, "SetFileTime");
    return ok();
}

result plant_decoy_7045(const std::wstring& fake_service_name) {
    // The cleanest decoy is a stale service entry under HKLM\SYSTEM\CurrentControlSet\Services
    // pointing to a fake imagepath, then creating + deleting the service so SCM logs a 7045.
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (!scm) return fail(status::api_fail, "OpenSCManager");
    SC_HANDLE s = CreateServiceW(scm, fake_service_name.c_str(), fake_service_name.c_str(),
        SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
        L"System32\\drivers\\null.sys", nullptr, nullptr, nullptr, nullptr, nullptr);
    if (s) { DeleteService(s); CloseServiceHandle(s); }
    CloseServiceHandle(scm);
    JC_LOG(ok, "evasion: decoy 7045 planted as %ls", fake_service_name.c_str());
    return ok();
}

bool vgk_is_loaded() {
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!scm) return false;
    SC_HANDLE s = OpenServiceW(scm, L"vgk", SERVICE_QUERY_STATUS);
    bool loaded = false;
    if (s) {
        SERVICE_STATUS st{};
        if (QueryServiceStatus(s, &st)) loaded = (st.dwCurrentState != SERVICE_STOPPED);
        CloseServiceHandle(s);
    }
    CloseServiceHandle(scm);
    return loaded;
}

void xor_buf(uint8_t* p, size_t n, uint8_t key) {
    for (size_t i = 0; i < n; ++i) p[i] ^= key;
}

} // namespace jc::evasion
