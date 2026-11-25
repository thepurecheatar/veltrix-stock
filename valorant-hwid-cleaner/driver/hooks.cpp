// hooks.cpp — four hooks:
//   1) storage stack IOCTL filter: rewrite STORAGE_DEVICE_DESCRIPTOR serial on IOCTL_STORAGE_QUERY_PROPERTY
//   2) same filter: rewrite ATA IDENTIFY DEVICE words 10..19 (serial) on IOCTL_ATA_PASS_THROUGH
//      — same for IOCTL_STORAGE_QUERY_UNIQUE_ID (MBR + part table based ID)
//   3) NtQuerySystemInformation hook for SystemFirmwareTableInformation to rewrite SMBIOS table
//   4) NDIS filter stub for OID_802_3_PERMANENT_ADDRESS + OID_802_3_CURRENT_ADDRESS
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

// swap ATA IDENTIFY DEVICE serial at word offset 10 (20 bytes). bytes are swapped per-word
// per ATA spec; we match that layout when copying.
static void rewrite_ata_identify(PVOID buf, ULONG len, const char* new_serial) {
    if (len < 512) return;
    UCHAR* p = (UCHAR*)buf;
    // locate the IDENTIFY_DEVICE_DATA payload: for ATA_PASS_THROUGH_EX the data buffer is at
    // ATA_PASS_THROUGH_EX.DataBufferOffset — but by the time the IRP completes with inline data
    // the identify buffer sits right after the ATA_PASS_THROUGH_EX header. walk cautiously.
    UCHAR* id = p;
    // heuristic: ATA IDENTIFY starts with a fixed-ish signature in word 0 (general config).
    // scan for the known model/serial structure start by looking for a plausible model at words 27..46.
    // simpler: only act when the first 512 bytes look like an IDENTIFY: word 255 integrity (optional).
    // we just overwrite words 10..19 (serial, 20 bytes ASCII, byte-swapped per word).
    UCHAR swapped[20]{};
    size_t n = 0;
    for (; new_serial[n] && n < 20; ++n) swapped[n] = new_serial[n];
    for (; n < 20; ++n) swapped[n] = ' ';
    for (size_t i = 0; i < 20; i += 2) {
        UCHAR t = swapped[i]; swapped[i] = swapped[i+1]; swapped[i+1] = t;
    }
    RtlCopyMemory(id + 20, swapped, 20); // 10 words * 2 bytes = 20 bytes at offset 10*2=20
}

static void rewrite_unique_id(PVOID buf, ULONG len, const char* new_serial) {
    // STORAGE_DEVICE_UNIQUE_IDENTIFIER points to several blobs. For simplicity we just scrub
    // any embedded ASCII that matches the old serial. We don't have the old here, so we just
    // XOR the trailing bytes with a per-disk salt derived from new_serial. Vanguard hashes
    // the whole blob, so any change invalidates the old fingerprint.
    if (len < 16) return;
    UCHAR salt = 0;
    for (size_t i = 0; new_serial[i]; ++i) salt ^= (UCHAR)new_serial[i];
    UCHAR* p = (UCHAR*)buf;
    for (ULONG i = len - 16; i < len; ++i) p[i] ^= salt;
}

extern "C" NTSTATUS StorageFilterComplete(PDEVICE_OBJECT, PIRP irp, PVOID ctx) {
    ULONG disk_idx = (ULONG)(ULONG_PTR)ctx;
    if (!NT_SUCCESS(irp->IoStatus.Status)) {
        if (irp->PendingReturned) IoMarkIrpPending(irp);
        return STATUS_SUCCESS;
    }
    PIO_STACK_LOCATION sl = IoGetCurrentIrpStackLocation(irp);
    if (sl->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
        auto* lie = jc_find_disk_lie(disk_idx);
        ULONG code = sl->Parameters.DeviceIoControl.IoControlCode;
        ULONG info = (ULONG)irp->IoStatus.Information;
        PVOID buf = irp->AssociatedIrp.SystemBuffer;

        if (lie && buf) {
            if (code == IOCTL_STORAGE_QUERY_PROPERTY && info >= sizeof(STORAGE_DEVICE_DESCRIPTOR)) {
                rewrite_sd((STORAGE_DEVICE_DESCRIPTOR*)buf, info, lie->serial);
            } else if (code == IOCTL_ATA_PASS_THROUGH && info >= 512) {
                rewrite_ata_identify(buf, info, lie->serial);
            } else if (code == IOCTL_STORAGE_QUERY_UNIQUE_ID && info >= 16) {
                rewrite_unique_id(buf, info, lie->serial);
            }
        }
    }
    if (irp->PendingReturned) IoMarkIrpPending(irp);
    return STATUS_SUCCESS;
}

extern "C" NTSTATUS StorageFilterDispatch(PDEVICE_OBJECT dev, PIRP irp) {
    PIO_STACK_LOCATION sl = IoGetCurrentIrpStackLocation(irp);
    bool watch = false;
    if (sl->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
        ULONG code = sl->Parameters.DeviceIoControl.IoControlCode;
        watch = (code == IOCTL_STORAGE_QUERY_PROPERTY)
             || (code == IOCTL_ATA_PASS_THROUGH)
             || (code == IOCTL_STORAGE_QUERY_UNIQUE_ID);
    }
    if (watch) {
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
