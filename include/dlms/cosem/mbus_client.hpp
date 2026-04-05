#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>

#include <vector>
#include <string>

namespace dlms::cosem {

inline DlmsValue make_dlms_array(const std::vector<DlmsValue>& items);
inline DlmsValue make_dlms_structure(const std::vector<DlmsValue>& elements);

/**
 * IC57 M-Bus Client
 * 
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: mbus_port_reference (object)
 * 3: capture_period (long-unsigned)
 * 4: mbus_client_port (long-unsigned)
 * 5: number_of_mbus_clients (long-unsigned)
 * 6: mbus_client_list (array of structures)
 */
struct MbusSlaveInfo {
    uint8_t address;
    std::vector<uint8_t> identification_number;
    uint8_t version;
    uint8_t medium;
};

class MbusClient : public CosemObject {
public:
    uint16_t class_id() const override { return 57; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "MbusClient"; }

    ObisCode mbus_port_reference_{0, 0, 24, 1, 0, 255};
    uint16_t capture_period_ = 0;
    uint16_t mbus_client_port_ = 1;
    uint16_t number_of_mbus_clients_ = 0;
    std::vector<MbusSlaveInfo> mbus_client_list_;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "mbus_port_reference", true},
            AttributeAccess{3, "capture_period", false},
            AttributeAccess{4, "mbus_client_port", true},
            AttributeAccess{5, "number_of_mbus_clients", false},
            AttributeAccess{6, "mbus_client_list", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { 
            auto a = ln_.to_array(); 
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); 
        }
        if (id == 2) {
            auto ob = mbus_port_reference_.to_array();
            return DlmsValue(std::vector<uint8_t>(ob.begin(), ob.end()));
        }
        if (id == 3) return DlmsValue(capture_period_);
        if (id == 4) return DlmsValue(mbus_client_port_);
        if (id == 5) return DlmsValue(number_of_mbus_clients_);
        if (id == 6) {
            std::vector<DlmsValue> arr;
            for (const auto& slave : mbus_client_list_) {
                arr.push_back(make_dlms_structure({
                    DlmsValue(slave.address),
                    DlmsValue(slave.identification_number),
                    DlmsValue(slave.version),
                    DlmsValue(slave.medium)
                }));
            }
            return make_dlms_array(arr);
        }

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 3) { if (auto* p = std::get_if<uint16_t>(&v)) capture_period_ = *p; return make_void_ok(); }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void add_mbus_slave(uint8_t address, const std::vector<uint8_t>& id_number, uint8_t version, uint8_t medium) {
        mbus_client_list_.push_back({address, id_number, version, medium});
        number_of_mbus_clients_ = static_cast<uint16_t>(mbus_client_list_.size());
    }

    void clear_slaves() {
        mbus_client_list_.clear();
        number_of_mbus_clients_ = 0;
    }

    size_t slave_count() const { return mbus_client_list_.size(); }

    VoidResult scan_slaves() {
        // Simplified - would normally send M-Bus scan commands
        return make_void_ok();
    }
};

} // namespace dlms::cosem
