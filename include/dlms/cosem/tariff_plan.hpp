#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>

#include <vector>
#include <string>

namespace dlms::cosem {

class TariffPlan : public CosemObject {
public:
    uint16_t class_id() const override { return static_cast<uint16_t>(CosemInterface::UtilityTables); }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "TariffPlan"; }

    std::string name_;

    std::vector<AttributeAccess> get_attributes() const override {
        return {AttributeAccess{1, "logical_name", true}};
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { auto a = ln_.to_array(); return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); }
        
        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        
        return make_err<std::monostate>(DlmsError::AccessDenied);
    }
};

} // namespace dlms::cosem
