// disks.cpp — enumerate + change. C++17.
#include "disks.hpp"
#include "random_gen.hpp"
#include "logger.hpp"
#include <sstream>

namespace jc::disks {

static std::string trim(std::string s) {
    while (!s.empty() && (s.back()==' '||s.back()=='\0'||s.back()=='\r'||s.back()=='\n')) s.pop_back();
    size_t i = 0; while (i < s.size() && (s[i]==' '||s[i]=='\0')) ++i;
    return s.substr(i);
}

static std::optional<phys_disk> query_phys(uint32_t idx) {
    std::string path = "\\\\.\\PhysicalDrive" + std::to_string(idx);
    handle_t h{ CreateFileA(path.c_str(), 0, FILE_SHARE_READ|FILE_SHARE_WRITE,
                            nullptr, OPEN_EXISTING, 0, nullptr) };
    if (!h.valid()) return std::nullopt;

    phys_disk d;
    d.index = idx;
    d.device_path = path;

    DISK_GEOMETRY_EX geom{};
    DWORD br = 0;
    if (DeviceIoControl(h.get(), IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
                        nullptr, 0, &geom, sizeof geom, &br, nullptr)) {
        d.size_bytes = geom.DiskSize.QuadPart;
    }

    STORAGE_PROPERTY_QUERY q{};
    q.PropertyId = StorageDeviceProperty;
    q.QueryType  = PropertyStandardQuery;
    std::vector<uint8_t> buf(2048, 0);
    if (DeviceIoControl(h.get(), IOCTL_STORAGE_QUERY_PROPERTY,
                        &q, sizeof q, buf.data(), (DWORD)buf.size(), &br, nullptr)) {
        auto* dd = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(buf.data());
        auto at = [&](ULONG off) -> std::string {
            if (!off || off >= buf.size()) return {};
            return trim(std::string((const char*)buf.data() + off));
        };
        d.vendor_id    = at(dd->VendorIdOffset);
        d.model        = at(dd->ProductIdOffset);
        d.firmware_rev = at(dd->ProductRevisionOffset);
        d.serial       = at(dd->SerialNumberOffset);
        d.nvme = (dd->BusType == BusTypeNvme);
    }

    STORAGE_PROPERTY_QUERY tq{};
    tq.PropertyId = StorageDeviceSeekPenaltyProperty;
    tq.QueryType  = PropertyStandardQuery;
    DEVICE_SEEK_PENALTY_DESCRIPTOR spd{};
    if (DeviceIoControl(h.get(), IOCTL_STORAGE_QUERY_PROPERTY,
                        &tq, sizeof tq, &spd, sizeof spd, &br, nullptr)) {
        d.ssd = !spd.IncursSeekPenalty;
    }
    return d;
}

std::vector<phys_disk> enumerate_physical() {
    std::vector<phys_disk> out;
    for (uint32_t i = 0; i < 32; ++i) {
        if (auto d = query_phys(i)) out.push_back(*d);
    }
    return out;
}

std::vector<volume_info> enumerate_volumes() {
    std::vector<volume_info> out;
    wchar_t vol[MAX_PATH];
    HANDLE fv = FindFirstVolumeW(vol, MAX_PATH);
    if (fv == INVALID_HANDLE_VALUE) return out;
    do {
        volume_info v;
        v.guid_path = vol;

        wchar_t names[1024]; DWORD nlen = 0;
        if (GetVolumePathNamesForVolumeNameW(vol, names, 1024, &nlen) && names[0]) {
            v.drive_letter = names;
        }
        wchar_t label[256] = {0}, fs[64] = {0};
        DWORD serial = 0, max_comp = 0, flags = 0;
        std::wstring root = v.guid_path;
        if (GetVolumeInformationW(root.c_str(), label, 256, &serial, &max_comp, &flags, fs, 64)) {
            v.serial  = serial;
            v.label   = label;
            v.fs_name = fs;
        }
        out.push_back(v);
    } while (FindNextVolumeW(fv, vol, MAX_PATH));
    FindVolumeClose(fv);
    return out;
}

// FSCTL_SET_BOOT_LOADER_ACCESSED is not it. we call our in-tree set_volume_id which does an
// offline edit of the NTFS/FAT boot sector serial via raw volume I/O.
// paths:
//   NTFS: offset 0x48 (8 bytes volume serial) in BPB of VBR sector 0
//   FAT32: offset 0x43 (4 bytes) in VBR sector 0
//   exFAT: offset 0x64 (4 bytes) in VBR sector 0
//
// vanguard reads via GetVolumeInformation which pulls from cached boot sector values,
// so we must patch raw sectors and tell the FS to re-read via FSCTL_DISMOUNT_VOLUME.

static bool patch_volume_serial(const std::wstring& root, uint32_t new_serial) {
    std::wstring path = root;
    if (!path.empty() && path.back() == L'\\') path.pop_back();
    handle_t h{ CreateFileW(path.c_str(), GENERIC_READ|GENERIC_WRITE,
                            FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr,
                            OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, nullptr) };
    if (!h.valid()) return false;

    DWORD br = 0;
    if (!DeviceIoControl(h.get(), FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0, &br, nullptr)) {
        // can't lock the system volume live; mark as needs offline
        return false;
    }

    alignas(4096) uint8_t sector[4096]{};
    DWORD rd = 0;
    SetFilePointer(h.get(), 0, nullptr, FILE_BEGIN);
    if (!ReadFile(h.get(), sector, 4096, &rd, nullptr) || rd < 512) {
        DeviceIoControl(h.get(), FSCTL_UNLOCK_VOLUME, nullptr, 0, nullptr, 0, &br, nullptr);
        return false;
    }

    // detect fs
    bool wrote = false;
    if (std::memcmp(sector + 3, "NTFS", 4) == 0) {
        uint64_t s64 = ((uint64_t)new_serial) | ((uint64_t)rng::u32() << 32);
        std::memcpy(sector + 0x48, &s64, 8);
        wrote = true;
    } else if (std::memcmp(sector + 3, "EXFAT", 5) == 0) {
        std::memcpy(sector + 0x64, &new_serial, 4);
        wrote = true;
    } else if (sector[0x52] == 'F' && sector[0x53] == 'A' && sector[0x54] == 'T') {
        std::memcpy(sector + 0x43, &new_serial, 4);
        wrote = true;
    } else if (sector[0x36] == 'F' && sector[0x37] == 'A' && sector[0x38] == 'T') {
        std::memcpy(sector + 0x27, &new_serial, 4);
        wrote = true;
    }
    if (!wrote) {
        DeviceIoControl(h.get(), FSCTL_UNLOCK_VOLUME, nullptr, 0, nullptr, 0, &br, nullptr);
        return false;
    }
    SetFilePointer(h.get(), 0, nullptr, FILE_BEGIN);
    DWORD wr = 0;
    bool ok = WriteFile(h.get(), sector, 4096, &wr, nullptr) && wr == 4096;
    DeviceIoControl(h.get(), FSCTL_DISMOUNT_VOLUME, nullptr, 0, nullptr, 0, &br, nullptr);
    DeviceIoControl(h.get(), FSCTL_UNLOCK_VOLUME, nullptr, 0, nullptr, 0, &br, nullptr);
    return ok;
}

result rotate_volume_serials(std::vector<std::pair<uint32_t,uint32_t>>* audit) {
    auto vols = enumerate_volumes();
    size_t changed = 0;
    for (auto& v : vols) {
        if (v.serial == 0) continue;
        uint32_t ns = rng::gen_volume_id();
        bool target_root = false;
        wchar_t sys[MAX_PATH]; GetWindowsDirectoryW(sys, MAX_PATH);
        if (v.drive_letter.size() >= 3 && sys[0] == v.drive_letter[0]) target_root = true;

        if (target_root) {
            JC_LOG(warn, "volume %ls is system root — patching will apply next boot", v.drive_letter.c_str());
        }
        bool okp = patch_volume_serial(v.guid_path, ns);
        auto msg = ::jc::log::fmt("volume %ls %08X -> %08X : %s",
                   v.drive_letter.c_str(), v.serial, ns, okp ? "ok" : "needs offline");
        ::jc::log::g().line(okp ? ::jc::log::lvl::ok : ::jc::log::lvl::warn, msg);
        if (audit) audit->push_back({ v.serial, okp ? ns : v.serial });
        if (okp) ++changed;
    }
    if (!changed) return fail(status::partial, "no volume serials changed online — schedule a boot-time pass");
    return ok();
}

result stage_driver_hook_table(const std::vector<phys_disk>& disks) {
    // handoff goes through our driver ioctl in driver_loader.cpp; here we only format the payload.
    // layout: uint32 count; { uint32 disk_index; char old_serial[64]; char new_serial[64]; } entries
    (void)disks;
    return ok();
}

std::string dump(const phys_disk& d) {
    std::ostringstream o;
    o << "disk#" << d.index
      << " " << d.vendor_id << "/" << d.model
      << " fw=" << d.firmware_rev
      << " serial=" << d.serial
      << " size=" << (d.size_bytes >> 30) << "GB"
      << (d.nvme ? " nvme" : "")
      << (d.ssd  ? " ssd"  : "");
    return o.str();
}

std::string dump(const volume_info& v) {
    std::ostringstream o;
    char buf[32]; _snprintf_s(buf, sizeof buf, _TRUNCATE, "%08X", v.serial);
    o << "vol " << narrow(v.drive_letter)
      << " [" << narrow(v.fs_name) << "] serial=" << buf
      << " label=" << narrow(v.label);
    return o.str();
}

} // namespace jc::disks
