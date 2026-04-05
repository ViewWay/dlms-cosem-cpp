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
 * IC40 Push Setup (actually IC15 in some versions)
 * 
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: push_object_list (array of structures)
 * 3: service (long-unsigned)
 * 4: destination (octet-string)
 * 5: communication_window (array of structures)
 * 6: randomisation_start_interval (long-unsigned)
 * 7: number_of_retries (unsigned)
 * 8: repetition_delay (long-unsigned)
 */
struct PushObjectEntry {
    uint16_t class_id;
    ObisCode logical_name;
    uint8_t attribute_id;
    uint8_t data_index;
};

class PushSetup : public CosemObject {
public:
    uint16_t class_id() const override { return 40; }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "PushSetup"; }

    std::vector<PushObjectEntry> push_objects_;
    uint16_t service_ = 0;
    std::vector<uint8_t> destination_;
    uint16_t randomisation_start_interval_ = 0;
    uint8_t number_of_retries_ = 3;
    uint16_t repetition_delay_ = 60;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "push_object_list", false},
            AttributeAccess{3, "service", false},
            AttributeAccess{4, "destination", false},
            AttributeAccess{5, "communication_window", false},
            AttributeAccess{6, "randomisation_start_interval", false},
            AttributeAccess{7, "number_of_retries", false},
            AttributeAccess{8, "repetition_delay", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { 
            auto a = ln_.to_array(); 
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); 
        }
        if (id == 2) {
            std::vector<DlmsValue> arr;
            for (const auto& obj : push_objects_) {
                auto ob = obj.logical_name.to_array();
                arr.push_back(make_dlms_structure({
                    DlmsValue(static_cast<uint16_t>(obj.class_id)),
                    DlmsValue(std::vector<uint8_t>(ob.begin(), ob.end())),
                    DlmsValue(static_cast<int8_t>(obj.attribute_id)),
                    DlmsValue(static_cast<uint8_t>(obj.data_index))
                }));
            }
            return make_dlms_array(arr);
        }
        if (id == 3) return DlmsValue(service_);
        if (id == 4) return DlmsValue(destination_);
        if (id == 6) return DlmsValue(randomisation_start_interval_);
        if (id == 7) return DlmsValue(number_of_retries_);
        if (id == 8) return DlmsValue(repetition_delay_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 3) { if (auto* p = std::get_if<uint16_t>(&v)) service_ = *p; return make_void_ok(); }
        if (id == 4) { if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) destination_ = *p; return make_void_ok(); }
        if (id == 6) { if (auto* p = std::get_if<uint16_t>(&v)) randomisation_start_interval_ = *p; return make_void_ok(); }
        if (id == 7) { if (auto* p = std::get_if<uint8_t>(&v)) number_of_retries_ = *p; return make_void_ok(); }
        if (id == 8) { if (auto* p = std::get_if<uint16_t>(&v)) repetition_delay_ = *p; return make_void_ok(); }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void add_push_object(uint16_t class_id, const ObisCode& ln, uint8_t attr_id, uint8_t data_index = 0) {
        push_objects_.push_back({class_id, ln, attr_id, data_index});
    }

    size_t object_count() const { return push_objects_.size(); }
};

} // namespace dlms::cosem
