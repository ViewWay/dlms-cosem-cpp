#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>

#include <vector>
#include <string>

namespace dlms::cosem {

class SecuritySetup : public CosemObject {
public:
    uint16_t class_id() const override { return static_cast<uint16_t>(CosemInterface::SecuritySetup); }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "SecuritySetup"; }

    SecurityPolicy policy_ = SecurityPolicy::AuthenticatedEncrypted; int suite_ = 0; std::vector<uint8_t> system_title_; std::vector<uint8_t> auth_key_; std::vector<uint8_t> enc_key_;

    std::vector<AttributeAccess> get_attributes() const override {
        return {AttributeAccess{1, "logical_name", true}, AttributeAccess{2, "security_policy", true}, AttributeAccess{3, "security_suite", true}, AttributeAccess{5, "auth_key", true}, AttributeAccess{6, "enc_key", true}, AttributeAccess{8, "system_title", true}};
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { auto a = ln_.to_array(); return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); }
        if (id == 2) return DlmsValue(static_cast<uint8_t>(policy_));
if (id == 3) return DlmsValue(static_cast<uint8_t>(suite_));
if (id == 8) return DlmsValue(system_title_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) { if (auto* p = std::get_if<uint8_t>(&v)) policy_=static_cast<SecurityPolicy>(*p); return make_void_ok(); }
if (id == 3) { if (auto* p = std::get_if<uint8_t>(&v)) suite_=*p; return make_void_ok(); }
if (id == 8) { if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) system_title_=*p; return make_void_ok(); }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }
};

} // namespace dlms::cosem
