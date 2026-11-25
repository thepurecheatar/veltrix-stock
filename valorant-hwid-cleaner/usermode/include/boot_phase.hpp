// boot_phase.hpp — pieces that MUST run on the first boot after spoof, not in the same session.
// scheduled task re-runs the cleaner with --post-boot once, then deletes itself.
#pragma once
#include "common.hpp"

namespace jc::boot {

// install a one-shot scheduled task that runs the exe with --post-boot at the next user logon.
result install_post_boot_task(const std::filesystem::path& self_path);

// what --post-boot does: re-patch the system volume VBR (now writable), re-rotate MachineGuid
// if Windows restored it, re-check MAC stickiness, delete the scheduled task.
result run_post_boot();

// uninstall whatever we left — for cleanup after validation says everything stuck.
result uninstall_tasks();

// VSS: shadow copies preserve old volume state including old serials. Wipe them.
result wipe_vss_snapshots();

// Windows Credential Manager: Riot and Vanguard leave vault entries keyed to the old HWID.
result wipe_credential_manager_riot();

// hosts override: block Riot's telemetry endpoint during our spoof window so no handshake
// correlates old+new IDs on their side. REVERSED by uninstall_hosts_block.
result install_hosts_block();
result uninstall_hosts_block();

// reinstate Vanguard: unblock dir, install VALORANT via Riot Client CLI if present.
result reinstall_valorant_optional();

} // namespace jc::boot
