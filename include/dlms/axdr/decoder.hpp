#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/byte_buffer.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <cstdint>
#include <vector>

namespace dlms::axdr {

class AxdDecoder {
public:
    explicit AxdDecoder(const uint8_t* data, size_t len);
    explicit AxdDecoder(const std::vector<uint8_t>& data);

    Result<DlmsValue> decode();
    Result<DlmsValue> decode_one();
    bool has_data() const { return buf_.has_data(); }

private:
    Result<DlmsValue> decode_by_tag(uint8_t tag);
    ByteBuffer buf_;
};

} // namespace dlms::axdr
