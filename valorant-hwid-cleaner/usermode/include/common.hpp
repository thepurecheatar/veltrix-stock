// common.hpp — shared types, Windows headers, status codes. C++17, MSVC, Win11 x64.
#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <winternl.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <devguid.h>
#include <winioctl.h>
#include <ntddscsi.h>
#include <tbs.h>

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <array>
#include <variant>
#include <string_view>
#include <filesystem>

#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "Cfgmgr32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Tbs.lib")
#pragma comment(lib, "Wintrust.lib")
#pragma comment(lib, "Crypt32.lib")

namespace jc {

enum class status : uint32_t {
    ok = 0,
    denied_privilege,
    not_admin,
    api_fail,
    driver_fail,
    firmware_locked,
    vanguard_running,
    partial,
    skipped,
};

struct result {
    status st{status::ok};
    std::string what;
    explicit operator bool() const noexcept { return st == status::ok; }
};

inline result ok() { return {status::ok, {}}; }
inline result fail(status s, std::string w) { return {s, std::move(w)}; }

// RAII handle. not shared — one owner.
class handle_t {
    HANDLE h_{INVALID_HANDLE_VALUE};
public:
    handle_t() = default;
    explicit handle_t(HANDLE h) noexcept : h_(h) {}
    handle_t(const handle_t&) = delete;
    handle_t& operator=(const handle_t&) = delete;
    handle_t(handle_t&& o) noexcept : h_(o.h_) { o.h_ = INVALID_HANDLE_VALUE; }
    handle_t& operator=(handle_t&& o) noexcept {
        if (this != &o) { close(); h_ = o.h_; o.h_ = INVALID_HANDLE_VALUE; }
        return *this;
    }
    ~handle_t() { close(); }
    HANDLE get() const noexcept { return h_; }
    bool valid() const noexcept { return h_ != INVALID_HANDLE_VALUE && h_ != nullptr; }
    HANDLE release() noexcept { auto t = h_; h_ = INVALID_HANDLE_VALUE; return t; }
    void close() noexcept { if (valid()) { CloseHandle(h_); } h_ = INVALID_HANDLE_VALUE; }
};

// one snapshot of the whole machine fingerprint — pre and post spoof compared.
struct machine_snapshot {
    std::string bios_vendor;
    std::string bios_version;
    std::string bios_serial;
    std::string system_serial;
    std::string system_uuid;
    std::string baseboard_serial;
    std::string chassis_serial;
    std::vector<std::string> disk_serials;
    std::vector<std::string> mac_addresses;
    std::vector<std::string> volume_serials;
    std::string machine_guid;
    std::string sqm_machine_id;
    std::string sus_client_id;
    std::string windows_installation_id;
    std::string hw_profile_guid;
    std::string tpm_ek_pub_hash;
    std::vector<std::string> gpu_uuids;
    std::vector<std::string> monitor_edid_hashes;
};

std::string to_hex(const uint8_t* p, size_t n);
std::string guid_to_string(const GUID& g);
GUID new_guid();
std::wstring widen(std::string_view s);
std::string narrow(std::wstring_view s);

} // namespace jc
