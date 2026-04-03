#include <dlms/axdr/encoder.hpp>
#include <cstring>

namespace dlms::axdr {

void AxdEncoder::encode_null() { buf_.write_u8(static_cast<uint8_t>(DlmsTag::Null)); }

void AxdEncoder::encode_bool(bool v) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::Boolean));
    buf_.write_u8(v ? 1 : 0);
}

void AxdEncoder::encode_int8(int8_t v) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::Integer));
    buf_.write_i8(v);
}

void AxdEncoder::encode_int16(int16_t v) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::Long));
    buf_.write_i16(v);
}

void AxdEncoder::encode_int32(int32_t v) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::DoubleLong));
    buf_.write_i32(v);
}

void AxdEncoder::encode_int64(int64_t v) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::Long64));
    buf_.write_i64(v);
}

void AxdEncoder::encode_uint8(uint8_t v) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::UnsignedInteger));
    buf_.write_u8(v);
}

void AxdEncoder::encode_uint16(uint16_t v) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::UnsignedLong));
    buf_.write_u16(v);
}

void AxdEncoder::encode_uint32(uint32_t v) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::DoubleLongUnsigned));
    buf_.write_u32(v);
}

void AxdEncoder::encode_uint64(uint64_t v) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::UnsignedLong64));
    buf_.write_u64(v);
}

void AxdEncoder::encode_float32(float v) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::Float32));
    buf_.write_bytes(reinterpret_cast<const uint8_t*>(&v), 4);
}

void AxdEncoder::encode_float64(double v) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::Float64));
    buf_.write_bytes(reinterpret_cast<const uint8_t*>(&v), 8);
}

void AxdEncoder::encode_octet_string(const uint8_t* data, size_t len) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::OctetString));
    buf_.write_variable_length(len);
    buf_.write_bytes(data, len);
}

void AxdEncoder::encode_octet_string(const std::vector<uint8_t>& v) {
    encode_octet_string(v.data(), v.size());
}

void AxdEncoder::encode_string(const std::string& s) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::VisibleString));
    buf_.write_variable_length(s.size());
    buf_.write_string(s);
}

void AxdEncoder::encode_enum(uint8_t v) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::Enum));
    buf_.write_u8(v);
}

void AxdEncoder::encode_date(const uint8_t* data) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::Date));
    buf_.write_bytes(data, 5);
}

void AxdEncoder::encode_time(const uint8_t* data) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::Time));
    buf_.write_bytes(data, 4);
}

void AxdEncoder::encode_datetime(const uint8_t* data) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::DateTime));
    buf_.write_bytes(data, 12);
}

void AxdEncoder::encode_array_header(size_t count) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::Array));
    buf_.write_variable_length(count);
}

void AxdEncoder::encode_structure_header(size_t count) {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::Structure));
    buf_.write_variable_length(count);
}

void AxdEncoder::encode_dont_care() {
    buf_.write_u8(static_cast<uint8_t>(DlmsTag::DontCare));
}

void AxdEncoder::encode_value(const DlmsValue& v) {
    if (std::holds_alternative<DlmsNone>(v)) { encode_null(); return; }
    if (auto* p = std::get_if<bool>(&v)) { encode_bool(*p); return; }
    if (auto* p = std::get_if<int8_t>(&v)) { encode_int8(*p); return; }
    if (auto* p = std::get_if<int16_t>(&v)) { encode_int16(*p); return; }
    if (auto* p = std::get_if<int32_t>(&v)) { encode_int32(*p); return; }
    if (auto* p = std::get_if<int64_t>(&v)) { encode_int64(*p); return; }
    if (auto* p = std::get_if<uint8_t>(&v)) { encode_uint8(*p); return; }
    if (auto* p = std::get_if<uint16_t>(&v)) { encode_uint16(*p); return; }
    if (auto* p = std::get_if<uint32_t>(&v)) { encode_uint32(*p); return; }
    if (auto* p = std::get_if<uint64_t>(&v)) { encode_uint64(*p); return; }
    if (auto* p = std::get_if<float>(&v)) { encode_float32(*p); return; }
    if (auto* p = std::get_if<double>(&v)) { encode_float64(*p); return; }
    if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) { encode_octet_string(*p); return; }
    if (auto* p = std::get_if<std::string>(&v)) { encode_string(*p); return; }
    if (auto* p = std::get_if<DlmsArray>(&v)) {
        encode_array_header(p->items.size());
        for (auto& item : p->items) encode_value(*item);
        return;
    }
    if (auto* p = std::get_if<DlmsStructure>(&v)) {
        encode_structure_header(p->elements.size());
        for (auto& el : p->elements) encode_value(*el);
        return;
    }
}

} // namespace dlms::axdr
