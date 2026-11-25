// evasion.hpp — operational evasion layer. keep the cleaner itself off radar while it runs.
#pragma once
#include "common.hpp"

namespace jc::evasion {

// patch ntdll!EtwEventWrite with a ret-first-byte so our process generates no ETW events.
// Vanguard's telemetry drains come from ETW. Patching ours drops the paper trail of what we did.
result disable_etw_for_self();

// stamp a file's creation/write/access times to match %WINDIR%\explorer.exe so timeline
// correlation on the forensics side flattens.
result smear_timestamps(const std::filesystem::path& p);

// write a plausible Service Control Manager event 7045 referencing a fake installer so
// post-fact log analysis stumbles on the fake first.
result plant_decoy_7045(const std::wstring& fake_service_name);

// hash check: refuse to run if vgk.sys is loaded in the current boot — SMBIOS flash while
// Vanguard watches will show in its telemetry channel before we uninstall.
bool vgk_is_loaded();

// obfuscate strings in-place (XOR). pair with decrypt_at_use at call site.
void xor_buf(uint8_t* p, size_t n, uint8_t key);

} // namespace jc::evasion
