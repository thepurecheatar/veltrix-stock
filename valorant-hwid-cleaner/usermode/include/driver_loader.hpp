// driver_loader.hpp — BYOVD: load a vulnerable signed Intel driver, use its PhysicalMemory primitive
// to manually map our own unsigned spoofer driver into the kernel and start its entry thread.
#pragma once
#include "common.hpp"

namespace jc::byovd {

enum class vuln { iqvw64e_intel, rtcore64_micsai, dbutil_dell, gdrv_gigabyte, capcom };

struct capability {
    vuln       which{};
    bool       mm_map_io_space{};
    bool       arbitrary_phys_rw{};
    bool       mdl_write{};
    bool       pte_leak{};
};

// enable SeLoadDriverPrivilege, write the vuln driver to %TEMP%\<rand>.sys, create SCM service, start it.
result load_vuln(vuln v, std::wstring* out_service_name = nullptr);
result unload_vuln(const std::wstring& service_name);
capability probe_capability(vuln v);

// map our own driver (payload in a flat PE file) into the kernel via phys-mem primitive.
// returns the entry point address so we can PsCreateSystemThread it, plus service control handle.
struct mapped_driver {
    uint64_t base_va{};
    uint64_t entry_point{};
    uint32_t size{};
    HANDLE   control_device{};
};
result map_our_driver(vuln via, const std::filesystem::path& driver_sys, mapped_driver* out);
result unmap(mapped_driver& m);

// convenience: open our driver's user-facing IOCTL interface after map.
HANDLE open_control();

// IOCTL codes shared with driver/hooks.cpp
constexpr DWORD IOCTL_JC_SET_DISK_LIE     = CTL_CODE(0x8000, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS);
constexpr DWORD IOCTL_JC_SET_SMBIOS_LIE   = CTL_CODE(0x8000, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS);
constexpr DWORD IOCTL_JC_SET_MAC_LIE      = CTL_CODE(0x8000, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS);
constexpr DWORD IOCTL_JC_UNLOAD           = CTL_CODE(0x8000, 0x8FF, METHOD_BUFFERED, FILE_ANY_ACCESS);

} // namespace jc::byovd
