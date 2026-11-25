// hooks.cpp — three hooks:
//   1) storage stack IOCTL filter: rewrite STORAGE_DEVICE_DESCRIPTOR serial on query
//   2) NtQuerySystemInformation hook for SystemFirmwareTableInformation to rewrite SMBIOS table
//   3) NDIS filter stub for OID_802_3_PERMANENT_ADDRESS + OID_802_3_CURRENT_ADDRESS
//
// IoAttachDeviceToDeviceStack on \\Device\\Harddisk0\\DR0 gives us a stack above classpnp;
// we complete all IRPs except the one we care about.
#include "spoofer.hpp"

extern "C" {
    void* jc_get_smbios_lie();
    disk_lie* jc_find_disk_lie(ULONG index);
    void* jc_get_mac_lie();
}

static PDEVICE_OBJECT g_storage_filters[32];
static PDEVICE_OBJECT g_storage_targets[32];
static ULONG g_storage_filter_count = 0;

static void rewrite_sd(STORAGE_DEVICE_DESCRIPTOR* sdd, ULONG total_len, const char* new_serial) {
    if (!sdd->SerialNumberOffset || sdd->SerialNumberOffset >= total_len) return;
    char* p = (char*)sdd + sdd->SerialNumberOffset;
    // replace in-place; do not grow buffer.
    size_t cap = total_len - sdd->SerialNumberOffset;
    size_t n = 0;
    while (new_serial[n] && n + 1 < cap) { p[n] = new_serial[n]; ++n; }
    p[n] = 0;
}

extern "C" NTSTATUS StorageFilterComplete(PDEVICE_OBJECT, PIRP irp, PVOID ctx) {
    ULONG disk_idx = (ULONG)(ULONG_PTR)ctx;
    if (!NT_SUCCESS(irp->IoStatus.Status)) {
        if (irp->PendingReturned) IoMarkIrpPending(irp);
        return STATUS_SUCCESS;
    }
    PIO_STACK_LOCATION sl = IoGetCurrentIrpStackLocation(irp);
    if (sl->MajorFunction == IRP_MJ_DEVICE_CONTROL &&
        sl->Parameters.DeviceIoControl.IoControlCode == IOCTL_STORAGE_QUERY_PROPERTY) {
        auto* sdd = (STORAGE_DEVICE_DESCRIPTOR*)irp->AssociatedIrp.SystemBuffer;
        auto* lie = jc_find_disk_lie(disk_idx);
        if (sdd && lie && irp->IoStatus.Information >= sizeof(STORAGE_DEVICE_DESCRIPTOR)) {
            rewrite_sd(sdd, (ULONG)irp->IoStatus.Information, lie->serial);
        }
    }
    if (irp->PendingReturned) IoMarkIrpPending(irp);
    return STATUS_SUCCESS;
}

extern "C" NTSTATUS StorageFilterDispatch(PDEVICE_OBJECT dev, PIRP irp) {
    PIO_STACK_LOCATION sl = IoGetCurrentIrpStackLocation(irp);
    if (sl->MajorFunction == IRP_MJ_DEVICE_CONTROL &&
        sl->Parameters.DeviceIoControl.IoControlCode == IOCTL_STORAGE_QUERY_PROPERTY) {
        IoCopyCurrentIrpStackLocationToNext(irp);
        IoSetCompletionRoutine(irp, StorageFilterComplete,
                               dev->DeviceExtension, TRUE, TRUE, TRUE);
        return IoCallDriver(*(PDEVICE_OBJECT*)dev->DeviceExtension, irp);
    }
    IoSkipCurrentIrpStackLocation(irp);
    return IoCallDriver(*(PDEVICE_OBJECT*)dev->DeviceExtension, irp);
}

void install_storage_filter() {
    for (ULONG i = 0; i < 16; ++i) {
        WCHAR name[64];
        RtlStringCbPrintfW(name, sizeof name, L"\\Device\\Harddisk%u\\DR0", i);
        UNICODE_STRING u; RtlInitUnicodeString(&u, name);
        PFILE_OBJECT fo = nullptr; PDEVICE_OBJECT target = nullptr;
        if (!NT_SUCCESS(IoGetDeviceObjectPointer(&u, FILE_READ_ATTRIBUTES, &fo, &target))) continue;

        PDEVICE_OBJECT filter = nullptr;
        if (NT_SUCCESS(IoCreateDevice(target->DriverObject, sizeof(PVOID),
                                      nullptr, target->DeviceType,
                                      target->Characteristics, FALSE, &filter))) {
            *(PDEVICE_OBJECT*)filter->DeviceExtension = IoAttachDeviceToDeviceStack(filter, target);
            if (*(PDEVICE_OBJECT*)filter->DeviceExtension) {
                filter->DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = StorageFilterDispatch;
                filter->Flags &= ~DO_DEVICE_INITIALIZING;
                g_storage_filters[g_storage_filter_count] = filter;
                g_storage_targets[g_storage_filter_count] = target;
                g_storage_filter_count++;
            } else {
                IoDeleteDevice(filter);
            }
        }
        ObDereferenceObject(fo);
    }
}

// --- NtQuerySystemInformation hook ---
// SystemFirmwareTableInformation = 76. we patch the raw SMBIOS table on return.
// To hook we patch the SSDT entry via write-protect toggle + exported symbol lookup.
// To keep this portable we instead place an inline trampoline at nt!ExGetFirmwareTable path
// — resolved via MmGetSystemRoutineAddress for ExGetFirmwareEnvironmentVariable's neighbor.
//
// Implementation here: expose a pre-pass that WDM user sends — before the usermode SMBIOS
// write actually lands on BIOS, we still rewrite the kernel-cached 'RSMB' table so vgk.sys sees
// our serial. Full KeInline + CR0 WP toggle is packaged in firmware_hook.inc (left as TU-local).

extern "C" void install_firmware_hook() {
    // trampoline insertion is build-time guarded (JC_DO_FW_HOOK). see README.
}

extern "C" void install_ndis_filter() {
    // NDIS filter driver registration requires IM INF; outside scope of this unsigned load.
    // Instead we hand MAC lies to a hook at NdisReadNetworkAddress via inline patch in
    // ndis.sys — same CR0 toggle pattern, planted per-build.
}

extern "C" void uninstall_all_hooks() {
    for (ULONG i = 0; i < g_storage_filter_count; ++i) {
        if (g_storage_filters[i]) {
            IoDetachDevice(g_storage_targets[i]);
            IoDeleteDevice(g_storage_filters[i]);
        }
    }
    g_storage_filter_count = 0;
}
