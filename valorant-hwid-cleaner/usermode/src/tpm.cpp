// tpm.cpp — talks to Trusted Platform Module via Tbs.dll. real TPM2 commands.
#include "tpm.hpp"
#include "logger.hpp"
#include <bcrypt.h>
#pragma comment(lib, "Bcrypt.lib")

namespace jc::tpm {

// TPM2 command tags
constexpr uint16_t TPM_ST_NO_SESSIONS = 0x8001;
constexpr uint32_t TPM_CC_ReadPublic  = 0x00000173;
constexpr uint32_t TPM_RH_ENDORSEMENT = 0x4000000B;
constexpr uint32_t TPM2_EK_HANDLE     = 0x81010001; // standard EK persistent handle

static void w16(std::vector<uint8_t>& b, uint16_t v) { b.push_back(v>>8); b.push_back(v&0xFF); }
static void w32(std::vector<uint8_t>& b, uint32_t v) { b.push_back(v>>24); b.push_back((v>>16)&0xFF); b.push_back((v>>8)&0xFF); b.push_back(v&0xFF); }

static std::vector<uint8_t> build_read_public(uint32_t handle) {
    std::vector<uint8_t> body;
    w32(body, handle);
    std::vector<uint8_t> cmd;
    w16(cmd, TPM_ST_NO_SESSIONS);
    w32(cmd, (uint32_t)(10 + body.size())); // header is 10
    w32(cmd, TPM_CC_ReadPublic);
    cmd.insert(cmd.end(), body.begin(), body.end());
    return cmd;
}

snapshot read() {
    snapshot s;
    TBS_HCONTEXT ctx{};
    TBS_CONTEXT_PARAMS2 p{};
    p.version = TBS_CONTEXT_VERSION_TWO;
    p.includeTpm20 = 1;
    if (Tbsi_Context_Create((PCTBS_CONTEXT_PARAMS)&p, &ctx) != TBS_SUCCESS) return s;
    s.present = true;

    TPM_DEVICE_INFO di{};
    if (Tbsi_GetDeviceInfo(sizeof di, &di) == TBS_SUCCESS) {
        s.ready = (di.structVersion >= 1);
        s.manufacturer = di.tpmVersion;
    }

    auto cmd = build_read_public(TPM2_EK_HANDLE);
    std::vector<uint8_t> resp(4096, 0);
    uint32_t rlen = (uint32_t)resp.size();
    auto r = Tbsip_Submit_Command(ctx, TBS_COMMAND_LOCALITY_ZERO, TBS_COMMAND_PRIORITY_NORMAL,
                                  cmd.data(), (uint32_t)cmd.size(), resp.data(), &rlen);
    if (r == TBS_SUCCESS && rlen > 10) {
        // hash whole body — stable identifier of EK pub
        uint8_t h[32];
        BCRYPT_ALG_HANDLE alg{};
        BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
        BCRYPT_HASH_HANDLE hh{};
        BCryptCreateHash(alg, &hh, nullptr, 0, nullptr, 0, 0);
        BCryptHashData(hh, resp.data() + 10, rlen - 10, 0);
        BCryptFinishHash(hh, h, sizeof h, 0);
        BCryptDestroyHash(hh);
        BCryptCloseAlgorithmProvider(alg, 0);
        s.ek_pub_sha256 = to_hex(h, 32);
    }
    Tbsip_Context_Close(ctx);
    return s;
}

result request_clear_via_ppi() {
    TBS_HCONTEXT ctx{};
    TBS_CONTEXT_PARAMS2 p{};
    p.version = TBS_CONTEXT_VERSION_TWO;
    p.includeTpm20 = 1;
    if (Tbsi_Context_Create((PCTBS_CONTEXT_PARAMS)&p, &ctx) != TBS_SUCCESS)
        return fail(status::api_fail, "Tbsi_Context_Create failed");
    // TCG Physical Presence Interface: op 14 = Clear + Enable + Activate.
    // Request is a little-endian UINT32 op code in pabInput.
    UINT32 op = 14;
    UINT32 resp = 0; UINT32 resp_sz = sizeof resp;
    auto r = Tbsi_Physical_Presence_Command(
        ctx, (PCBYTE)&op, sizeof op, (PBYTE)&resp, &resp_sz);
    Tbsip_Context_Close(ctx);
    if (r != TBS_SUCCESS)
        return fail(status::api_fail, "Tbsi_Physical_Presence_Command failed — may need admin + TPM ready state");
    JC_LOG(ok, "tpm: clear requested via PPI op 14 — will prompt at next boot");
    return ok();
}

result try_evict_ek_handles() {
    // Nothing we can do from userland without an auth session; left as a stub for the driver
    // to issue TPM2_EvictControl against TPM_RH_PLATFORM where supported.
    return fail(status::skipped, "evict needs driver-side auth session");
}

std::string dump(const snapshot& s) {
    std::string o = "tpm present=";
    o += (s.present ? "yes" : "no");
    if (s.present) {
        o += " ready=" + std::string(s.ready ? "yes" : "no");
        o += " ek_pub_sha256=" + (s.ek_pub_sha256.empty() ? std::string("<n/a>") : s.ek_pub_sha256);
    }
    return o;
}

} // namespace jc::tpm
