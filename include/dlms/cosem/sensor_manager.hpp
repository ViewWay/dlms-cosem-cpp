#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <dlms/cosem/association_ln.hpp>
#include <vector>
#include <string>
#include <memory>

namespace dlms::cosem {

/**
 * IC67 Sensor Manager - Sensor Device Manager
 *
 * This class manages sensor devices in a DLMS/COSEM system.
 *
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: sensor_list (array of structures)
 *
 * Methods:
 * 1: add_sensor
 * 2: remove_sensor
 * 3: get_sensor_info
 */

class SensorManager : public CosemObject {
public:
    uint16_t class_id() const override { return 67; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "SensorManager"; }

    struct SensorEntry {
        ObisCode sensor_reference;
        uint8_t sensor_type;
    };

    std::vector<SensorEntry> sensor_list_;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "sensor_list", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) {
            auto a = ln_.to_array();
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end()));
        }
        if (id == 2) {
            std::vector<DlmsValue> arr;
            for (const auto& sensor : sensor_list_) {
                auto ref = sensor.sensor_reference.to_array();
                arr.push_back(make_dlms_structure({
                    DlmsValue(std::vector<uint8_t>(ref.begin(), ref.end())),
                    DlmsValue(sensor.sensor_type)
                }));
            }
            return make_dlms_array(arr);
        }

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void add_sensor(const SensorEntry& entry) { sensor_list_.push_back(entry); }
    bool remove_sensor(size_t index) {
        if (index < sensor_list_.size()) {
            sensor_list_.erase(sensor_list_.begin() + index);
            return true;
        }
        return false;
    }
    const std::vector<SensorEntry>& get_sensor_list() const { return sensor_list_; }
};

} // namespace dlms::cosem
