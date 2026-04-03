#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <dlms/core/datetime.hpp>
#include <vector>
#include <string>

namespace dlms::cosem {

class Clock : public CosemObject {
public:
    uint16_t class_id() const override { return static_cast<uint16_t>(CosemInterface::Clock); }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "Clock"; }

    CosemDateTime time_{}; int16_t timezone_ = 480; uint8_t status_ = 0;

    std::vector<AttributeAccess> get_attributes() const override {
        return {AttributeAccess{1, "logical_name", true}, AttributeAccess{2, "time", false}, AttributeAccess{3, "timezone", true}, AttributeAccess{4, "status", false}};
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { auto a = ln_.to_array(); return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); }
        if (id == 2) { uint8_t b[12]; time_.to_bytes(b); return DlmsValue(std::vector<uint8_t>(b, b+12)); }
if (id == 3) return DlmsValue(timezone_);
if (id == 4) return DlmsValue(status_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) { if (auto* p = std::get_if<std::vector<uint8_t>>(&v); p && p->size()==12) time_=CosemDateTime::from_bytes(p->data()); return make_void_ok(); }
        if (id == 3) { if (auto* p = std::get_if<int16_t>(&v); p) timezone_=*p; return make_void_ok(); }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }
};

} // namespace dlms::cosem
