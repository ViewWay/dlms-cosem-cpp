#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/byte_buffer.hpp>
#include <cstdint>
#include <vector>
#include <optional>

namespace dlms::asn1 {

struct BerTlv {
    uint8_t tag = 0;
    std::vector<uint8_t> value;
};

class BerEncoder {
public:
    ByteBuffer& buffer() { return buf_; }
    void encode(uint8_t tag, const uint8_t* value, size_t len);
    void encode(uint8_t tag, const std::vector<uint8_t>& v);
    void encode(uint8_t tag, uint8_t value);
    void encode(uint8_t tag, uint16_t value);
    void encode(uint8_t tag, uint32_t value);
    void encode_context(uint8_t context, bool constructed, const uint8_t* value, size_t len);
    std::vector<uint8_t> data() const { return buf_.vector(); }
private:
    ByteBuffer buf_;
};

class BerDecoder {
public:
    explicit BerDecoder(const uint8_t* data, size_t len);
    explicit BerDecoder(const std::vector<uint8_t>& data);
    Result<BerTlv> read_tlv();
    bool has_data() const { return buf_.has_data(); }
    Result<std::vector<uint8_t>> read_value(uint8_t expected_tag);
    Result<std::vector<BerTlv>> read_all();
private:
    ByteBuffer buf_;
};

// ACSE: AARQ / AARE / RLRQ / RLRE
struct AarqApdu {
    std::vector<uint8_t> application_context_name;
    std::vector<uint8_t> calling_auth_value;
    std::vector<uint8_t> mechanism_name;
    std::vector<uint8_t> user_information;

    std::vector<uint8_t> encode() const;
    static Result<AarqApdu> decode(const uint8_t* data, size_t len);
};

struct AareApdu {
    uint8_t result = 0;
    std::vector<uint8_t> application_context_name;
    std::vector<uint8_t> responding_auth_value;
    std::vector<uint8_t> mechanism_name;
    std::vector<uint8_t> user_information;

    std::vector<uint8_t> encode() const;
    static Result<AareApdu> decode(const uint8_t* data, size_t len);
};

} // namespace dlms::asn1
