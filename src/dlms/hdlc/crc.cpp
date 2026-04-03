#include <dlms/hdlc/crc.hpp>
#include <vector>

namespace dlms::hdlc {

const std::array<uint16_t, 256>& Crc16::table() {
    static std::array<uint16_t, 256> tbl{};
    static bool init = false;
    if (!init) {
        for (int i = 0; i < 256; ++i) {
            uint16_t crc = 0; uint16_t c = i << 8;
            for (int j = 0; j < 8; ++j) {
                if ((crc ^ c) & 0x8000) crc = (crc << 1) ^ POLY;
                else crc = crc << 1;
                c = c << 1;
            }
            tbl[i] = crc & 0xFFFF;
        }
        init = true;
    }
    return tbl;
}

uint8_t Crc16::reverse_bits(uint8_t b) {
    uint8_t r = 0;
    for (int i = 0; i < 8; ++i) {
        r |= ((b >> i) & 1) << (7 - i);
    }
    return r;
}

std::vector<uint8_t> Crc16::reverse_bytes(const uint8_t* data, size_t len) {
    std::vector<uint8_t> out(len);
    for (size_t i = 0; i < len; ++i) out[i] = reverse_bits(data[i]);
    return out;
}

std::array<uint8_t, 2> Crc16::calculate(const uint8_t* data, size_t len, bool lsb_first) {
    auto rev = Crc16::reverse_bytes(data, len);
    uint16_t crc = INIT;
    const auto& tbl = table();
    for (size_t i = 0; i < rev.size(); ++i) {
        uint16_t tmp = ((crc >> 8) & 0xFF) ^ rev[i];
        crc = ((crc << 8) & 0xFF00) ^ tbl[tmp];
    }
    uint8_t lsb = reverse_bits(crc & 0xFF) ^ 0xFF;
    uint8_t msb = reverse_bits((crc >> 8) & 0xFF) ^ 0xFF;
    if (lsb_first) return {lsb, msb};
    return {msb, lsb};
}

bool Crc16::verify(const uint8_t* data, size_t len, const uint8_t* crc, bool lsb_first) {
    auto calc = calculate(data, len, lsb_first);
    return calc[0] == crc[0] && calc[1] == crc[1];
}

} // namespace dlms::hdlc
