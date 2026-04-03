#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>

#include <vector>
#include <string>

namespace dlms::cosem {

class Rs485Setup : public CosemObject {
public:
    uint16_t class_id() const override { return static_cast<uint16_t>(CosemInterface::IecHdlcSetup); }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "RS485Setup"; }

    uint32_t baud_rate_ = 9600; uint8_t mode_ = 0;

    std::vector<AttributeAccess> get_attributes() const override {
        return {AttributeAccess{1, "logical_name", true}, AttributeAccess{2, "baud_rate", true}, AttributeAccess{3, "mode", true}};
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { auto a = ln_.to_array(); return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); }
        if (id == 2) return DlmsValue(baud_rate_);
        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) { if (auto* p = std::get_if<uint32_t>(&v)) baud_rate_=*p; return make_void_ok(); }
        return make_err<std::monostate>(DlmsError::AccessDenied);
    }
};

} // namespace dlms::cosem
