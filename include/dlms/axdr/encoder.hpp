#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/byte_buffer.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace dlms::axdr {

class AxdEncoder {
public:
    ByteBuffer& buffer() { return buf_; }
    const std::vector<uint8_t>& data() const { return buf_.vector(); }

    void encode_null();
    void encode_bool(bool v);
    void encode_int8(int8_t v);
    void encode_int16(int16_t v);
    void encode_int32(int32_t v);
    void encode_int64(int64_t v);
    void encode_uint8(uint8_t v);
    void encode_uint16(uint16_t v);
    void encode_uint32(uint32_t v);
    void encode_uint64(uint64_t v);
    void encode_float32(float v);
    void encode_float64(double v);
    void encode_octet_string(const uint8_t* data, size_t len);
    void encode_octet_string(const std::vector<uint8_t>& v);
    void encode_string(const std::string& s);
    void encode_enum(uint8_t v);
    void encode_date(const uint8_t* data); // 5 bytes
    void encode_time(const uint8_t* data); // 4 bytes
    void encode_datetime(const uint8_t* data); // 12 bytes
    void encode_array_header(size_t count);
    void encode_structure_header(size_t count);
    void encode_dont_care();

    void encode_value(const DlmsValue& v);

private:
    ByteBuffer buf_;
};

} // namespace dlms::axdr
