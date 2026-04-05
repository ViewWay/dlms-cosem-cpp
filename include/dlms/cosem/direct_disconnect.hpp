#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <vector>
#include <string>

namespace dlms::cosem {

/**
 * IC63 Direct Disconnect - Direct Disconnect Control Object
 *
 * This class controls the direct disconnect output of a DLMS/COSEM device.
 * Used for load control and remote disconnect/connect functionality.
 *
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: output_state (enum)
 * 3: control_state (enum)
 * 4: effective_control_state (enum)
 *
 * Methods:
 * 1: remote_disconnect - disconnect the load
 * 2: remote_connect - connect the load
 */
class DirectDisconnect : public CosemObject {
public:
    uint16_t class_id() const override { return 63; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "DirectDisconnect"; }

    enum class OutputState : uint8_t {
        Disconnected = 0,
        Connected = 1,
        ReadyForReconnection = 2,
    };

    enum class ControlState : uint8_t {
        Disabled = 0,
        Enabled = 1,
    };

    OutputState output_state_ = OutputState::Connected;
    ControlState control_state_ = ControlState::Disabled;
    ControlState effective_control_state_ = ControlState::Disabled;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "output_state", true},
            AttributeAccess{3, "control_state", true},
            AttributeAccess{4, "effective_control_state", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) {
            auto a = ln_.to_array();
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end()));
        }
        if (id == 2) return DlmsValue(static_cast<uint8_t>(output_state_));
        if (id == 3) return DlmsValue(static_cast<uint8_t>(control_state_));
        if (id == 4) return DlmsValue(static_cast<uint8_t>(effective_control_state_));

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* p = std::get_if<uint8_t>(&v)) output_state_ = static_cast<OutputState>(*p);
            return make_void_ok();
        }
        if (id == 3) {
            if (auto* p = std::get_if<uint8_t>(&v)) control_state_ = static_cast<ControlState>(*p);
            return make_void_ok();
        }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    bool is_connected() const { return output_state_ == OutputState::Connected; }
    void set_output_state(OutputState state) { output_state_ = state; }
    void set_control_state(ControlState state) { control_state_ = state; }
};

} // namespace dlms::cosem
