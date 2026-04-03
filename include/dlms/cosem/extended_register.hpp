#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>

#include <vector>
#include <string>

namespace dlms::cosem {

class ExtendedRegister : public CosemObject {
public:
    uint16_t class_id() const override { return static_cast<uint16_t>(CosemInterface::ExtendedRegister); }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "ExtendedRegister"; }

    DlmsValue value_ = DlmsValue(DlmsNone{}); int8_t scaler_ = 0; uint8_t unit_ = 0; uint32_t status_ = 0;

    std::vector<AttributeAccess> get_attributes() const override {
        return {AttributeAccess{1, "logical_name", true}, AttributeAccess{2, "value", false}, AttributeAccess{3, "scaler", true}, AttributeAccess{4, "unit", true}, AttributeAccess{5, "status", false}};
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { auto a = ln_.to_array(); return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); }
        if (id == 2) return value_;
if (id == 3) return DlmsValue(scaler_);
if (id == 4) return DlmsValue(unit_);
if (id == 5) return DlmsValue(status_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) { value_ = v; return make_void_ok(); }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }
};

} // namespace dlms::cosem
