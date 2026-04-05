#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <vector>
#include <string>

namespace dlms::cosem {

/**
 * IC42 IPv4 Setup - IPv4 Network Configuration Object
 *
 * This class manages IPv4 network configuration for DLMS/COSEM devices.
 *
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: ip_address (octet-string)
 * 3: subnet_mask (octet-string)
 * 4: gateway (octet-string)
 * 5: primary_dns (octet-string)
 * 6: secondary_dns (octet-string)
 * 7: dhcp_enabled (boolean)
 */
class Ipv4Setup : public CosemObject {
public:
    uint16_t class_id() const override { return 42; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "Ipv4Setup"; }

    std::vector<uint8_t> ip_address_{0, 0, 0, 0};
    std::vector<uint8_t> subnet_mask_{255, 255, 255, 0};
    std::vector<uint8_t> gateway_{0, 0, 0, 0};
    std::vector<uint8_t> primary_dns_{0, 0, 0, 0};
    std::vector<uint8_t> secondary_dns_{0, 0, 0, 0};
    bool dhcp_enabled_ = false;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "ip_address", true},
            AttributeAccess{3, "subnet_mask", true},
            AttributeAccess{4, "gateway", true},
            AttributeAccess{5, "primary_dns", true},
            AttributeAccess{6, "secondary_dns", true},
            AttributeAccess{7, "dhcp_enabled", true}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) {
            auto a = ln_.to_array();
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end()));
        }
        if (id == 2) return DlmsValue(ip_address_);
        if (id == 3) return DlmsValue(subnet_mask_);
        if (id == 4) return DlmsValue(gateway_);
        if (id == 5) return DlmsValue(primary_dns_);
        if (id == 6) return DlmsValue(secondary_dns_);
        if (id == 7) return DlmsValue(dhcp_enabled_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) ip_address_ = *p;
            return make_void_ok();
        }
        if (id == 3) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) subnet_mask_ = *p;
            return make_void_ok();
        }
        if (id == 4) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) gateway_ = *p;
            return make_void_ok();
        }
        if (id == 5) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) primary_dns_ = *p;
            return make_void_ok();
        }
        if (id == 6) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) secondary_dns_ = *p;
            return make_void_ok();
        }
        if (id == 7) {
            if (auto* p = std::get_if<bool>(&v)) dhcp_enabled_ = *p;
            return make_void_ok();
        }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void set_ip_address(const std::vector<uint8_t>& ip) { ip_address_ = ip; }
    void set_subnet_mask(const std::vector<uint8_t>& mask) { subnet_mask_ = mask; }
    void set_gateway(const std::vector<uint8_t>& gw) { gateway_ = gw; }
    void set_dhcp_enabled(bool enabled) { dhcp_enabled_ = enabled; }
};

} // namespace dlms::cosem
