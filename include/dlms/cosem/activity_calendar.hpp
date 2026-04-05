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
 * IC20 Activity Calendar
 * 
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: calendar_name_active (visible-string)
 * 3: season_profile_active (array of structures)
 * 4: week_profile_table_active (array of structures)
 * 5: day_profile_table_active (array of structures)
 * 6: calendar_name_passive (visible-string)
 * 7: season_profile_passive (array of structures)
 * 8: week_profile_table_passive (array of structures)
 * 9: day_profile_table_passive (array of structures)
 * 10: activate_passive_calendar_time (date-time)
 */
struct SeasonEntry {
    std::string season_name;
    std::string week_name;
    std::vector<uint8_t> season_start;
};

struct WeekProfile {
    std::string week_name;
    uint8_t monday;
    uint8_t tuesday;
    uint8_t wednesday;
    uint8_t thursday;
    uint8_t friday;
    uint8_t saturday;
    uint8_t sunday;
};

struct DayProfile {
    uint8_t day_id;
    std::vector<uint8_t> schedule;
};

class ActivityCalendar : public CosemObject {
public:
    uint16_t class_id() const override { return static_cast<uint16_t>(CosemInterface::ActivityCalendar); }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "ActivityCalendar"; }

    std::string calendar_name_active_;
    std::vector<SeasonEntry> season_profile_active_;
    std::vector<WeekProfile> week_profile_active_;
    std::vector<DayProfile> day_profile_active_;
    
    std::string calendar_name_passive_;
    std::vector<SeasonEntry> season_profile_passive_;
    std::vector<WeekProfile> week_profile_passive_;
    std::vector<DayProfile> day_profile_passive_;
    
    std::vector<uint8_t> activate_passive_calendar_time_;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "calendar_name_active", false},
            AttributeAccess{3, "season_profile_active", false},
            AttributeAccess{4, "week_profile_table_active", false},
            AttributeAccess{5, "day_profile_table_active", false},
            AttributeAccess{6, "calendar_name_passive", false},
            AttributeAccess{7, "season_profile_passive", false},
            AttributeAccess{8, "week_profile_table_passive", false},
            AttributeAccess{9, "day_profile_table_passive", false},
            AttributeAccess{10, "activate_passive_calendar_time", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { 
            auto a = ln_.to_array(); 
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); 
        }
        if (id == 2) return DlmsValue(calendar_name_active_);
        if (id == 6) return DlmsValue(calendar_name_passive_);
        if (id == 10) return DlmsValue(activate_passive_calendar_time_);

        // For complex attributes, return empty array for now
        if (id >= 3 && id <= 9) {
            return make_dlms_array(std::vector<DlmsValue>{});
        }

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) { if (auto* p = std::get_if<std::string>(&v)) calendar_name_active_ = *p; return make_void_ok(); }
        if (id == 6) { if (auto* p = std::get_if<std::string>(&v)) calendar_name_passive_ = *p; return make_void_ok(); }
        if (id == 10) { if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) activate_passive_calendar_time_ = *p; return make_void_ok(); }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void activate_passive() {
        calendar_name_active_ = calendar_name_passive_;
        season_profile_active_ = season_profile_passive_;
        week_profile_active_ = week_profile_passive_;
        day_profile_active_ = day_profile_passive_;
    }

    void add_week_profile(const WeekProfile& wp) {
        week_profile_active_.push_back(wp);
    }

    void add_day_profile(const DayProfile& dp) {
        day_profile_active_.push_back(dp);
    }
};

} // namespace dlms::cosem
