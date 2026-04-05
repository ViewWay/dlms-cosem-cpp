#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <dlms/core/datetime.hpp>
#include <vector>
#include <string>

namespace dlms::cosem {

/**
 * IC9 Maximum Demand - Maximum Demand Register
 *
 * This class records maximum demand values in a DLMS/COSEM system.
 *
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: value (double-long-unsigned)
 * 3: start_time (datetime)
 * 4: duration (unsigned)
 * 5: status (unsigned)
 */
class MaximumDemand : public CosemObject {
public:
    uint16_t class_id() const override { return 9; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "MaximumDemand"; }

    uint64_t value_ = 0;
    CosemDateTime start_time_{};
    uint16_t duration_ = 0;
    uint8_t status_ = 0;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "value", false},
            AttributeAccess{3, "start_time", false},
            AttributeAccess{4, "duration", false},
            AttributeAccess{5, "status", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) {
            auto a = ln_.to_array();
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end()));
        }
        if (id == 2) return DlmsValue(value_);
        if (id == 3) {
            uint8_t b[12];
            start_time_.to_bytes(b);
            return DlmsValue(std::vector<uint8_t>(b, b+12));
        }
        if (id == 4) return DlmsValue(duration_);
        if (id == 5) return DlmsValue(status_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* p = std::get_if<uint64_t>(&v)) value_ = *p;
            return make_void_ok();
        }
        if (id == 3) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v); p && p->size()==12)
                start_time_ = CosemDateTime::from_bytes(p->data());
            return make_void_ok();
        }
        if (id == 4) {
            if (auto* p = std::get_if<uint16_t>(&v)) duration_ = *p;
            return make_void_ok();
        }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    uint64_t get_value() const { return value_; }
    void set_value(uint64_t val) { value_ = val; }
};

} // namespace dlms::cosem
