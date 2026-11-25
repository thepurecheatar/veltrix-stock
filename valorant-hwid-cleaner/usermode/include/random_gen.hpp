// random_gen.hpp — cryptographic rng, vendor-plausible ID generators. C++17.
#pragma once
#include "common.hpp"

namespace jc::rng {

void fill(void* dst, size_t n);
uint64_t u64();
uint32_t u32();
uint8_t  u8();

// IDs that look vendor-real. detection surfaces complain when strings are obviously synthetic.
std::string gen_system_uuid();              // SMBIOS type 1 UUID (lower-byte-ordered per DMTF)
std::string gen_serial(size_t len);         // uppercase alnum, OEM style
std::string gen_dell_svctag();              // 7-char dell service tag
std::string gen_asus_board_serial();        // ASUS-style baseboard
std::string gen_nvme_serial();              // 20-char nvme serial
std::string gen_sata_serial();              // "WD-..." or "S4EVNX0..." style
std::string gen_mac_oui(uint32_t oui);      // fixed OUI, random NIC
std::string gen_mac_plausible();            // random valid consumer OUI
uint32_t    gen_volume_id();                // 32-bit volume serial
GUID        gen_guid();
std::vector<uint8_t> gen_edid_block(uint16_t vendor_pnp, uint16_t product);

} // namespace jc::rng
