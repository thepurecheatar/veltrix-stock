// smbios.cpp — reads DMTF SMBIOS, generates plan, invokes vendor flash tool.
// vendor tools (user supplies, placed in ./tools/):
//   AMIDEWINx64.exe      — AMI BIOS (most desktops / ASUS / MSI / Gigabyte / ASRock)
//   H2OSDE-Wx64-RW.exe   — Insyde H2O (most laptops: Dell / HP / Lenovo / Acer)
// we never ship the binaries — we drive whichever one is sitting next to our exe.
#include "smbios.hpp"
#include "random_gen.hpp"
#include "logger.hpp"
#include <sstream>

namespace jc::smbios {

#pragma pack(push, 1)
struct raw_sm_bios_header {
    uint8_t  used20_calling_method;
    uint8_t  major;
    uint8_t  minor;
    uint8_t  dmi_revision;
    uint32_t length;
    uint8_t  table[1];
};

struct dmi_header {
    uint8_t  type;
    uint8_t  length;
    uint16_t handle;
};
#pragma pack(pop)

static std::string read_string(const uint8_t* table_end, const uint8_t* strings, uint8_t idx) {
    if (idx == 0) return {};
    const uint8_t* p = strings;
    for (uint8_t i = 1; p < table_end && *p; ++i) {
        const uint8_t* s = p;
        while (p < table_end && *p) ++p;
        if (i == idx) return std::string((const char*)s, p - s);
        ++p;
    }
    return {};
}

std::vector<uint8_t> raw_smbios_table() {
    UINT need = GetSystemFirmwareTable('RSMB', 0, nullptr, 0);
    if (!need) return {};
    std::vector<uint8_t> buf(need);
    UINT got = GetSystemFirmwareTable('RSMB', 0, buf.data(), need);
    if (got != need) buf.clear();
    return buf;
}

snapshot read() {
    snapshot s;
    auto buf = raw_smbios_table();
    if (buf.size() < sizeof(raw_sm_bios_header)) return s;
    auto* hdr = reinterpret_cast<raw_sm_bios_header*>(buf.data());
    const uint8_t* p = hdr->table;
    const uint8_t* end = p + hdr->length;

    while (p + sizeof(dmi_header) <= end) {
        auto* d = reinterpret_cast<const dmi_header*>(p);
        if (d->length < 4) break;
        const uint8_t* body = p;
        const uint8_t* strs = p + d->length;
        const uint8_t* se = strs;
        while (se + 1 < end && !(se[0] == 0 && se[1] == 0)) ++se;
        if (se + 1 >= end) break;

        switch (d->type) {
            case 0: { // BIOS
                s.bios_vendor  = read_string(end, strs, body[4]);
                s.bios_version = read_string(end, strs, body[5]);
                s.bios_release = read_string(end, strs, body[8]);
                break;
            }
            case 1: { // System
                s.system_manufacturer = read_string(end, strs, body[4]);
                s.system_product      = read_string(end, strs, body[5]);
                s.system_serial       = read_string(end, strs, body[7]);
                char uuid[40];
                _snprintf_s(uuid, sizeof uuid, _TRUNCATE,
                    "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                    body[11],body[10],body[9],body[8],
                    body[13],body[12],body[15],body[14],
                    body[16],body[17],body[18],body[19],
                    body[20],body[21],body[22],body[23]);
                s.system_uuid = uuid;
                break;
            }
            case 2: { // Baseboard
                s.baseboard_manufacturer = read_string(end, strs, body[4]);
                s.baseboard_product      = read_string(end, strs, body[5]);
                s.baseboard_serial       = read_string(end, strs, body[7]);
                break;
            }
            case 3: { // Chassis
                s.chassis_serial    = read_string(end, strs, body[7]);
                s.chassis_asset_tag = read_string(end, strs, body[8]);
                break;
            }
        }
        p = se + 2;
    }
    return s;
}

oem detect_vendor(const snapshot& s) {
    auto has = [&](std::string_view needle, const std::string& hay) {
        std::string h = hay; for (auto& c : h) c = (char)toupper((unsigned char)c);
        std::string n(needle); for (auto& c : n) c = (char)toupper((unsigned char)c);
        return h.find(n) != std::string::npos;
    };
    if (has("DELL",      s.system_manufacturer) || has("DELL",     s.bios_vendor)) return oem::dell;
    if (has("HP",        s.system_manufacturer) || has("HEWLETT",  s.bios_vendor)) return oem::hp;
    if (has("LENOVO",    s.system_manufacturer))                                   return oem::lenovo;
    if (has("ASUS",      s.system_manufacturer) || has("ASUS",     s.baseboard_manufacturer)) return oem::asus;
    if (has("MSI",       s.system_manufacturer) || has("MICRO-STAR", s.baseboard_manufacturer)) return oem::msi;
    if (has("GIGABYTE",  s.baseboard_manufacturer))                                return oem::gigabyte;
    if (has("ASROCK",    s.baseboard_manufacturer))                                return oem::asrock;
    if (has("INSYDE",    s.bios_vendor))                                           return oem::insyde_laptop;
    return oem::unknown;
}

plan make_plan(const snapshot& s) {
    plan p;
    p.vendor          = detect_vendor(s);
    p.system_uuid     = rng::gen_system_uuid();
    p.system_serial   = (p.vendor == oem::dell) ? rng::gen_dell_svctag() : rng::gen_serial(12);
    p.baseboard_serial = (p.vendor == oem::asus) ? rng::gen_asus_board_serial() : rng::gen_serial(16);
    p.chassis_serial  = rng::gen_serial(12);
    p.asset_tag       = rng::gen_serial(10);
    return p;
}

std::filesystem::path find_vendor_tool(oem v) {
    namespace fs = std::filesystem;
    fs::path here = fs::current_path() / "tools";
    auto try_names = [&](std::initializer_list<const char*> names) -> fs::path {
        for (auto n : names) {
            fs::path p = here / n;
            if (fs::exists(p)) return p;
        }
        return {};
    };
    switch (v) {
        case oem::dell:
        case oem::hp:
        case oem::lenovo:
        case oem::insyde_laptop:
            return try_names({ "H2OSDE-Wx64-RW.exe", "H2OSDE.exe", "InsydeSetupInterface.exe" });
        default:
            return try_names({ "AMIDEWINx64.EXE", "AMIDEWINx64.exe", "AMIDEWINx32.EXE" });
    }
}

// pack a /SS, /SU, /BS invocation for AMIDEWIN
static int run_tool(const std::filesystem::path& tool, const std::vector<std::string>& args) {
    std::string cmd = "\"" + tool.string() + "\"";
    for (auto& a : args) { cmd += ' '; cmd += '"'; cmd += a; cmd += '"'; }
    JC_LOG(trace, "exec: %s", cmd.c_str());

    STARTUPINFOA si{sizeof si};
    PROCESS_INFORMATION pi{};
    std::string mut = cmd; // CreateProcess wants writable
    si.dwFlags    = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    if (!CreateProcessA(nullptr, mut.data(), nullptr, nullptr, FALSE,
                        CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        return -1;
    }
    WaitForSingleObject(pi.hProcess, 120000);
    DWORD ec = 0;
    GetExitCodeProcess(pi.hProcess, &ec);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return (int)ec;
}

result apply(const plan& p) {
    auto tool = find_vendor_tool(p.vendor);
    if (tool.empty()) {
        return fail(status::firmware_locked,
            "no vendor flash tool next to exe; drop AMIDEWINx64.exe (AMI) or "
            "H2OSDE-Wx64-RW.exe (Insyde) in ./tools/ and re-run");
    }
    bool ami = tool.filename().string().find("AMIDE") != std::string::npos;

    int rc = 0;
    if (ami) {
        rc |= run_tool(tool, {"/SS", p.system_serial});
        rc |= run_tool(tool, {"/SU", p.system_uuid});
        rc |= run_tool(tool, {"/BS", p.baseboard_serial});
        rc |= run_tool(tool, {"/CS", p.chassis_serial});
        rc |= run_tool(tool, {"/CT", p.asset_tag});
    } else {
        // Insyde H2OSDE uses /sm /ss /sv /sku syntax. keep conservative set.
        rc |= run_tool(tool, {"/ss", p.system_serial});
        rc |= run_tool(tool, {"/su", p.system_uuid});
        rc |= run_tool(tool, {"/bs", p.baseboard_serial});
    }
    if (rc != 0) {
        return fail(status::firmware_locked,
            std::string("vendor tool returned ") + std::to_string(rc) + " — BIOS SMBIOS lock may be engaged; disable Secure/BIOS Lock in firmware setup, then retry");
    }
    return ok();
}

result verify(const plan& p) {
    auto post = read();
    auto norm = [](std::string s) {
        for (auto& c : s) c = (char)toupper((unsigned char)c);
        return s;
    };
    if (norm(post.system_serial)     != norm(p.system_serial))
        return fail(status::partial, "system serial did not stick");
    if (norm(post.system_uuid)       != norm(p.system_uuid))
        return fail(status::partial, "system UUID did not stick");
    if (norm(post.baseboard_serial)  != norm(p.baseboard_serial))
        return fail(status::partial, "baseboard serial did not stick");
    return ok();
}

std::string dump(const snapshot& s) {
    std::ostringstream o;
    o << "bios="<<s.bios_vendor<<"/"<<s.bios_version
      << " sys="<<s.system_manufacturer<<"/"<<s.system_product
      << " sys_serial="<<s.system_serial
      << " sys_uuid="<<s.system_uuid
      << " board="<<s.baseboard_manufacturer<<"/"<<s.baseboard_product
      << " board_serial="<<s.baseboard_serial
      << " chassis_serial="<<s.chassis_serial;
    return o.str();
}

} // namespace jc::smbios
