// main.cpp — jailcrash valorant hwid cleaner.
// windows 11 x64, msvc c++17, must run as administrator.
// order of operations matters: vanguard off -> firmware -> block volumes -> macs -> regs -> tpm -> gpu/edid -> validate.
#include "common.hpp"
#include "logger.hpp"
#include "smbios.hpp"
#include "disks.hpp"
#include "macs.hpp"
#include "registry_ids.hpp"
#include "tpm.hpp"
#include "gpu_edid.hpp"
#include "vanguard.hpp"
#include "validator.hpp"
#include "driver_loader.hpp"
#include "extras.hpp"
#include "evasion.hpp"
#include "boot_phase.hpp"
#include "firmware_vars.hpp"
#include "self_check.hpp"

#include <iostream>

static bool is_admin() {
    BOOL elevated = FALSE;
    HANDLE t{};
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &t)) {
        TOKEN_ELEVATION e{};
        DWORD sz = sizeof e;
        if (GetTokenInformation(t, TokenElevation, &e, sz, &sz)) elevated = e.TokenIsElevated;
        CloseHandle(t);
    }
    return !!elevated;
}

static void banner() {
    JC_LOG(step, "jailcrash — valorant hardban cleaner");
    JC_LOG(info, "surfaces: smbios  disks  volumes  macs  regids  tpm  gpu  edid  vanguard");
    JC_LOG(info, "run order: capture -> wipe vanguard -> spoof each surface -> validate");
}

