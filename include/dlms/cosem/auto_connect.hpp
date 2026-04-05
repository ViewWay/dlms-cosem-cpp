#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <vector>
#include <string>

namespace dlms::cosem {

/**
 * IC18 Auto Connect - Automatic Connection Management
 *
 * This class manages automatic connection configuration for DLMS/COSEM devices.
 *
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: mode (enum)
 * 3: calling_interval (unsigned)
 * 4: repetition_delay (unsigned)
 * 5: repetition_count (unsigned)
 */
class AutoConnect : public CosemObject {
public:
    uint16_t class_id() const override { return 18; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "AutoConnect"; }

    enum class Mode : uint8_t {
        Disabled = 0,
        Enabled = 1,
        Auto = 2
    };

    Mode mode_ = Mode::Disabled;
    uint16_t calling_interval_ = 3600;
    uint16_t repetition_delay_ = 30;
    uint8_t repetition_count_ = 3;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "mode", true},
            AttributeAccess{3, "calling_interval", true},
            AttributeAccess{4, "repetition_delay", true},
            AttributeAccess{5, "repetition_count", true}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) {
            auto a = ln_.to_array();
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end()));
        }
        if (id == 2) return DlmsValue(static_cast<uint8_t>(mode_));
        if (id == 3) return DlmsValue(calling_interval_);
        if (id == 4) return DlmsValue(repetition_delay_);
        if (id == 5) return DlmsValue(repetition_count_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* p = std::get_if<uint8_t>(&v)) mode_ = static_cast<Mode>(*p);
            return make_void_ok();
        }
        if (id == 3) {
            if (auto* p = std::get_if<uint16_t>(&v)) calling_interval_ = *p;
            return make_void_ok();
        }
        if (id == 4) {
            if (auto* p = std::get_if<uint16_t>(&v)) repetition_delay_ = *p;
            return make_void_ok();
        }
        if (id == 5) {
            if (auto* p = std::get_if<uint8_t>(&v)) repetition_count_ = *p;
            return make_void_ok();
        }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void set_mode(Mode mode) { mode_ = mode; }
    bool is_enabled() const { return mode_ != Mode::Disabled; }
};

} // namespace dlms::cosem
