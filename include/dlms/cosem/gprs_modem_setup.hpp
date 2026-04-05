#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>

#include <vector>
#include <string>

namespace dlms::cosem {

inline DlmsValue make_dlms_array(const std::vector<DlmsValue>& items);
inline DlmsValue make_dlms_structure(const std::vector<DlmsValue>& elements);

/**
 * IC45 GPRS Modem Setup
 * 
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: apn (visible-string)
 * 3: pin_code (visible-string)
 * 4: quality_of_service (structure: qos_profile, qos_params)
 * 5: pdp_context_parameters (array of structures)
 */
struct PdpContext {
    uint8_t pdp_context_id;
    std::string pdp_type;
    std::string pdp_address;
    std::string pdp_apn;
};

class GprsModemSetup : public CosemObject {
public:
    uint16_t class_id() const override { return static_cast<uint16_t>(CosemInterface::GprsModemSetup); }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "GprsModemSetup"; }

    std::string apn_;
    std::string pin_code_;
    std::vector<uint8_t> quality_of_service_;
    std::vector<PdpContext> pdp_contexts_;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "apn", false},
            AttributeAccess{3, "pin_code", false},
            AttributeAccess{4, "quality_of_service", false},
            AttributeAccess{5, "pdp_context_parameters", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { 
            auto a = ln_.to_array(); 
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); 
        }
        if (id == 2) return DlmsValue(apn_);
        if (id == 3) return DlmsValue(pin_code_);
        if (id == 4) return DlmsValue(quality_of_service_);
        if (id == 5) {
            std::vector<DlmsValue> arr;
            for (const auto& ctx : pdp_contexts_) {
                arr.push_back(make_dlms_structure({
                    DlmsValue(ctx.pdp_context_id),
                    DlmsValue(ctx.pdp_type),
                    DlmsValue(ctx.pdp_address),
                    DlmsValue(ctx.pdp_apn)
                }));
            }
            return make_dlms_array(arr);
        }

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) { if (auto* p = std::get_if<std::string>(&v)) apn_ = *p; return make_void_ok(); }
        if (id == 3) { if (auto* p = std::get_if<std::string>(&v)) pin_code_ = *p; return make_void_ok(); }
        if (id == 4) { if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) quality_of_service_ = *p; return make_void_ok(); }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void set_apn(const std::string& apn) { apn_ = apn; }
    void set_pin(const std::string& pin) { pin_code_ = pin; }
    
    void add_pdp_context(uint8_t id, const std::string& type, const std::string& addr, const std::string& apn) {
        pdp_contexts_.push_back({id, type, addr, apn});
    }

    const std::string& apn() const { return apn_; }
    const std::string& pin_code() const { return pin_code_; }
};

} // namespace dlms::cosem
