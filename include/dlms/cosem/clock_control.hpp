#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <vector>
#include <string>

namespace dlms::cosem {

/**
 * IC28 Clock Control - DLMS Clock Control Object
 *
 * This class provides control over the clock in a DLMS/COSEM device.
 * Used to synchronize and control timing functions.
 *
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: control_mode (enum)
 * 3: adjustment_time (datetime)
 * 4: adjustment_source (unsigned)
 * 5: adjustment_status (unsigned)
 * 6: adjustment_status2 (unsigned)
 *
 * Methods:
 * 1: adjust_time - adjust the clock
 */
class ClockControl : public CosemObject {
public:
    uint16_t class_id() const override { return 28; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "ClockControl"; }

    enum class ControlMode : uint8_t {
        Manual = 0,
        Automatic = 1
    };

    ControlMode control_mode_ = ControlMode::Manual;
    uint8_t adjustment_source_ = 0;
    uint8_t adjustment_status_ = 0;
    uint8_t adjustment_status2_ = 0;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "control_mode", true},
            AttributeAccess{3, "adjustment_source", true},
            AttributeAccess{4, "adjustment_status", false},
            AttributeAccess{5, "adjustment_status2", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) {
            auto a = ln_.to_array();
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end()));
        }
        if (id == 2) return DlmsValue(static_cast<uint8_t>(control_mode_));
        if (id == 3) return DlmsValue(adjustment_source_);
        if (id == 4) return DlmsValue(adjustment_status_);
        if (id == 5) return DlmsValue(adjustment_status2_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* p = std::get_if<uint8_t>(&v)) control_mode_ = static_cast<ControlMode>(*p);
            return make_void_ok();
        }
        if (id == 3) {
            if (auto* p = std::get_if<uint8_t>(&v)) adjustment_source_ = *p;
            return make_void_ok();
        }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    ControlMode get_control_mode() const { return control_mode_; }
    void set_control_mode(ControlMode mode) { control_mode_ = mode; }
    uint8_t get_adjustment_source() const { return adjustment_source_; }
    uint8_t get_adjustment_status() const { return adjustment_status_; }
};

} // namespace dlms::cosem
