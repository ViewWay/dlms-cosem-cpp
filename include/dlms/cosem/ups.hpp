#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <vector>
#include <string>

namespace dlms::cosem {

/**
 * IC40 UPS - Uninterruptible Power Supply
 *
 * This class manages UPS status and configuration for DLMS/COSEM devices.
 *
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: ups_status (unsigned)
 * 3: battery_status (unsigned)
 * 4: battery_voltage (unsigned)
 * 5: battery_capacity (unsigned)
 */
class Ups : public CosemObject {
public:
    uint16_t class_id() const override { return 40; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "Ups"; }

    enum class UpsStatus : uint8_t {
        Offline = 0,
        Online = 1,
        OnBattery = 2,
        LowBattery = 3,
        ReplaceBattery = 4
    };

    enum class BatteryStatus : uint8_t {
        Unknown = 0,
        Good = 1,
        Low = 2,
        Replace = 3
    };

    UpsStatus ups_status_ = UpsStatus::Offline;
    BatteryStatus battery_status_ = BatteryStatus::Unknown;
    uint16_t battery_voltage_ = 0;
    uint8_t battery_capacity_ = 0;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "ups_status", true},
            AttributeAccess{3, "battery_status", true},
            AttributeAccess{4, "battery_voltage", false},
            AttributeAccess{5, "battery_capacity", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) {
            auto a = ln_.to_array();
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end()));
        }
        if (id == 2) return DlmsValue(static_cast<uint8_t>(ups_status_));
        if (id == 3) return DlmsValue(static_cast<uint8_t>(battery_status_));
        if (id == 4) return DlmsValue(battery_voltage_);
        if (id == 5) return DlmsValue(battery_capacity_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* p = std::get_if<uint8_t>(&v)) ups_status_ = static_cast<UpsStatus>(*p);
            return make_void_ok();
        }
        if (id == 3) {
            if (auto* p = std::get_if<uint8_t>(&v)) battery_status_ = static_cast<BatteryStatus>(*p);
            return make_void_ok();
        }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    bool is_on_battery() const { return ups_status_ == UpsStatus::OnBattery; }
};

} // namespace dlms::cosem
