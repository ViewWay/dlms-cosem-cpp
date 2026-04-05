#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <vector>
#include <string>

namespace dlms::cosem {

/**
 * IC100 NTP Setup - Network Time Protocol Configuration
 *
 * This class manages NTP time synchronization configuration.
 *
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: ntp_server_address (octet-string)
 * 3: ntp_port (unsigned)
 * 4: authentication_method (unsigned)
 * 5: randomization_interval (unsigned)
 * 6: time_offset (integer)
 * 7: time_threshold (unsigned)
 */
class NtpSetup : public CosemObject {
public:
    uint16_t class_id() const override { return 100; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "NtpSetup"; }

    std::vector<uint8_t> ntp_server_address_;
    uint16_t ntp_port_ = 123;
    uint8_t authentication_method_ = 0;
    uint32_t randomization_interval_ = 300;
    int32_t time_offset_ = 0;
    uint32_t time_threshold_ = 1;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "ntp_server_address", true},
            AttributeAccess{3, "ntp_port", true},
            AttributeAccess{4, "authentication_method", true},
            AttributeAccess{5, "randomization_interval", true},
            AttributeAccess{6, "time_offset", true},
            AttributeAccess{7, "time_threshold", true}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) {
            auto a = ln_.to_array();
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end()));
        }
        if (id == 2) return DlmsValue(ntp_server_address_);
        if (id == 3) return DlmsValue(ntp_port_);
        if (id == 4) return DlmsValue(authentication_method_);
        if (id == 5) return DlmsValue(randomization_interval_);
        if (id == 6) return DlmsValue(time_offset_);
        if (id == 7) return DlmsValue(time_threshold_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) ntp_server_address_ = *p;
            return make_void_ok();
        }
        if (id == 3) {
            if (auto* p = std::get_if<uint16_t>(&v)) ntp_port_ = *p;
            return make_void_ok();
        }
        if (id == 4) {
            if (auto* p = std::get_if<uint8_t>(&v)) authentication_method_ = *p;
            return make_void_ok();
        }
        if (id == 5) {
            if (auto* p = std::get_if<uint32_t>(&v)) randomization_interval_ = *p;
            return make_void_ok();
        }
        if (id == 6) {
            if (auto* p = std::get_if<int32_t>(&v)) time_offset_ = *p;
            return make_void_ok();
        }
        if (id == 7) {
            if (auto* p = std::get_if<uint32_t>(&v)) time_threshold_ = *p;
            return make_void_ok();
        }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void set_ntp_server(const std::string& server) {
        ntp_server_address_.assign(server.begin(), server.end());
    }
    void set_ntp_port(uint16_t port) { ntp_port_ = port; }
};

} // namespace dlms::cosem
