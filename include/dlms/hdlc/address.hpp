#pragma once
#include <dlms/core/types.hpp>
#include <cstdint>
#include <optional>

namespace dlms::hdlc {

struct HdlcAddress {
    uint16_t logical = 0;
    std::optional<uint16_t> physical;
    enum class Type { Client, Server } type = Type::Client;

    static HdlcAddress from_bytes(const uint8_t* data, size_t len);
    std::vector<uint8_t> to_bytes() const;
    size_t length() const;

    static HdlcAddress parse(const uint8_t* data, size_t len, Type t);
};

} // namespace dlms::hdlc
