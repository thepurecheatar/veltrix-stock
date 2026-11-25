// validator.cpp — reads every surface from scratch and prints a diff.
#include "validator.hpp"
#include "smbios.hpp"
#include "disks.hpp"
#include "macs.hpp"
#include "registry_ids.hpp"
#include "tpm.hpp"
#include "gpu_edid.hpp"
#include "vanguard.hpp"
#include "logger.hpp"
#include <sstream>
#include <fstream>

namespace jc::validate {

machine_snapshot capture() {
    machine_snapshot s;
    auto sm = jc::smbios::read();
    s.bios_vendor = sm.bios_vendor;
    s.bios_version = sm.bios_version;
    s.bios_serial = sm.system_serial; // SMBIOS type 0 has no serial, we use type 1
    s.system_serial = sm.system_serial;
    s.system_uuid = sm.system_uuid;
    s.baseboard_serial = sm.baseboard_serial;
    s.chassis_serial = sm.chassis_serial;

    for (auto& d : jc::disks::enumerate_physical()) s.disk_serials.push_back(d.serial);
    for (auto& v : jc::disks::enumerate_volumes()) {
        char b[16]; _snprintf_s(b, sizeof b, _TRUNCATE, "%08X", v.serial);
        s.volume_serials.push_back(b);
    }
    for (auto& n : jc::macs::enumerate()) s.mac_addresses.push_back(n.mac_runtime);

    auto r = jc::regids::read();
    s.machine_guid = r.machine_guid;
    s.sqm_machine_id = r.sqm_machine_id;
    s.sus_client_id = r.sus_client_id;
    s.hw_profile_guid = r.hw_profile_guid;
    s.windows_installation_id = r.windows_install_id;

    auto tpm = jc::tpm::read();
    s.tpm_ek_pub_hash = tpm.ek_pub_sha256;

    for (auto& g : jc::gpu::enum_gpus()) s.gpu_uuids.push_back(narrow(g.device_instance_id));
    for (auto& m : jc::gpu::enum_monitors()) s.monitor_edid_hashes.push_back(m.edid_sha256);
    return s;
}

static bool differ(const std::string& a, const std::string& b) {
    return !a.empty() && !b.empty() && a != b;
}

report run_full(const machine_snapshot& a, const machine_snapshot& b) {
    report r;
    r.smbios_moved = differ(a.system_serial, b.system_serial)
                  || differ(a.system_uuid, b.system_uuid)
                  || differ(a.baseboard_serial, b.baseboard_serial);
    r.machine_guid_moved = differ(a.machine_guid, b.machine_guid);
    r.sqm_moved          = differ(a.sqm_machine_id, b.sqm_machine_id);
    r.sus_moved          = differ(a.sus_client_id, b.sus_client_id);
    r.hw_profile_moved   = differ(a.hw_profile_guid, b.hw_profile_guid);

    for (size_t i = 0; i < a.volume_serials.size() && i < b.volume_serials.size(); ++i)
        if (differ(a.volume_serials[i], b.volume_serials[i])) ++r.volume_serials_moved;
    for (size_t i = 0; i < a.mac_addresses.size() && i < b.mac_addresses.size(); ++i)
        if (differ(a.mac_addresses[i], b.mac_addresses[i])) ++r.macs_moved;
    for (size_t i = 0; i < a.monitor_edid_hashes.size() && i < b.monitor_edid_hashes.size(); ++i)
        if (differ(a.monitor_edid_hashes[i], b.monitor_edid_hashes[i])) ++r.edid_moved;

    if (!r.smbios_moved)       r.notes.push_back("SMBIOS unchanged — BIOS lock? vendor tool missing?");
    if (!r.machine_guid_moved) r.notes.push_back("MachineGuid unchanged — did you run as admin?");
    if (!r.volume_serials_moved) r.notes.push_back("no volume serials moved — reboot and re-run for the system disk");
    if (!r.macs_moved)         r.notes.push_back("no MACs moved — driver may be ignoring NetworkAddress (firmware override); retry with the kernel NDIS hook");
    return r;
}

std::string print(const report& r) {
    std::ostringstream o;
    auto yn = [](bool b) { return b ? "yes" : "no "; };
    o << "=== spoof report ===\n"
      << "smbios moved      : " << yn(r.smbios_moved)       << "\n"
      << "machine_guid moved: " << yn(r.machine_guid_moved) << "\n"
      << "sqm moved         : " << yn(r.sqm_moved)          << "\n"
      << "sus moved         : " << yn(r.sus_moved)          << "\n"
      << "hw_profile moved  : " << yn(r.hw_profile_moved)   << "\n"
      << "volumes changed   : " << r.volume_serials_moved   << "\n"
      << "macs changed      : " << r.macs_moved             << "\n"
      << "edid blobs changed: " << r.edid_moved             << "\n"
      << "tpm clear sched   : " << yn(r.tpm_clear_scheduled)<< "\n"
      << "vanguard wiped    : " << yn(r.vanguard_wiped)     << "\n";
    for (auto& n : r.notes) o << "note: " << n << "\n";
    return o.str();
}

static std::string jesc(const std::string& s) {
    std::string o; o.reserve(s.size()+2);
    for (auto c : s) {
        if (c == '"' || c == '\\') o += '\\';
        if ((unsigned char)c < 0x20) { char b[8]; _snprintf_s(b,sizeof b,_TRUNCATE,"\\u%04x",c); o += b; continue; }
        o += c;
    }
    return o;
}

void export_json(const report& r, const std::filesystem::path& out) {
    std::ofstream f(out);
    f << "{\n";
    f << "  \"smbios_moved\":" << (r.smbios_moved ? "true" : "false") << ",\n";
    f << "  \"machine_guid_moved\":" << (r.machine_guid_moved ? "true" : "false") << ",\n";
    f << "  \"sqm_moved\":" << (r.sqm_moved ? "true" : "false") << ",\n";
    f << "  \"sus_moved\":" << (r.sus_moved ? "true" : "false") << ",\n";
    f << "  \"hw_profile_moved\":" << (r.hw_profile_moved ? "true" : "false") << ",\n";
    f << "  \"volume_serials_moved\":" << r.volume_serials_moved << ",\n";
    f << "  \"macs_moved\":" << r.macs_moved << ",\n";
    f << "  \"edid_moved\":" << r.edid_moved << ",\n";
    f << "  \"tpm_clear_scheduled\":" << (r.tpm_clear_scheduled ? "true" : "false") << ",\n";
    f << "  \"vanguard_wiped\":" << (r.vanguard_wiped ? "true" : "false") << ",\n";
    f << "  \"notes\":[";
    for (size_t i = 0; i < r.notes.size(); ++i) {
        if (i) f << ",";
        f << "\"" << jesc(r.notes[i]) << "\"";
    }
    f << "]\n}\n";
}

} // namespace jc::validate
