#pragma once
#include <cstdint>
#include <array>
#include <vector>

namespace dlms::hdlc {

class Crc16 {
public:
    static constexpr uint16_t POLY = 0x1021;
    static constexpr uint16_t INIT = 0xFFFF;

    static std::array<uint8_t, 2> calculate(const uint8_t* data, size_t len, bool lsb_first=true);
    static bool verify(const uint8_t* data, size_t len, const uint8_t* crc, bool lsb_first=true);

private:
    static const std::array<uint16_t, 256>& table();
    static uint8_t reverse_bits(uint8_t b);
    static std::vector<uint8_t> reverse_bytes(const uint8_t* data, size_t len);
};

} // namespace dlms::hdlc
