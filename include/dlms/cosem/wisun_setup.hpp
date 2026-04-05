#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <vector>
#include <string>

namespace dlms::cosem {

/**
 * IC44 Wi-SUN Setup - Wi-SUN Network Configuration
 *
 * This class manages Wi-SUN network configuration for DLMS/COSEM devices.
 *
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: network_id (octet-string)
 * 3: channel (unsigned)
 * 4: pan_id (unsigned)
 * 5: mode (enum)
 */
class WisunSetup : public CosemObject {
public:
    uint16_t class_id() const override { return 44; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "WisunSetup"; }

    enum class Mode : uint8_t {
        Router = 0,
        BorderRouter = 1,
        SleepyDevice = 2
    };

    std::vector<uint8_t> network_id_;
    uint8_t channel_ = 0;
    uint16_t pan_id_ = 0xFFFF;
    Mode mode_ = Mode::Router;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "network_id", true},
            AttributeAccess{3, "channel", true},
            AttributeAccess{4, "pan_id", true},
            AttributeAccess{5, "mode", true}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) {
            auto a = ln_.to_array();
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end()));
        }
        if (id == 2) return DlmsValue(network_id_);
        if (id == 3) return DlmsValue(channel_);
        if (id == 4) return DlmsValue(pan_id_);
        if (id == 5) return DlmsValue(static_cast<uint8_t>(mode_));

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) network_id_ = *p;
            return make_void_ok();
        }
        if (id == 3) {
            if (auto* p = std::get_if<uint8_t>(&v)) channel_ = *p;
            return make_void_ok();
        }
        if (id == 4) {
            if (auto* p = std::get_if<uint16_t>(&v)) pan_id_ = *p;
            return make_void_ok();
        }
        if (id == 5) {
            if (auto* p = std::get_if<uint8_t>(&v)) mode_ = static_cast<Mode>(*p);
            return make_void_ok();
        }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void set_channel(uint8_t channel) { channel_ = channel; }
    void set_mode(Mode mode) { mode_ = mode; }
};

} // namespace dlms::cosem
