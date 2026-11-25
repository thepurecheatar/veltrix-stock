// spoofer.cpp — driver entry, control device, IOCTL dispatch.
// loaded via BYOVD manual map; DriverObject may be synthetic. we tolerate that.
#include "spoofer.hpp"

static PDEVICE_OBJECT g_ctrl_dev = nullptr;
static UNICODE_STRING g_dev_name   = RTL_CONSTANT_STRING(L"\\Device\\JailCrashSpoof");
static UNICODE_STRING g_link_name  = RTL_CONSTANT_STRING(L"\\DosDevices\\JailCrashSpoof");

static disk_lie   g_disk_lies[16];
static ULONG      g_disk_lie_count = 0;
static smbios_lie g_smbios_lie{};
static BOOLEAN    g_smbios_lie_set = FALSE;
static mac_lie    g_mac_lie{};
static BOOLEAN    g_mac_lie_set = FALSE;
static FAST_MUTEX g_lock;

extern "C" NTSTATUS DriverIrpDefault(PDEVICE_OBJECT, PIRP irp) {
    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = 0;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

extern "C" NTSTATUS DriverIrpControl(PDEVICE_OBJECT, PIRP irp) {
    PIO_STACK_LOCATION sl = IoGetCurrentIrpStackLocation(irp);
    NTSTATUS st = STATUS_INVALID_PARAMETER;
    ULONG written = 0;
    PVOID buf = irp->AssociatedIrp.SystemBuffer;
    ULONG inlen = sl->Parameters.DeviceIoControl.InputBufferLength;

    ExAcquireFastMutex(&g_lock);
    switch (sl->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_JC_SET_DISK_LIE: {
            if (inlen >= sizeof(disk_lie) && g_disk_lie_count < 16) {
                RtlCopyMemory(&g_disk_lies[g_disk_lie_count++], buf, sizeof(disk_lie));
                st = STATUS_SUCCESS;
            }
            break;
        }
        case IOCTL_JC_SET_SMBIOS_LIE: {
            if (inlen >= sizeof(smbios_lie)) {
                RtlCopyMemory(&g_smbios_lie, buf, sizeof(smbios_lie));
                g_smbios_lie_set = TRUE;
                st = STATUS_SUCCESS;
            }
            break;
        }
        case IOCTL_JC_SET_MAC_LIE: {
            if (inlen >= sizeof(mac_lie)) {
                RtlCopyMemory(&g_mac_lie, buf, sizeof(mac_lie));
                g_mac_lie_set = TRUE;
                st = STATUS_SUCCESS;
            }
            break;
        }
        case IOCTL_JC_UNLOAD: {
            uninstall_all_hooks();
            st = STATUS_SUCCESS;
            break;
        }
    }
    ExReleaseFastMutex(&g_lock);

    irp->IoStatus.Status = st;
    irp->IoStatus.Information = written;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return st;
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT drv, PUNICODE_STRING) {
    ExInitializeFastMutex(&g_lock);
    NTSTATUS st = IoCreateDevice(drv, 0, &g_dev_name, FILE_DEVICE_UNKNOWN,
                                 FILE_DEVICE_SECURE_OPEN, FALSE, &g_ctrl_dev);
    if (!NT_SUCCESS(st)) return st;
    IoCreateSymbolicLink(&g_link_name, &g_dev_name);

    for (ULONG i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; ++i)
        drv->MajorFunction[i] = DriverIrpDefault;
    drv->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverIrpControl;

    install_storage_filter();
    install_firmware_hook();
    install_ndis_filter();
    return STATUS_SUCCESS;
}

void* jc_get_smbios_lie() { return g_smbios_lie_set ? &g_smbios_lie : nullptr; }
disk_lie* jc_find_disk_lie(ULONG index) {
    for (ULONG i = 0; i < g_disk_lie_count; ++i)
        if (g_disk_lies[i].disk_index == index) return &g_disk_lies[i];
    return nullptr;
}
void* jc_get_mac_lie() { return g_mac_lie_set ? &g_mac_lie : nullptr; }
