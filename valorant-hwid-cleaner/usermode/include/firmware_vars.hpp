// firmware_vars.hpp — UEFI runtime variable rotation + BCD options.
// Some Vanguard checks pull entropy from UEFI boot variables; rotating them costs nothing.
#pragma once
#include "common.hpp"

namespace jc::fwvar {

result enable_privilege();                   // SeSystemEnvironmentPrivilege
result read_var(const std::wstring& name, const GUID& ns, std::vector<uint8_t>& out);
result write_var(const std::wstring& name, const GUID& ns, const std::vector<uint8_t>& v);

// rotate the handful of vars that hold machine-identifying blobs under the global UEFI namespace.
// hostnames / Mok lists / platform strings. we don't touch Secure Boot keys.
result rotate_identity_vars();

// optional: flip BCD to allow unsigned drivers. only enable if --bcd-unlock. breaks HVCI.
result enable_test_signing();
result disable_test_signing();

} // namespace jc::fwvar