int wmain(int argc, wchar_t** argv) {
    jc::log::g().open("spoofer.log");
    jc::self::install_crash_handler();
    if (!jc::self::acquire_single_instance()) {
        JC_LOG(err, "another jc_hwid_cleaner instance is already running");
        return 4;
    }
    banner();

    bool dry = false;
    bool keep_vg = false;
    bool validate_only = false;
    bool use_driver = false;
    bool rotate_name = false;
    bool keep_riot_client = false;
    bool post_boot = false;
    bool unblock_hosts = false;
    bool auto_launch = false;
    bool bcd_unlock = false;
    bool bcd_lock = false;
    bool uefi_rotate = false;
    for (int i = 1; i < argc; ++i) {
        std::wstring a = argv[i];
        if      (a == L"--dry-run")           dry = true;
        else if (a == L"--keep-vanguard")     keep_vg = true;
        else if (a == L"--keep-riot-client")  keep_riot_client = true;
        else if (a == L"--validate-only")     validate_only = true;
        else if (a == L"--with-driver")       use_driver = true;
        else if (a == L"--rotate-name")       rotate_name = true;
        else if (a == L"--post-boot")         post_boot = true;
        else if (a == L"--unblock-hosts")     unblock_hosts = true;
        else if (a == L"--auto-launch")       auto_launch = true;
        else if (a == L"--bcd-unlock")        bcd_unlock = true;
        else if (a == L"--bcd-lock")          bcd_lock = true;
        else if (a == L"--uefi-rotate")       uefi_rotate = true;
    }

    if (bcd_lock) { jc::fwvar::disable_test_signing(); return 0; }

    if (post_boot) {
        JC_LOG(step, "post-boot mode — running deferred sweep");
        jc::boot::run_post_boot();
        return 0;
    }
    if (unblock_hosts) {
        jc::boot::uninstall_hosts_block();
        return 0;
    }

    if (!is_admin()) {
        JC_LOG(err, "must run elevated — right-click, Run as Administrator");
        return 2;
    }

    // evasion: quieten ETW *before* we touch anything sensitive.
    jc::evasion::disable_etw_for_self();

    // refuse to run while Vanguard is live — flashing SMBIOS with vgk loaded is a silent ban trigger.
    if (jc::evasion::vgk_is_loaded()) {
        JC_LOG(err, "vgk.sys is currently loaded — reboot with Vanguard disabled (BCDEDIT nointegritychecks or safe mode), then re-run");
        return 3;
    }

    JC_LOG(step, "snapshot: capturing pre-spoof state");
    auto before = jc::validate::capture();
    JC_LOG(info, "pre: sys_serial=%s uuid=%s machineguid=%s",
           before.system_serial.c_str(),
           before.system_uuid.c_str(),
           before.machine_guid.c_str());
    JC_LOG(info, "pre: disks=%zu volumes=%zu macs=%zu monitors=%zu",
           before.disk_serials.size(),
           before.volume_serials.size(),
           before.mac_addresses.size(),
           before.monitor_edid_hashes.size());

    if (validate_only) {
        JC_LOG(ok, "validate-only mode — dumping and exiting");
        return 0;
    }
    if (dry) {
        JC_LOG(warn, "dry-run — no changes will be applied");
        return 0;
    }

    jc::validate::report rep{};

    // 1) vanguard first — can't change SMBIOS while vgk is loaded on some boards
    if (!keep_vg) {
        JC_LOG(step, "vanguard: uninstall + trace wipe");
        auto vs = jc::vanguard::status();
        JC_LOG(info, "%s", jc::vanguard::dump(vs).c_str());
        jc::vanguard::uninstall();
        jc::vanguard::wipe_traces();
        jc::vanguard::block_reinstall();
        rep.vanguard_wiped = true;
    } else {
        JC_LOG(warn, "vanguard: keeping (--keep-vanguard)");
    }

    // 2) SMBIOS
    JC_LOG(step, "smbios: plan + apply");
    auto sms = jc::smbios::read();
    JC_LOG(info, "pre: %s", jc::smbios::dump(sms).c_str());
    auto plan = jc::smbios::make_plan(sms);
    JC_LOG(info, "plan: sys=%s uuid=%s board=%s chassis=%s",
           plan.system_serial.c_str(), plan.system_uuid.c_str(),
           plan.baseboard_serial.c_str(), plan.chassis_serial.c_str());
    auto rsmb = jc::smbios::apply(plan);
    if (!rsmb) JC_LOG(warn, "smbios apply: %s", rsmb.what.c_str());

    // 3) MACs
    JC_LOG(step, "mac: enum + spoof");
    std::vector<std::pair<std::string,std::string>> mac_audit;
    auto rmac = jc::macs::spoof_all(&mac_audit);
    if (!rmac) JC_LOG(warn, "mac spoof: %s", rmac.what.c_str());

    // 4) Registry IDs
    JC_LOG(step, "regids: rotate");
    jc::regids::rotate();

    // 5) Volume serials
    JC_LOG(step, "volumes: rotate serials");
    std::vector<std::pair<uint32_t,uint32_t>> vol_audit;
    jc::disks::rotate_volume_serials(&vol_audit);

    // 6) GPU + EDID
    JC_LOG(step, "gpu+edid: rotate");
    jc::gpu::rotate_gpu_registry_ids();
    jc::gpu::rotate_monitor_edid();

    // 6.5) optional UEFI rotation
    if (uefi_rotate) {
        JC_LOG(step, "uefi: rotating identity vars");
        jc::fwvar::rotate_identity_vars();
    }
    if (bcd_unlock) {
        JC_LOG(step, "bcd: enabling testsigning (breaks HVCI until --bcd-lock)");
        jc::fwvar::enable_test_signing();
    }

    // 7) TPM clear (reboot-time consent)
    JC_LOG(step, "tpm: request clear via PPI");
    auto rtpm = jc::tpm::request_clear_via_ppi();
    rep.tpm_clear_scheduled = !!rtpm;
    if (!rtpm) JC_LOG(warn, "tpm clear: %s", rtpm.what.c_str());

    // 7.5) second-pass caches
    JC_LOG(step, "extras: bluetooth, WMI CIM, BAM/DAM, system vault, prefetch, VSS, credentials");
    jc::extras::wipe_bluetooth_bd_addr();
    jc::extras::wipe_wmi_cim_cache();
    jc::extras::wipe_bam_dam_history();
    jc::extras::wipe_system_vault_device_id();
    jc::extras::wipe_prefetch_global();
    jc::boot::wipe_vss_snapshots();
    jc::boot::wipe_credential_manager_riot();
    jc::boot::install_hosts_block();
    if (!keep_riot_client) {
        JC_LOG(step, "riot client: wipe local install (you'll fresh-login after)");
        jc::extras::wipe_riot_client_remnants();
    }
    if (rotate_name) {
        JC_LOG(step, "computer name: rotate");
        jc::extras::rotate_computer_name();
    }

    // 8) optional: load kernel spoofer so Vanguard's first post-boot read from storage stack
    //    returns our lies even if BIOS was locked for firmware strings.
    if (use_driver) {
        JC_LOG(step, "byovd: load vuln driver + manual map our spoofer");
        std::wstring svc;
        auto rld = jc::byovd::load_vuln(jc::byovd::vuln::iqvw64e_intel, &svc);
        if (!rld) JC_LOG(warn, "byovd load: %s", rld.what.c_str());
        else {
            jc::byovd::mapped_driver md{};
            auto rm = jc::byovd::map_our_driver(jc::byovd::vuln::iqvw64e_intel,
                                                std::filesystem::current_path() / "tools" / "spoofer.sys", &md);
            if (!rm) JC_LOG(warn, "byovd map: %s", rm.what.c_str());
            // vuln driver stays loaded while our hooks are active.
        }
    }

    JC_LOG(step, "snapshot: capturing post-spoof state");
    auto after = jc::validate::capture();
    auto r = jc::validate::run_full(before, after);
    r.smbios_moved      = r.smbios_moved      || (rsmb.st == jc::status::ok);
    r.tpm_clear_scheduled = rep.tpm_clear_scheduled;
    r.vanguard_wiped    = rep.vanguard_wiped;
    JC_LOG(info, "%s", jc::validate::print(r).c_str());
    jc::validate::export_json(r, "spoof_report.json");

    // evasion closure: timestamp-smear our exe, plant decoy
    wchar_t self[MAX_PATH]; GetModuleFileNameW(nullptr, self, MAX_PATH);
    jc::evasion::smear_timestamps(self);
    jc::evasion::plant_decoy_7045(L"NetAdapterCx_Audit");

    // schedule the post-boot sweep (system VBR patch, MAC re-apply, cleanup)
    jc::boot::install_post_boot_task(self);

    JC_LOG(step, "done. reboot now.");
    JC_LOG(info, "post-boot pass will auto-run at next logon and clean up after itself.");
    JC_LOG(info, "after validation passes, run --unblock-hosts to restore Riot telemetry endpoints,");
    JC_LOG(info, "then reinstall Valorant from https://playvalorant.com");
    if (auto_launch) {
        JC_LOG(step, "auto-launch: handing off to Riot Client");
        jc::vanguard::unblock();
        jc::boot::uninstall_hosts_block();
        jc::boot::reinstall_valorant_optional();
    }
    return 0;
}
