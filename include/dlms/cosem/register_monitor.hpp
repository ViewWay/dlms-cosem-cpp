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
 * IC21 Register Monitor
 * 
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: threshold_values (array of values)
 * 3: monitored_value (structure: class_id, logical_name, attribute)
 * 4: captured_value (structure)
 * 5: capture_time (date-time)
 * 6: status (enum)
 * 7: capture_period (long-unsigned)
 */
struct MonitoredValue {
    uint16_t class_id;
    ObisCode logical_name;
    uint8_t attribute_id;
};

class RegisterMonitor : public CosemObject {
public:
    uint16_t class_id() const override { return static_cast<uint16_t>(CosemInterface::RegisterMonitor); }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "RegisterMonitor"; }

    std::vector<DlmsValue> threshold_values_;
    MonitoredValue monitored_value_{0, ObisCode(), 2};
    DlmsValue captured_value_{DlmsNone{}};
    std::vector<uint8_t> capture_time_;
    uint8_t status_ = 0;
    uint16_t capture_period_ = 0;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "threshold_values", false},
            AttributeAccess{3, "monitored_value", false},
            AttributeAccess{4, "captured_value", false},
            AttributeAccess{5, "capture_time", false},
            AttributeAccess{6, "status", false},
            AttributeAccess{7, "capture_period", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { 
            auto a = ln_.to_array(); 
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); 
        }
        if (id == 2) return make_dlms_array(threshold_values_);
        if (id == 3) {
            auto ob = monitored_value_.logical_name.to_array();
            return make_dlms_structure({
                DlmsValue(monitored_value_.class_id),
                DlmsValue(std::vector<uint8_t>(ob.begin(), ob.end())),
                DlmsValue(static_cast<int8_t>(monitored_value_.attribute_id))
            });
        }
        if (id == 4) return captured_value_;
        if (id == 5) return DlmsValue(capture_time_);
        if (id == 6) return DlmsValue(status_);
        if (id == 7) return DlmsValue(capture_period_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* arr = std::get_if<DlmsArray>(&v)) {
                threshold_values_.clear();
                for (const auto& item : arr->items) {
                    if (item) threshold_values_.push_back(*item);
                }
                return make_void_ok();
            }
        }
        if (id == 5) { if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) capture_time_ = *p; return make_void_ok(); }
        if (id == 6) { if (auto* p = std::get_if<uint8_t>(&v)) status_ = *p; return make_void_ok(); }
        if (id == 7) { if (auto* p = std::get_if<uint16_t>(&v)) capture_period_ = *p; return make_void_ok(); }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void set_monitored_value(uint16_t class_id, const ObisCode& ln, uint8_t attr_id) {
        monitored_value_ = {class_id, ln, attr_id};
    }

    void add_threshold(const DlmsValue& value) {
        threshold_values_.push_back(value);
    }

    void capture(const DlmsValue& value) {
        captured_value_ = value;
        // Update capture time (simplified)
        capture_time_ = {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0};
    }

    size_t threshold_count() const { return threshold_values_.size(); }
};

} // namespace dlms::cosem
