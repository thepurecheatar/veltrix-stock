// random_gen.cpp — impl. BCryptGenRandom backs everything; no rand(). C++17.
#include "random_gen.hpp"
#include <bcrypt.h>
#pragma comment(lib, "Bcrypt.lib")

namespace jc::rng {

void fill(void* dst, size_t n) {
    NTSTATUS s = BCryptGenRandom(nullptr, (PUCHAR)dst, (ULONG)n, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (s < 0) {
        // fallback — never hit in practice, but never ship something that can zero-fill a serial.
        for (size_t i = 0; i < n; ++i) ((uint8_t*)dst)[i] = (uint8_t)(__rdtsc() ^ (i * 1103515245));
    }
}

uint64_t u64() { uint64_t v; fill(&v, 8); return v; }
uint32_t u32() { uint32_t v; fill(&v, 4); return v; }
uint8_t  u8()  { uint8_t v;  fill(&v, 1); return v; }

static char alnum_at(uint8_t b) {
    static const char tbl[] = "0123456789ABCDEFGHJKLMNPQRSTUVWXYZ";
    return tbl[b % (sizeof(tbl) - 1)];
}

std::string gen_serial(size_t len) {
    std::string s(len, '0');
    for (size_t i = 0; i < len; ++i) s[i] = alnum_at(u8());
    return s;
}

std::string gen_system_uuid() {
    uint8_t b[16];
    fill(b, 16);
    // DMTF v4 random. SMBIOS stores little-endian first three fields but the string form is BE-canonical.
    b[6] = (b[6] & 0x0F) | 0x40;
    b[8] = (b[8] & 0x3F) | 0x80;
    char out[37];
    _snprintf_s(out, sizeof out, _TRUNCATE,
        "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        b[0],b[1],b[2],b[3],b[4],b[5],b[6],b[7],b[8],b[9],b[10],b[11],b[12],b[13],b[14],b[15]);
    return out;
}

std::string gen_dell_svctag()       { return gen_serial(7); }
std::string gen_asus_board_serial() { return std::string("MT7") + gen_serial(10); }
std::string gen_nvme_serial()       { return gen_serial(20); }
std::string gen_sata_serial()       { return std::string("WD-WCC") + gen_serial(10); }

std::string gen_mac_oui(uint32_t oui) {
    uint8_t m[6];
    m[0] = (oui >> 16) & 0xFF;
    m[1] = (oui >>  8) & 0xFF;
    m[2] = (oui      ) & 0xFF;
    fill(m + 3, 3);
    char buf[18];
    _snprintf_s(buf, sizeof buf, _TRUNCATE, "%02X%02X%02X%02X%02X%02X",
                m[0],m[1],m[2],m[3],m[4],m[5]);
    return buf;
}

std::string gen_mac_plausible() {
    // consumer OUIs: intel, realtek, dell, asus, mediatek, qualcomm. rotate.
    static const uint32_t ouis[] = {
        0x001B21, 0x6CEC4B, 0x002248, 0x4CEDFB, 0x18F0E4,
        0xA4C3F0, 0x9C7BEF, 0x00E04C, 0x3C7C3F, 0xD05099,
    };
    uint32_t pick = ouis[u32() % (sizeof(ouis)/sizeof(ouis[0]))];
    return gen_mac_oui(pick);
}

uint32_t gen_volume_id() { return u32() | 0x00000001u; } // avoid zero

GUID gen_guid() {
    GUID g;
    fill(&g, sizeof g);
    g.Data3 = (g.Data3 & 0x0FFF) | 0x4000;
    g.Data4[0] = (g.Data4[0] & 0x3F) | 0x80;
    return g;
}

// EDID: 128-byte block. header + mfr + product + checksum.
std::vector<uint8_t> gen_edid_block(uint16_t vendor_pnp, uint16_t product) {
    std::vector<uint8_t> e(128, 0);
    static const uint8_t header[8] = { 0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00 };
    std::memcpy(e.data(), header, 8);
    e[8]  = (vendor_pnp >> 8) & 0xFF;
    e[9]  = vendor_pnp & 0xFF;
    e[10] = product & 0xFF;
    e[11] = (product >> 8) & 0xFF;
    uint32_t serial = u32();
    e[12] = serial & 0xFF; e[13] = (serial>>8)&0xFF;
    e[14] = (serial>>16)&0xFF; e[15] = (serial>>24)&0xFF;
    e[16] = (uint8_t)(1 + (u8() % 53)); // mfg week
    e[17] = (uint8_t)(28 + (u8() % 6)); // mfg year since 1990 → 2018..2023
    e[18] = 1; e[19] = 4; // EDID 1.4
    // fill remainder with plausible junk, then checksum
    for (size_t i = 20; i < 127; ++i) e[i] = (uint8_t)(u8() & 0x7F);
    uint8_t sum = 0;
    for (size_t i = 0; i < 127; ++i) sum += e[i];
    e[127] = (uint8_t)(256 - sum);
    return e;
}

} // namespace jc::rng

namespace jc {

std::string to_hex(const uint8_t* p, size_t n) {
    static const char* h = "0123456789abcdef";
    std::string s(n * 2, '0');
    for (size_t i = 0; i < n; ++i) { s[2*i] = h[p[i] >> 4]; s[2*i+1] = h[p[i] & 0xF]; }
    return s;
}

std::string guid_to_string(const GUID& g) {
    char b[40];
    _snprintf_s(b, sizeof b, _TRUNCATE,
        "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        g.Data1, g.Data2, g.Data3,
        g.Data4[0],g.Data4[1],g.Data4[2],g.Data4[3],
        g.Data4[4],g.Data4[5],g.Data4[6],g.Data4[7]);
    return b;
}

GUID new_guid() { return rng::gen_guid(); }

std::wstring widen(std::string_view s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}

std::string narrow(std::wstring_view s) {
    if (s.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string r(n, 0);
    WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), r.data(), n, nullptr, nullptr);
    return r;
}

} // namespace jc
