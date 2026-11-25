// spoofer.hpp — kernel-mode spoofer. WDK, Win11 x64. loaded manually via BYOVD, no PnP, no signature.
#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
#include <ntstrsafe.h>
#include <ntddstor.h>

constexpr ULONG JC_POOL_TAG = 'ScJc';

// shared IOCTLs with usermode driver_loader.hpp
#define IOCTL_JC_SET_DISK_LIE   CTL_CODE(0x8000, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_JC_SET_SMBIOS_LIE CTL_CODE(0x8000, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_JC_SET_MAC_LIE    CTL_CODE(0x8000, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_JC_UNLOAD         CTL_CODE(0x8000, 0x8FF, METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma pack(push, 1)
struct disk_lie {
    ULONG  disk_index;
    CHAR   model[64];
    CHAR   serial[64];
    CHAR   firmware_rev[16];
    CHAR   vendor[16];
};

struct smbios_lie {
    CHAR system_serial[32];
    CHAR system_uuid[48];
    CHAR baseboard_serial[32];
    CHAR chassis_serial[32];
};

struct mac_lie {
    ULONG count;
    struct { UCHAR mac[6]; UCHAR pad[2]; WCHAR instance_id[80]; } entries[16];
};
#pragma pack(pop)

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT drv, _In_ PUNICODE_STRING reg);
void     install_storage_filter();
void     install_firmware_hook();
void     install_ndis_filter();
void     uninstall_all_hooks();
