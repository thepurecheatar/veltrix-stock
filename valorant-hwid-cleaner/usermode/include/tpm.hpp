// tpm.hpp — TBS clear path. Vanguard reads TPM 2.0 EK public via TBSi_Submit_Command (TPM2_ReadPublic).
#pragma once
#include "common.hpp"

namespace jc::tpm {

struct snapshot {
    bool        present{};
    bool        ready{};
    uint32_t    manufacturer{};
    std::string ek_pub_sha256; // hex
};

snapshot read();
// request a clear via Tbsi_Physical_Presence SetOperation(5). Takes effect after reboot + firmware prompt.
result   request_clear_via_ppi();
// alternative: Tbsi_Create_Attestation_Identity path to zero EK cache — only works pre-ownership.
result   try_evict_ek_handles();

std::string dump(const snapshot& s);

} // namespace jc::tpm
