// validator.hpp — snapshot before / after; diff every surface; refuse to declare success without movement.
#pragma once
#include "common.hpp"

namespace jc::validate {

struct report {
    bool smbios_moved{};
    bool machine_guid_moved{};
    bool sqm_moved{};
    bool sus_moved{};
    bool hw_profile_moved{};
    size_t volume_serials_moved{};
    size_t macs_moved{};
    size_t gpu_cache_moved{};
    size_t edid_moved{};
    bool   tpm_clear_scheduled{};
    bool   vanguard_wiped{};
    std::vector<std::string> notes;
};

report run_full(const machine_snapshot& before, const machine_snapshot& after);
machine_snapshot capture();
std::string      print(const report& r);
void             export_json(const report& r, const std::filesystem::path& out);

} // namespace jc::validate
