// gpu_edid.cpp — enumerates Enum\DISPLAY\* subkeys, replaces EDID binary, refreshes device.
#include "gpu_edid.hpp"
#include "random_gen.hpp"
#include "logger.hpp"
#include <bcrypt.h>
#include <sstream>
#pragma comment(lib, "Bcrypt.lib")

namespace jc::gpu {

static std::string sha256_hex(const uint8_t* p, size_t n) {
    BCRYPT_ALG_HANDLE alg{}; BCRYPT_HASH_HANDLE h{};
    BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    BCryptCreateHash(alg, &h, nullptr, 0, nullptr, 0, 0);
    BCryptHashData(h, (PUCHAR)p, (ULONG)n, 0);
    uint8_t out[32];
    BCryptFinishHash(h, out, 32, 0);
    BCryptDestroyHash(h);
    BCryptCloseAlgorithmProvider(alg, 0);
    return to_hex(out, 32);
}

static std::vector<uint8_t> read_bin(HKEY root, const wchar_t* path, const wchar_t* name) {
    HKEY k{};
    if (RegOpenKeyExW(root, path, 0, KEY_QUERY_VALUE, &k) != ERROR_SUCCESS) return {};
    DWORD type = 0, sz = 0;
    RegQueryValueExW(k, name, nullptr, &type, nullptr, &sz);
    std::vector<uint8_t> v(sz);
    if (sz) RegQueryValueExW(k, name, nullptr, &type, v.data(), &sz);
    RegCloseKey(k);
    return v;
}

static bool write_bin(HKEY root, const wchar_t* path, const wchar_t* name, const std::vector<uint8_t>& v) {
    HKEY k{};
    if (RegOpenKeyExW(root, path, 0, KEY_SET_VALUE, &k) != ERROR_SUCCESS) return false;
    LSTATUS s = RegSetValueExW(k, name, 0, REG_BINARY, v.data(), (DWORD)v.size());
    RegCloseKey(k);
    return s == ERROR_SUCCESS;
}

std::vector<monitor> enum_monitors() {
    std::vector<monitor> out;
    HKEY disp{};
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY",
                      0, KEY_READ, &disp) != ERROR_SUCCESS)
        return out;

    for (DWORD i = 0; ; ++i) {
        wchar_t mfr[64]{}; DWORD ml = 64;
        if (RegEnumKeyExW(disp, i, mfr, &ml, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
            break;
        HKEY mfrk{};
        if (RegOpenKeyExW(disp, mfr, 0, KEY_READ, &mfrk) != ERROR_SUCCESS) continue;
        for (DWORD j = 0; ; ++j) {
            wchar_t inst[128]{}; DWORD il = 128;
            if (RegEnumKeyExW(mfrk, j, inst, &il, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
                break;
            std::wstring sub = std::wstring(L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY\\") + mfr + L"\\" + inst + L"\\Device Parameters";
            auto e = read_bin(HKEY_LOCAL_MACHINE, sub.c_str(), L"EDID");
            if (!e.empty()) {
                monitor m;
                m.device_instance_id = std::wstring(L"DISPLAY\\") + mfr + L"\\" + inst;
                m.edid = e;
                m.edid_sha256 = sha256_hex(e.data(), e.size());
                out.push_back(m);
            }
        }
        RegCloseKey(mfrk);
    }
    RegCloseKey(disp);
    return out;
}

result rotate_monitor_edid() {
    auto mons = enum_monitors();
    if (mons.empty()) return fail(status::partial, "no monitor EDID blobs found");
    size_t changed = 0;
    for (auto& m : mons) {
        uint16_t vendor = (uint16_t)(rng::u32() & 0x7FFF);
        uint16_t product = (uint16_t)rng::u32();
        auto fresh = rng::gen_edid_block(vendor, product);
        std::wstring sub = L"SYSTEM\\CurrentControlSet\\Enum\\" + m.device_instance_id + L"\\Device Parameters";
        if (write_bin(HKEY_LOCAL_MACHINE, sub.c_str(), L"EDID", fresh)) {
            JC_LOG(ok, "edid: %ls was=%s new=%s",
                   m.device_instance_id.c_str(),
                   m.edid_sha256.c_str(),
                   sha256_hex(fresh.data(), fresh.size()).c_str());
            ++changed;
        } else {
            JC_LOG(warn, "edid: write denied on %ls — ACL via SYSTEM required", m.device_instance_id.c_str());
        }
    }
    if (!changed) return fail(status::denied_privilege, "EDID writes denied — run as TrustedInstaller or psexec -s");
    return ok();
}

std::vector<gpu_adapter> enum_gpus() {
    std::vector<gpu_adapter> out;
    HDEVINFO dev = SetupDiGetClassDevsW(&GUID_DEVCLASS_DISPLAY, nullptr, nullptr, DIGCF_PRESENT);
    if (dev == INVALID_HANDLE_VALUE) return out;
    SP_DEVINFO_DATA did{sizeof did};
    for (DWORD i = 0; SetupDiEnumDeviceInfo(dev, i, &did); ++i) {
        wchar_t iid[512]{}; DWORD sz = sizeof iid;
        if (!SetupDiGetDeviceInstanceIdW(dev, &did, iid, sz, &sz)) continue;
        gpu_adapter g;
        g.device_instance_id = iid;
        wchar_t desc[256]{}; sz = sizeof desc;
        if (SetupDiGetDeviceRegistryPropertyW(dev, &did, SPDRP_DEVICEDESC, nullptr, (PBYTE)desc, sz, &sz))
            g.model = narrow(desc);
        wchar_t drv[64]{}; sz = sizeof drv;
        if (SetupDiGetDeviceRegistryPropertyW(dev, &did, SPDRP_DRIVER, nullptr, (PBYTE)drv, sz, &sz))
            g.driver_subkey = drv;
        out.push_back(g);
    }
    SetupDiDestroyDeviceInfoList(dev);
    return out;
}

result rotate_gpu_registry_ids() {
    // nvidia + amd leave cached UUID strings in driver subkeys. rewrite them.
    auto gpus = enum_gpus();
    size_t changed = 0;
    for (auto& g : gpus) {
        if (g.driver_subkey.empty()) continue;
        std::wstring key = L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\" + g.driver_subkey;
        HKEY k{};
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &k) != ERROR_SUCCESS)
            continue;
        // NVIDIA: "GPUID", "GpuUUID", "GPU_UUID"; AMD: "AdapterNumber", "DeviceUUID"
        const wchar_t* nv_names[] = { L"GPUID", L"GpuUUID", L"GPU_UUID", L"DeviceUUID", L"AdapterUUID" };
        auto g2 = guid_to_string(rng::gen_guid());
        std::wstring wg2 = widen(g2);
        for (auto n : nv_names) {
            RegSetValueExW(k, n, 0, REG_SZ, (const BYTE*)wg2.c_str(),
                           (DWORD)((wg2.size()+1)*sizeof(wchar_t)));
        }
        RegCloseKey(k);
        JC_LOG(ok, "gpu: rewrote UUID cache in %ls -> %s", g.driver_subkey.c_str(), g2.c_str());
        ++changed;
    }
    if (!changed) return fail(status::partial, "no GPU subkeys rewritten");
    return ok();
}

std::string dump(const gpu_adapter& g) {
    std::ostringstream o;
    o << "gpu " << g.model << " inst=" << narrow(g.device_instance_id);
    return o.str();
}

std::string dump(const monitor& m) {
    std::ostringstream o;
    o << "monitor " << narrow(m.device_instance_id)
      << " edid_sha=" << m.edid_sha256;
    return o.str();
}

} // namespace jc::gpu
