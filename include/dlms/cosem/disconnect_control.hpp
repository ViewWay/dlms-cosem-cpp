#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>

#include <vector>
#include <string>
#include <memory>

namespace dlms::cosem {

// Forward declaration of helpers (defined in association_ln.hpp)
inline DlmsValue make_dlms_array(const std::vector<DlmsValue>& items);
inline DlmsValue make_dlms_structure(const std::vector<DlmsValue>& elements);

/**
 * IC70 Disconnect Control
 * 
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: control_state (enum: 0=disconnected, 1=connected, 2=ready_for_disconnect, 3=ready_for_reconnect)
 * 3: control_value (enum)
 * 
 * Methods:
 * 1: remote_disconnect
 * 2: remote_reconnect
 */
enum class DisconnectState : uint8_t {
    Disconnected = 0,
    Connected = 1,
    ReadyForDisconnect = 2,
    ReadyForReconnect = 3,
};

class DisconnectControl : public CosemObject {
public:
    uint16_t class_id() const override { return 70; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "DisconnectControl"; }

    DisconnectState control_state_ = DisconnectState::Connected;
    DisconnectState output_state_ = DisconnectState::Connected;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "control_state", false},
            AttributeAccess{3, "control_value", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { 
            auto a = ln_.to_array(); 
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); 
        }
        if (id == 2) return DlmsValue(static_cast<uint8_t>(control_state_));
        if (id == 3) return DlmsValue(static_cast<uint8_t>(output_state_));

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* p = std::get_if<uint8_t>(&v)) {
                control_state_ = static_cast<DisconnectState>(*p);
                return make_void_ok();
            }
        }
        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    Result<std::vector<uint8_t>> execute_method(uint8_t method_id) {
        if (method_id == 1) {
            // Remote disconnect
            control_state_ = DisconnectState::Disconnected;
            output_state_ = DisconnectState::Disconnected;
            return std::vector<uint8_t>{};
        }
        if (method_id == 2) {
            // Remote reconnect
            control_state_ = DisconnectState::Connected;
            output_state_ = DisconnectState::Connected;
            return std::vector<uint8_t>{};
        }
        return make_err<std::vector<uint8_t>>(DlmsError::NotSupported);
    }

    DisconnectState control_state() const { return control_state_; }
    DisconnectState output_state() const { return output_state_; }
    void set_control_state(DisconnectState s) { control_state_ = s; }
};

} // namespace dlms::cosem
