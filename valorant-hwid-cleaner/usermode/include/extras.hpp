// extras.hpp — surfaces Vanguard sometimes reaches: Bluetooth stack, WMI caches, BAM, Riot client
// install leftovers, DeviceID, Windows camera-mic device id.
#pragma once
#include "common.hpp"

namespace jc::extras {

result wipe_bluetooth_bd_addr();      // HKLM\SYSTEM\CurrentControlSet\Services\BTHPORT\Parameters\...
result wipe_wmi_cim_cache();          // net stop winmgmt + delete repository
result wipe_bam_dam_history();        // HKLM\SYSTEM\CurrentControlSet\Services\bam\State\UserSettings\*
result wipe_riot_client_remnants();   // ProgramData\Riot Games\Metadata, AppData\Local\Riot Games
result rotate_computer_name();        // optional — only if the caller passes --rotate-name
result wipe_system_vault_device_id(); // HKCU + HKLM device token caches used by MS diag telemetry
result wipe_prefetch_global();        // nuke all .pf and rebuild

} // namespace jc::extras
