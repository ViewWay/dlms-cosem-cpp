#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <vector>
#include <string>

namespace dlms::cosem {

/**
 * IC102 SMTP Setup - SMTP Configuration
 *
 * This class manages SMTP email configuration for DLMS/COSEM devices.
 *
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: smtp_server (octet-string)
 * 3: smtp_port (unsigned)
 * 4: username (octet-string)
 * 5: password (octet-string)
 * 6: from_address (octet-string)
 * 7: authentication_enabled (boolean)
 * 8: tls_enabled (boolean)
 */
class SmtpSetup : public CosemObject {
public:
    uint16_t class_id() const override { return 102; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "SmtpSetup"; }

    std::vector<uint8_t> smtp_server_;
    uint16_t smtp_port_ = 25;
    std::vector<uint8_t> username_;
    std::vector<uint8_t> password_;
    std::vector<uint8_t> from_address_;
    bool authentication_enabled_ = false;
    bool tls_enabled_ = false;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "smtp_server", true},
            AttributeAccess{3, "smtp_port", true},
            AttributeAccess{4, "username", true},
            AttributeAccess{5, "password", true},
            AttributeAccess{6, "from_address", true},
            AttributeAccess{7, "authentication_enabled", true},
            AttributeAccess{8, "tls_enabled", true}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) {
            auto a = ln_.to_array();
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end()));
        }
        if (id == 2) return DlmsValue(smtp_server_);
        if (id == 3) return DlmsValue(smtp_port_);
        if (id == 4) return DlmsValue(username_);
        if (id == 5) return DlmsValue(password_);
        if (id == 6) return DlmsValue(from_address_);
        if (id == 7) return DlmsValue(authentication_enabled_);
        if (id == 8) return DlmsValue(tls_enabled_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) smtp_server_ = *p;
            return make_void_ok();
        }
        if (id == 3) {
            if (auto* p = std::get_if<uint16_t>(&v)) smtp_port_ = *p;
            return make_void_ok();
        }
        if (id == 4) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) username_ = *p;
            return make_void_ok();
        }
        if (id == 5) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) password_ = *p;
            return make_void_ok();
        }
        if (id == 6) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) from_address_ = *p;
            return make_void_ok();
        }
        if (id == 7) {
            if (auto* p = std::get_if<bool>(&v)) authentication_enabled_ = *p;
            return make_void_ok();
        }
        if (id == 8) {
            if (auto* p = std::get_if<bool>(&v)) tls_enabled_ = *p;
            return make_void_ok();
        }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void set_smtp_server(const std::string& server) {
        smtp_server_.assign(server.begin(), server.end());
    }
    void set_smtp_port(uint16_t port) { smtp_port_ = port; }
};

} // namespace dlms::cosem
