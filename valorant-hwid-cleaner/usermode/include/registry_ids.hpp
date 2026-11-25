// registry_ids.hpp — rotate every machine-identity GUID Vanguard reads from registry.
#pragma once
#include "common.hpp"

namespace jc::regids {

struct snapshot {
    std::string machine_guid;
    std::string sqm_machine_id;
    std::string sus_client_id;
    std::string sus_client_id_validation;
    std::string hw_profile_guid;
    std::string windows_install_id;
    std::string device_metadata_id;
    std::string device_token;
};

snapshot read();
result   rotate(snapshot* new_values = nullptr);
std::string dump(const snapshot& s);

} // namespace jc::regids
