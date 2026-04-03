#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>

#include <vector>
#include <string>

namespace dlms::cosem {

class DemandRegister : public CosemObject {
public:
    uint16_t class_id() const override { return static_cast<uint16_t>(CosemInterface::DemandRegister); }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "DemandRegister"; }

    DlmsValue value_ = DlmsValue(DlmsNone{}); uint32_t status_ = 0; int8_t scaler_ = 0; uint8_t unit_ = 0; uint32_t period_ = 0;

    std::vector<AttributeAccess> get_attributes() const override {
        return {AttributeAccess{1, "logical_name", true}, AttributeAccess{2, "current_value", false}, AttributeAccess{3, "status", false}, AttributeAccess{4, "scaler", true}, AttributeAccess{5, "unit", true}, AttributeAccess{6, "period", true}};
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { auto a = ln_.to_array(); return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); }
        if (id == 2) return value_;
if (id == 3) return DlmsValue(status_);
if (id == 4) return DlmsValue(scaler_);
if (id == 5) return DlmsValue(unit_);
if (id == 6) return DlmsValue(period_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) { value_ = v; return make_void_ok(); }
if (id == 6) { if (auto* p = std::get_if<uint32_t>(&v)) period_=*p; return make_void_ok(); }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }
};

} // namespace dlms::cosem
