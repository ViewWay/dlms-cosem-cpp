#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <vector>
#include <string>

namespace dlms::cosem {

/**
 * IC51 M-Bus Master - M-Bus Master Port
 *
 * This class manages M-Bus master port functionality for DLMS/COSEM devices.
 *
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: baud_rate (unsigned)
 * 3: primary_address (unsigned)
 * 4: response_timeout (unsigned)
 */
class MbusMaster : public CosemObject {
public:
    uint16_t class_id() const override { return 51; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "MbusMaster"; }

    uint32_t baud_rate_ = 2400;
    uint8_t primary_address_ = 0;
    uint16_t response_timeout_ = 2000;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "baud_rate", true},
            AttributeAccess{3, "primary_address", true},
            AttributeAccess{4, "response_timeout", true}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) {
            auto a = ln_.to_array();
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end()));
        }
        if (id == 2) return DlmsValue(baud_rate_);
        if (id == 3) return DlmsValue(primary_address_);
        if (id == 4) return DlmsValue(response_timeout_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* p = std::get_if<uint32_t>(&v)) baud_rate_ = *p;
            return make_void_ok();
        }
        if (id == 3) {
            if (auto* p = std::get_if<uint8_t>(&v)) primary_address_ = *p;
            return make_void_ok();
        }
        if (id == 4) {
            if (auto* p = std::get_if<uint16_t>(&v)) response_timeout_ = *p;
            return make_void_ok();
        }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void set_baud_rate(uint32_t rate) { baud_rate_ = rate; }
    void set_primary_address(uint8_t addr) { primary_address_ = addr; }
};

} // namespace dlms::cosem
