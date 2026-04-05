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
 * IC30 Limiter
 * 
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: monitored_value (structure: class_id, logical_name, attribute)
 * 3: threshold_active (value)
 * 4: threshold_normal (value)
 * 5: threshold_emergency (value)
 * 6: min_over_threshold_duration (long-unsigned)
 * 7: min_under_threshold_duration (long-unsigned)
 * 8: emergency_profile (structure)
 * 9: emergency_profile_group (array)
 * 10: emergency_profile_active (boolean)
 * 11: actions (array of two action sets)
 */
struct LimiterMonitoredValue {
    uint16_t class_id;
    ObisCode logical_name;
    uint8_t attribute_id;
};

class Limiter : public CosemObject {
public:
    uint16_t class_id() const override { return 30; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "Limiter"; }

    LimiterMonitoredValue monitored_value_{0, ObisCode(), 2};
    DlmsValue threshold_active_{static_cast<int32_t>(0)};
    DlmsValue threshold_normal_{static_cast<int32_t>(0)};
    DlmsValue threshold_emergency_{static_cast<int32_t>(0)};
    uint16_t min_over_threshold_duration_ = 0;
    uint16_t min_under_threshold_duration_ = 0;
    bool emergency_profile_active_ = false;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "monitored_value", false},
            AttributeAccess{3, "threshold_active", false},
            AttributeAccess{4, "threshold_normal", false},
            AttributeAccess{5, "threshold_emergency", false},
            AttributeAccess{6, "min_over_threshold_duration", false},
            AttributeAccess{7, "min_under_threshold_duration", false},
            AttributeAccess{8, "emergency_profile", false},
            AttributeAccess{9, "emergency_profile_group", false},
            AttributeAccess{10, "emergency_profile_active", false},
            AttributeAccess{11, "actions", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { 
            auto a = ln_.to_array(); 
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); 
        }
        if (id == 2) {
            auto ob = monitored_value_.logical_name.to_array();
            return make_dlms_structure({
                DlmsValue(monitored_value_.class_id),
                DlmsValue(std::vector<uint8_t>(ob.begin(), ob.end())),
                DlmsValue(static_cast<int8_t>(monitored_value_.attribute_id))
            });
        }
        if (id == 3) return threshold_active_;
        if (id == 4) return threshold_normal_;
        if (id == 5) return threshold_emergency_;
        if (id == 6) return DlmsValue(min_over_threshold_duration_);
        if (id == 7) return DlmsValue(min_under_threshold_duration_);
        if (id == 10) return DlmsValue(emergency_profile_active_);

        // For complex attributes, return empty
        if (id == 8 || id == 9 || id == 11) {
            return make_dlms_array(std::vector<DlmsValue>{});
        }

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 3) { threshold_active_ = v; return make_void_ok(); }
        if (id == 4) { threshold_normal_ = v; return make_void_ok(); }
        if (id == 5) { threshold_emergency_ = v; return make_void_ok(); }
        if (id == 6) { if (auto* p = std::get_if<uint16_t>(&v)) min_over_threshold_duration_ = *p; return make_void_ok(); }
        if (id == 7) { if (auto* p = std::get_if<uint16_t>(&v)) min_under_threshold_duration_ = *p; return make_void_ok(); }
        if (id == 10) { if (auto* p = std::get_if<bool>(&v)) emergency_profile_active_ = *p; return make_void_ok(); }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void set_monitored_value(uint16_t class_id, const ObisCode& ln, uint8_t attr_id) {
        monitored_value_ = {class_id, ln, attr_id};
    }

    bool is_over_threshold(const DlmsValue& current_value) const {
        // Simplified check - compare as int32
        if (auto* current = std::get_if<int32_t>(&current_value)) {
            if (auto* threshold = std::get_if<int32_t>(&threshold_active_)) {
                return *current > *threshold;
            }
        }
        return false;
    }
};

} // namespace dlms::cosem
