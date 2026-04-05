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
 * IC11 Special Days Table
 * 
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: entries (array of structures)
 * 
 * Each entry: index, date, day_id
 */
struct SpecialDayEntry {
    uint16_t index;
    std::vector<uint8_t> date;
    uint8_t day_id;
};

class SpecialDaysTable : public CosemObject {
public:
    uint16_t class_id() const override { return static_cast<uint16_t>(CosemInterface::SpecialDaysTable); }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "SpecialDaysTable"; }

    std::vector<SpecialDayEntry> entries_;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "entries", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { 
            auto a = ln_.to_array(); 
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); 
        }
        if (id == 2) {
            std::vector<DlmsValue> arr;
            for (const auto& entry : entries_) {
                arr.push_back(make_dlms_structure({
                    DlmsValue(entry.index),
                    DlmsValue(entry.date),
                    DlmsValue(entry.day_id)
                }));
            }
            return make_dlms_array(arr);
        }

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) {
            // Accept array of entries
            return make_void_ok();
        }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void add_entry(uint16_t index, const std::vector<uint8_t>& date, uint8_t day_id) {
        entries_.push_back({index, date, day_id});
    }

    void clear_entries() {
        entries_.clear();
    }

    size_t entry_count() const { return entries_.size(); }

    Result<std::vector<uint8_t>> execute_insert(uint16_t index, const std::vector<uint8_t>& date, uint8_t day_id) {
        entries_.insert(entries_.begin() + index, {index, date, day_id});
        return std::vector<uint8_t>{};
    }

    Result<std::vector<uint8_t>> execute_delete(uint16_t index) {
        if (index < entries_.size()) {
            entries_.erase(entries_.begin() + index);
            return std::vector<uint8_t>{};
        }
        return make_err<std::vector<uint8_t>>(DlmsError::InvalidData);
    }
};

} // namespace dlms::cosem
