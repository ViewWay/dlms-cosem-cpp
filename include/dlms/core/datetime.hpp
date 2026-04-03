#pragma once
#include <dlms/core/types.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <sstream>

namespace dlms {

struct ClockStatus {
    bool invalid=false, doubtful=false, different_base=false, invalid_status=false, daylight_saving_active=false;
    static ClockStatus from_bytes(uint8_t b){
        ClockStatus s; s.invalid=b&0x01; s.doubtful=b&0x02; s.different_base=b&0x04;
        s.invalid_status=b&0x08; s.daylight_saving_active=b&0x80; return s;
    }
    uint8_t to_byte()const{
        uint8_t b=0; if(invalid)b|=0x01; if(doubtful)b|=0x02; if(different_base)b|=0x04;
        if(invalid_status)b|=0x08; if(daylight_saving_active)b|=0x80; return b;
    }
};

struct CosemDate {
    static constexpr uint16_t NOT_SPECIFIED_YEAR=0xFFFF;
    static constexpr uint8_t NOT_SPECIFIED=0xFF;
    std::optional<uint16_t> year; std::optional<uint8_t> month, day, day_of_week;

    static CosemDate from_bytes(const uint8_t* d){
        CosemDate r; uint16_t y=(uint16_t(d[0])<<8)|d[1];
        r.year=(y==NOT_SPECIFIED_YEAR)?std::nullopt:std::optional<uint16_t>(y);
        r.month=(d[2]==NOT_SPECIFIED)?std::nullopt:std::optional<uint8_t>(d[2]);
        r.day=(d[3]==NOT_SPECIFIED)?std::nullopt:std::optional<uint8_t>(d[3]);
        r.day_of_week=(d[4]==NOT_SPECIFIED)?std::nullopt:std::optional<uint8_t>(d[4]);
        return r;
    }
    void to_bytes(uint8_t* o)const{
        uint16_t y=year.value_or(NOT_SPECIFIED_YEAR);
        o[0]=(y>>8)&0xFF;o[1]=y&0xFF;o[2]=month.value_or(NOT_SPECIFIED);
        o[3]=day.value_or(NOT_SPECIFIED);o[4]=day_of_week.value_or(NOT_SPECIFIED);
    }
    std::string to_string()const{
        std::ostringstream oss;
        oss<<(year.value_or(0xFFFF))<<"-"<<(int)month.value_or(0xFF)<<"-"<<(int)day.value_or(0xFF);
        return oss.str();
    }
};

struct CosemTime {
    static constexpr uint8_t NOT_SPECIFIED=0xFF;
    std::optional<uint8_t> hour, minute, second, hundredths;

    static CosemTime from_bytes(const uint8_t* d){
        CosemTime t; t.hour=(d[0]==NOT_SPECIFIED)?std::nullopt:std::optional<uint8_t>(d[0]);
        t.minute=(d[1]==NOT_SPECIFIED)?std::nullopt:std::optional<uint8_t>(d[1]);
        t.second=(d[2]==NOT_SPECIFIED)?std::nullopt:std::optional<uint8_t>(d[2]);
        t.hundredths=(d[3]==NOT_SPECIFIED)?std::nullopt:std::optional<uint8_t>(d[3]);
        return t;
    }
    void to_bytes(uint8_t* o)const{
        o[0]=hour.value_or(NOT_SPECIFIED);o[1]=minute.value_or(NOT_SPECIFIED);
        o[2]=second.value_or(NOT_SPECIFIED);o[3]=hundredths.value_or(NOT_SPECIFIED);
    }
    std::string to_string()const{
        std::ostringstream oss;
        oss<<(int)hour.value_or(0)<<":"<<(int)minute.value_or(0)<<":"<<(int)second.value_or(0);
        return oss.str();
    }
};

struct CosemDateTime {
    static constexpr int16_t DEV_NOT_SPEC=-32768;
    static constexpr size_t BYTE_SIZE=12;
    CosemDate date; CosemTime time;
    std::optional<int16_t> deviation;
    ClockStatus clock_status;

    static CosemDateTime from_bytes(const uint8_t* d){
        CosemDateTime dt; dt.date=CosemDate::from_bytes(d); dt.time=CosemTime::from_bytes(d+5);
        int16_t dev=(int16_t(d[9])<<8)|d[10];
        dt.deviation=(dev==DEV_NOT_SPEC)?std::nullopt:std::optional<int16_t>(dev);
        dt.clock_status=ClockStatus::from_bytes(d[11]); return dt;
    }
    void to_bytes(uint8_t* o)const{
        date.to_bytes(o); time.to_bytes(o+5);
        int16_t dev=deviation.value_or(DEV_NOT_SPEC);
        o[9]=(dev>>8)&0xFF;o[10]=dev&0xFF;o[11]=clock_status.to_byte();
    }
    std::string to_string()const{return date.to_string()+" "+time.to_string();}
};

} // namespace dlms
