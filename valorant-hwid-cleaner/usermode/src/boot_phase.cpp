// boot_phase.cpp — reboot discipline + post-reboot cleanup wired through schtasks.
#include "boot_phase.hpp"
#include "logger.hpp"
#include "disks.hpp"
#include "registry_ids.hpp"
#include "macs.hpp"
#include "validator.hpp"
#include <sstream>

namespace jc::boot {

static const wchar_t* TASK_NAME = L"\\Microsoft\\Windows\\WaaS\\DeviceAttestationBridge";

static int run_cmd(const std::wstring& cmd) {
    STARTUPINFOW si{sizeof si}; PROCESS_INFORMATION pi{};
    si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE;
    std::wstring m = cmd;
    if (!CreateProcessW(nullptr, m.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                        nullptr, nullptr, &si, &pi)) return -1;
    WaitForSingleObject(pi.hProcess, 60000);
    DWORD ec = 0; GetExitCodeProcess(pi.hProcess, &ec);
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    return (int)ec;
}

result install_post_boot_task(const std::filesystem::path& self) {
    // schtasks create at logon of any user, SYSTEM principal, highest.
    std::wstring cmd = L"schtasks /Create /F /TN \"" + std::wstring(TASK_NAME) +
        L"\" /SC ONLOGON /RL HIGHEST /RU SYSTEM /TR \"\\\"" + self.wstring() +
        L"\\\" --post-boot\"";
    int rc = run_cmd(cmd);
    if (rc != 0) return fail(status::api_fail, "schtasks create failed: " + std::to_string(rc));
    JC_LOG(ok, "boot: scheduled post-boot task registered");
    return ok();
}

result run_post_boot() {
    JC_LOG(step, "post-boot: re-sweep deferred surfaces");
    // 1) system volume is now read/writable before Vanguard spins up (if we didn't reinstall yet)
    std::vector<std::pair<uint32_t,uint32_t>> va;
    disks::rotate_volume_serials(&va);
    // 2) MachineGuid can be auto-regen by Windows on first boot — re-check and re-roll
    auto rid = regids::read();
    if (rid.machine_guid.empty() || rid.machine_guid.size() < 32) {
        regids::rotate();
    }
    // 3) MAC: confirm each physical NIC kept our address; re-apply if the firmware override
    //    pushed back the OEM mac on cold boot.
    macs::spoof_all();
    // 4) drop the scheduled task — one-shot.
    std::wstring cmd = L"schtasks /Delete /F /TN \"" + std::wstring(TASK_NAME) + L"\"";
    run_cmd(cmd);
    JC_LOG(ok, "post-boot: done, scheduled task removed");
    return ok();
}

result uninstall_tasks() {
    std::wstring cmd = L"schtasks /Delete /F /TN \"" + std::wstring(TASK_NAME) + L"\"";
    run_cmd(cmd);
    return ok();
}

result wipe_vss_snapshots() {
    int rc = run_cmd(L"vssadmin delete shadows /all /quiet");
    if (rc != 0) return fail(status::partial, "vssadmin returned " + std::to_string(rc));
    JC_LOG(ok, "vss: all shadow copies dropped");
    return ok();
}

result wipe_credential_manager_riot() {
    // cmdkey /list, then delete entries whose target contains riotgames/valorant
    // we invoke cmdkey directly for each known target name set Vanguard/Riot uses.
    const wchar_t* targets[] = {
        L"LegacyGeneric:target=Microsoft_OC1:riotgames.com",
        L"LegacyGeneric:target=Microsoft_OC1:valorant.com",
        L"LegacyGeneric:target=Microsoft_OC1:Riot Vanguard",
        L"RiotClient",
        L"RiotGames.LocalMachine",
    };
    for (auto t : targets) {
        std::wstring cmd = L"cmdkey /delete:" + std::wstring(t);
        run_cmd(cmd);
    }
    JC_LOG(ok, "credman: Riot/Vanguard credentials purged");
    return ok();
}

result install_hosts_block() {
    namespace fs = std::filesystem;
    fs::path h = L"C:\\Windows\\System32\\drivers\\etc\\hosts";
    // backup once
    fs::path bak = h; bak += L".jc.bak";
    std::error_code ec;
    if (!fs::exists(bak, ec)) fs::copy_file(h, bak, ec);
    FILE* f = nullptr;
    _wfopen_s(&f, h.wstring().c_str(), L"ab");
    if (!f) return fail(status::api_fail, "cannot open hosts");
    const char* block =
        "\r\n# jc-hardban-cleaner begin\r\n"
        "0.0.0.0 telemetry.vanguard.riotgames.com\r\n"
        "0.0.0.0 telemetry.riotgames.com\r\n"
        "0.0.0.0 anticheat.riotgames.com\r\n"
        "0.0.0.0 vgtelemetry.riotgames.com\r\n"
        "# jc-hardban-cleaner end\r\n";
    fwrite(block, 1, strlen(block), f);
    fclose(f);
    run_cmd(L"ipconfig /flushdns");
    JC_LOG(ok, "hosts: Riot telemetry endpoints blackholed (reverse with --unblock-hosts)");
    return ok();
}

result uninstall_hosts_block() {
    namespace fs = std::filesystem;
    fs::path h = L"C:\\Windows\\System32\\drivers\\etc\\hosts";
    fs::path bak = h; bak += L".jc.bak";
    std::error_code ec;
    if (fs::exists(bak, ec)) {
        fs::copy_file(bak, h, fs::copy_options::overwrite_existing, ec);
        fs::remove(bak, ec);
        run_cmd(L"ipconfig /flushdns");
        JC_LOG(ok, "hosts: restored from backup");
    }
    return ok();
}

result reinstall_valorant_optional() {
    // find RiotClientServices in default spots; if found, launch it with install vector.
    namespace fs = std::filesystem;
    const wchar_t* candidates[] = {
        L"C:\\Riot Games\\Riot Client\\RiotClientServices.exe",
        L"C:\\Program Files\\Riot Games\\Riot Client\\RiotClientServices.exe",
        L"C:\\Program Files (x86)\\Riot Games\\Riot Client\\RiotClientServices.exe",
    };
    for (auto c : candidates) {
        if (fs::exists(c)) {
            std::wstring cmd = std::wstring(L"\"") + c + L"\" --launch-product=valorant --launch-patchline=live";
            run_cmd(cmd);
            JC_LOG(ok, "riot client launched: %ls", c);
            return ok();
        }
    }
    return fail(status::partial, "Riot Client not found — install from https://playvalorant.com");
}

} // namespace jc::boot
