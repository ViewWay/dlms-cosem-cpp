#include <dlms/hdlc/address.hpp>
#include <vector>

namespace dlms::hdlc {

size_t HdlcAddress::length() const { return to_bytes().size(); }

std::vector<uint8_t> HdlcAddress::to_bytes() const {
    std::vector<uint8_t> out;
    if (type == Type::Client) {
        out.push_back(static_cast<uint8_t>((logical << 1) | 0x01));
    } else {
        if (physical.has_value()) {
            uint16_t lh = (logical >> 7) & 0xFF, ll = (logical & 0x7F) << 1;
            uint16_t ph = (physical.value() >> 7) & 0xFF, pl = ((physical.value() & 0x7F) << 1) | 0x01;
            if (lh) out.push_back(lh);
            out.push_back(ll);
            if (ph) out.push_back(ph);
            out.push_back(pl);
        } else {
            uint16_t lh = (logical >> 7) & 0xFF, ll = ((logical & 0x7F) << 1) | 0x01;
            if (lh) out.push_back(lh);
            out.push_back(ll);
        }
    }
    return out;
}

HdlcAddress HdlcAddress::from_bytes(const uint8_t* data, size_t len) {
    return parse(data, len, Type::Server);
}

HdlcAddress HdlcAddress::parse(const uint8_t* data, size_t len, Type t) {
    HdlcAddress addr;
    addr.type = t;
    if (len == 0) return addr;

    if (t == Type::Client) {
        addr.logical = data[0] >> 1;
    } else {
        // Server: can be 1, 2, or 4 bytes
        size_t pos = 0;
        // Read logical address
        uint8_t b0 = data[pos++];
        uint8_t upper = (b0 >> 1) & 0x7F;
        bool end = b0 & 0x01;
        if (!end && pos < len) {
            uint8_t b1 = data[pos++];
            addr.logical = upper | (((b1 >> 1) & 0x7F) << 7);
            end = b1 & 0x01;
        } else {
            addr.logical = upper;
        }
        // If 4-byte address (2 bytes logical + 2 bytes physical)
        if (!end && pos < len) {
            // This shouldn't happen for standard 2-byte or 4-byte
        }
        // Read physical address if present
        if (!end && pos + 1 < len) {
            uint8_t b2 = data[pos++];
            uint8_t b3 = data[pos++];
            uint16_t p = (b2 >> 1) | (((b3 >> 1) & 0x7F) << 7);
            addr.physical = p;
        }
    }
    return addr;
}

} // namespace dlms::hdlc
