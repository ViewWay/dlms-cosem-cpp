#pragma once
#include <dlms/core/types.hpp>
#include <variant>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace dlms {

struct DlmsNone {};

// Use shared_ptr to break the recursive variant cycle
// DlmsValue is forward-declared, then defined with boxed array/structure types
struct DlmsValue;

struct DlmsArray {
    std::vector<std::shared_ptr<DlmsValue>> items;
};
struct DlmsStructure {
    std::vector<std::shared_ptr<DlmsValue>> elements;
};

struct DlmsValue : std::variant<
    DlmsNone, bool, int8_t, int16_t, int32_t, int64_t,
    uint8_t, uint16_t, uint32_t, uint64_t, float, double,
    std::vector<uint8_t>, std::string, DlmsArray, DlmsStructure
> {
    using Base = std::variant<
        DlmsNone, bool, int8_t, int16_t, int32_t, int64_t,
        uint8_t, uint16_t, uint32_t, uint64_t, float, double,
        std::vector<uint8_t>, std::string, DlmsArray, DlmsStructure>;
    using Base::variant;
    using Base::operator=;
};

struct EnumData { uint8_t value; };

inline bool is_null(const DlmsValue& v){return std::holds_alternative<DlmsNone>(v);}
inline bool is_bool(const DlmsValue& v){return std::holds_alternative<bool>(v);}
inline bool is_array(const DlmsValue& v){return std::holds_alternative<DlmsArray>(v);}
inline bool is_structure(const DlmsValue& v){return std::holds_alternative<DlmsStructure>(v);}
inline bool is_octet_string(const DlmsValue& v){return std::holds_alternative<std::vector<uint8_t>>(v);}
inline bool is_string(const DlmsValue& v){return std::holds_alternative<std::string>(v);}

inline int64_t to_int64(const DlmsValue& v){
    if(auto*p=std::get_if<int8_t>(&v))return *p;
    if(auto*p=std::get_if<int16_t>(&v))return *p;
    if(auto*p=std::get_if<int32_t>(&v))return *p;
    if(auto*p=std::get_if<int64_t>(&v))return *p;
    if(auto*p=std::get_if<uint8_t>(&v))return *p;
    if(auto*p=std::get_if<uint16_t>(&v))return *p;
    if(auto*p=std::get_if<uint32_t>(&v))return *p;
    if(auto*p=std::get_if<uint64_t>(&v))return static_cast<int64_t>(*p);
    return 0;
}

inline DlmsTag get_tag(const DlmsValue& v){
    if(std::holds_alternative<DlmsNone>(v))return DlmsTag::Null;
    if(std::holds_alternative<bool>(v))return DlmsTag::Boolean;
    if(std::holds_alternative<int8_t>(v))return DlmsTag::Integer;
    if(std::holds_alternative<int16_t>(v))return DlmsTag::Long;
    if(std::holds_alternative<int32_t>(v))return DlmsTag::DoubleLong;
    if(std::holds_alternative<int64_t>(v))return DlmsTag::Long64;
    if(std::holds_alternative<uint8_t>(v))return DlmsTag::UnsignedInteger;
    if(std::holds_alternative<uint16_t>(v))return DlmsTag::UnsignedLong;
    if(std::holds_alternative<uint32_t>(v))return DlmsTag::DoubleLongUnsigned;
    if(std::holds_alternative<uint64_t>(v))return DlmsTag::UnsignedLong64;
    if(std::holds_alternative<float>(v))return DlmsTag::Float32;
    if(std::holds_alternative<double>(v))return DlmsTag::Float64;
    if(std::holds_alternative<std::vector<uint8_t>>(v))return DlmsTag::OctetString;
    if(std::holds_alternative<std::string>(v))return DlmsTag::VisibleString;
    if(std::holds_alternative<DlmsArray>(v))return DlmsTag::Array;
    if(std::holds_alternative<DlmsStructure>(v))return DlmsTag::Structure;
    return DlmsTag::DontCare;
}

} // namespace dlms
