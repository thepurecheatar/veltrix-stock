// vanguard.hpp — stop + remove Vanguard so our BIOS/MAC/volume changes are the ones Vanguard sees on next boot.
#pragma once
#include "common.hpp"

namespace jc::vanguard {

struct state {
    bool vgc_installed{};
    bool vgk_driver_present{};
    bool vgc_running{};
    bool vgk_loaded{};
    std::string install_dir;
    std::string vgk_path;
};

state   status();
result  stop_services();   // net stop vgc; sc stop vgk (vgk is boot-start so will come back next reboot)
result  uninstall();       // runs VanguardUninstall.exe if present, falls back to manual wipe
result  wipe_traces();     // ProgramData\Riot Vanguard, registry, logs, prefetch, WER
result  block_reinstall(); // adds a firewall rule + file ACL deny so Vanguard installer can't resurrect mid-run
result  unblock();         // reverse of block_reinstall

std::string dump(const state& s);

} // namespace jc::vanguard
