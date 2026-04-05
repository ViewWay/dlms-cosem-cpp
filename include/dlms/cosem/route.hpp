#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <vector>
#include <string>

namespace dlms::cosem {

/**
 * IC86/219 Route - Network Routing Entry
 *
 * This class manages network routing configuration for DLMS/COSEM devices.
 *
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: destination (octet-string)
 * 3: next_hop (octet-string)
 * 4: metric (unsigned)
 * 5: route_status (unsigned)
 */
class Route : public CosemObject {
public:
    uint16_t class_id() const override { return 219; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "Route"; }

    std::vector<uint8_t> destination_;
    std::vector<uint8_t> next_hop_;
    uint8_t metric_ = 0;
    uint8_t route_status_ = 0;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "destination", true},
            AttributeAccess{3, "next_hop", true},
            AttributeAccess{4, "metric", true},
            AttributeAccess{5, "route_status", true}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) {
            auto a = ln_.to_array();
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end()));
        }
        if (id == 2) return DlmsValue(destination_);
        if (id == 3) return DlmsValue(next_hop_);
        if (id == 4) return DlmsValue(metric_);
        if (id == 5) return DlmsValue(route_status_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) destination_ = *p;
            return make_void_ok();
        }
        if (id == 3) {
            if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) next_hop_ = *p;
            return make_void_ok();
        }
        if (id == 4) {
            if (auto* p = std::get_if<uint8_t>(&v)) metric_ = *p;
            return make_void_ok();
        }
        if (id == 5) {
            if (auto* p = std::get_if<uint8_t>(&v)) route_status_ = *p;
            return make_void_ok();
        }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void set_destination(const std::vector<uint8_t>& dest) { destination_ = dest; }
    void set_next_hop(const std::vector<uint8_t>& hop) { next_hop_ = hop; }
    void set_metric(uint8_t metric) { metric_ = metric; }
};

} // namespace dlms::cosem
