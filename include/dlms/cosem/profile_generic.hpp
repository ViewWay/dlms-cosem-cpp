#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <vector>
#include <string>
#include <memory>

namespace dlms::cosem {

class ProfileGeneric : public CosemObject {
public:
    uint16_t class_id() const override { return static_cast<uint16_t>(CosemInterface::ProfileGeneric); }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "ProfileGeneric"; }

    std::vector<std::vector<DlmsValue>> buffer_;
    std::vector<std::pair<ObisCode,int>> capture_objects_;
    uint32_t capture_period_ = 0;
    uint32_t entries_in_use_ = 0;
    uint32_t profile_entries_ = 0;

    std::vector<AttributeAccess> get_attributes() const override {
        return {AttributeAccess{1, "logical_name", true}, AttributeAccess{2, "buffer", false}, AttributeAccess{3, "capture_objects", true}, AttributeAccess{4, "capture_period", true}, AttributeAccess{7, "entries_in_use", false}, AttributeAccess{8, "profile_entries", true}};
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { auto a = ln_.to_array(); return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); }
        if (id == 2) {
            auto arr = std::make_unique<DlmsArray>();
            for (auto& entry : buffer_) {
                auto s = std::make_unique<DlmsStructure>();
                for (auto& v : entry) s->elements.push_back(std::make_shared<DlmsValue>(v));
                arr->items.push_back(std::make_shared<DlmsValue>(DlmsValue(std::move(*s))));
            }
            return DlmsValue(std::move(*arr));
        }
        if (id == 7) return DlmsValue(entries_in_use_);
        if (id == 8) return DlmsValue(profile_entries_);
        if (id == 4) return DlmsValue(capture_period_);
        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 8) { if (auto* p = std::get_if<uint32_t>(&v)) profile_entries_=*p; return make_void_ok(); }
        if (id == 4) { if (auto* p = std::get_if<uint32_t>(&v)) capture_period_=*p; return make_void_ok(); }
        return make_err<std::monostate>(DlmsError::AccessDenied);
    }
};

} // namespace dlms::cosem
