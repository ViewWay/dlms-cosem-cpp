#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <vector>
#include <string>

namespace dlms::cosem {

/**
 * IC60 Serial Port - Serial Port Configuration Object
 *
 * This class manages serial port configuration for DLMS/COSEM devices.
 *
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: baud_rate (unsigned)
 * 3: data_bits (unsigned)
 * 4: stop_bits (unsigned)
 * 5: parity (enum)
 * 6: flow_control (enum)
 * 7: buffer_size (unsigned)
 * 8: timeout (unsigned)
 */
class SerialPort : public CosemObject {
public:
    uint16_t class_id() const override { return 60; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "SerialPort"; }

    enum class Parity : uint8_t {
        None = 0,
        Odd = 1,
        Even = 2,
        Mark = 3,
        Space = 4
    };

    enum class FlowControl : uint8_t {
        None = 0,
        Hardware = 1,
        Software = 2
    };

    uint32_t baud_rate_ = 9600;
    uint8_t data_bits_ = 8;
    uint8_t stop_bits_ = 1;
    Parity parity_ = Parity::None;
    FlowControl flow_control_ = FlowControl::None;
    uint16_t buffer_size_ = 1024;
    uint16_t timeout_ = 100;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "baud_rate", true},
            AttributeAccess{3, "data_bits", true},
            AttributeAccess{4, "stop_bits", true},
            AttributeAccess{5, "parity", true},
            AttributeAccess{6, "flow_control", true},
            AttributeAccess{7, "buffer_size", true},
            AttributeAccess{8, "timeout", true}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) {
            auto a = ln_.to_array();
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end()));
        }
        if (id == 2) return DlmsValue(baud_rate_);
        if (id == 3) return DlmsValue(data_bits_);
        if (id == 4) return DlmsValue(stop_bits_);
        if (id == 5) return DlmsValue(static_cast<uint8_t>(parity_));
        if (id == 6) return DlmsValue(static_cast<uint8_t>(flow_control_));
        if (id == 7) return DlmsValue(buffer_size_);
        if (id == 8) return DlmsValue(timeout_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* p = std::get_if<uint32_t>(&v)) baud_rate_ = *p;
            return make_void_ok();
        }
        if (id == 3) {
            if (auto* p = std::get_if<uint8_t>(&v)) data_bits_ = *p;
            return make_void_ok();
        }
        if (id == 4) {
            if (auto* p = std::get_if<uint8_t>(&v)) stop_bits_ = *p;
            return make_void_ok();
        }
        if (id == 5) {
            if (auto* p = std::get_if<uint8_t>(&v)) parity_ = static_cast<Parity>(*p);
            return make_void_ok();
        }
        if (id == 6) {
            if (auto* p = std::get_if<uint8_t>(&v)) flow_control_ = static_cast<FlowControl>(*p);
            return make_void_ok();
        }
        if (id == 7) {
            if (auto* p = std::get_if<uint16_t>(&v)) buffer_size_ = *p;
            return make_void_ok();
        }
        if (id == 8) {
            if (auto* p = std::get_if<uint16_t>(&v)) timeout_ = *p;
            return make_void_ok();
        }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void set_baud_rate(uint32_t rate) { baud_rate_ = rate; }
    void set_data_bits(uint8_t bits) { data_bits_ = bits; }
    void set_parity(Parity parity) { parity_ = parity; }
};

} // namespace dlms::cosem
