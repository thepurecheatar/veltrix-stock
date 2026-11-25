// firmware_vars.cpp — talks straight to UEFI RT via Get/SetFirmwareEnvironmentVariable.
#include "firmware_vars.hpp"
#include "random_gen.hpp"
#include "logger.hpp"

namespace jc::fwvar {

// EFI Global Variable GUID: {8BE4DF61-93CA-11D2-AA0D-00E098032B8C}
static const GUID EFI_GLOBAL_VAR = { 0x8BE4DF61, 0x93CA, 0x11D2,
    { 0xAA,0x0D,0x00,0xE0,0x98,0x03,0x2B,0x8C } };

result enable_privilege() {
    HANDLE t{};
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &t))
        return fail(status::api_fail, "OpenProcessToken");
    LUID luid{};
    if (!LookupPrivilegeValueW(nullptr, L"SeSystemEnvironmentPrivilege", &luid)) {
        CloseHandle(t); return fail(status::denied_privilege, "no SeSystemEnvironment");
    }
    TOKEN_PRIVILEGES tp{1, {{luid, SE_PRIVILEGE_ENABLED}}};
    AdjustTokenPrivileges(t, FALSE, &tp, sizeof tp, nullptr, nullptr);
    DWORD err = GetLastError();
    CloseHandle(t);
    if (err != ERROR_SUCCESS)
        return fail(status::denied_privilege, "AdjustToken: " + std::to_string(err));
    return ok();
}

static std::wstring guid_str(const GUID& g) {
    wchar_t b[48];
    _snwprintf_s(b, 48, _TRUNCATE,
        L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        g.Data1, g.Data2, g.Data3,
        g.Data4[0],g.Data4[1],g.Data4[2],g.Data4[3],
        g.Data4[4],g.Data4[5],g.Data4[6],g.Data4[7]);
    return b;
}

result read_var(const std::wstring& name, const GUID& ns, std::vector<uint8_t>& out) {
    out.assign(8192, 0);
    DWORD n = GetFirmwareEnvironmentVariableW(name.c_str(), guid_str(ns).c_str(), out.data(), (DWORD)out.size());
    if (!n) {
        out.clear();
        return fail(status::api_fail, "GetFirmwareEnvironmentVariable " + std::to_string(GetLastError()));
    }
    out.resize(n);
    return ok();
}

result write_var(const std::wstring& name, const GUID& ns, const std::vector<uint8_t>& v) {
    BOOL ok = SetFirmwareEnvironmentVariableW(name.c_str(), guid_str(ns).c_str(),
                                              (PVOID)v.data(), (DWORD)v.size());
    if (!ok) return fail(status::api_fail, "SetFirmwareEnvironmentVariable " + std::to_string(GetLastError()));
    return ::jc::ok();
}

result rotate_identity_vars() {
    auto priv = enable_privilege();
    if (!priv) return priv;

    // on secureboot-enabled systems we can't write most of these — that's fine, we iterate.
    // we overwrite with randomized junk of matching size.
    const wchar_t* targets[] = {
        L"PlatformLangCodes",  // short ascii
        L"KeyboardLayout",
    };
    size_t n = 0;
    for (auto t : targets) {
        std::vector<uint8_t> cur;
        auto r = read_var(t, EFI_GLOBAL_VAR, cur);
        if (!r || cur.empty()) continue;
        std::vector<uint8_t> fresh(cur.size());
        rng::fill(fresh.data(), fresh.size());
        // keep it ASCII for lang code to not brick keyboard enum
        if (std::wstring(t) == L"PlatformLangCodes") {
            for (auto& b : fresh) b = (uint8_t)('a' + (b % 26));
            if (!fresh.empty()) fresh.back() = 0;
        }
        if (write_var(t, EFI_GLOBAL_VAR, fresh)) {
            JC_LOG(ok, "uefi: var %ls rotated (%zu bytes)", t, fresh.size());
            ++n;
        }
    }
    if (!n) return fail(status::partial, "no UEFI vars rotated (SecureBoot or firmware lock)");
    return ok();
}

result enable_test_signing() {
    STARTUPINFOW si{sizeof si}; PROCESS_INFORMATION pi{};
    si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE;
    std::wstring cmd = L"bcdedit /set testsigning yes";
    if (!CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                        nullptr, nullptr, &si, &pi)) return fail(status::api_fail, "bcdedit");
    WaitForSingleObject(pi.hProcess, 10000);
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    JC_LOG(warn, "bcd: testsigning=yes — breaks HVCI, takes effect next reboot; reverse with --bcd-lock");
    return ok();
}

result disable_test_signing() {
    STARTUPINFOW si{sizeof si}; PROCESS_INFORMATION pi{};
    si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE;
    std::wstring cmd = L"bcdedit /set testsigning no";
    if (!CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                        nullptr, nullptr, &si, &pi)) return fail(status::api_fail, "bcdedit");
    WaitForSingleObject(pi.hProcess, 10000);
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    JC_LOG(ok, "bcd: testsigning=no restored");
    return ok();
}

} // namespace jc::fwvar
