#pragma once
#include <dlms/core/types.hpp>
#include <array>
#include <string>
#include <cstdint>
#include <sstream>

namespace dlms {

class ObisCode {
public:
    static constexpr int SIZE = 6;
    using ValueType = std::array<uint8_t, SIZE>;

    ObisCode() : value_{0,0,0,0,0,255} {}
    ObisCode(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f = 255)
        : value_{a,b,c,d,e,f} {}

    static Result<ObisCode> from_bytes(const uint8_t* data, size_t len) {
        if (len < SIZE) return make_err<ObisCode>(DlmsError::InvalidObis);
        ObisCode o;
        for (int i = 0; i < SIZE; ++i) o.value_[i] = data[i];
        return o;
    }

    static Result<ObisCode> from_string(const std::string& s) {
        ObisCode o;
        int vals[SIZE]; char sep;
        std::istringstream iss(s);
        for (int i = 0; i < SIZE; ++i) {
            if (!(iss >> vals[i])) return make_err<ObisCode>(DlmsError::InvalidObis);
            if (i < SIZE - 1) { if (!(iss >> sep)) break; }
        }
        for (int i = 0; i < SIZE; ++i) {
            if (vals[i] < 0 || vals[i] > 255) return make_err<ObisCode>(DlmsError::InvalidObis);
            o.value_[i] = static_cast<uint8_t>(vals[i]);
        }
        return o;
    }

    std::string to_canonical() const {
        std::ostringstream oss;
        oss << (int)value_[0] << "-" << (int)value_[1] << ":"
            << (int)value_[2] << "." << (int)value_[3] << "."
            << (int)value_[4] << "." << (int)value_[5];
        return oss.str();
    }

    const ValueType& value() const { return value_; }
    uint8_t operator[](int i) const { return value_[i]; }
    uint8_t& operator[](int i) { return value_[i]; }
    bool operator==(const ObisCode& o) const { return value_ == o.value_; }
    bool operator!=(const ObisCode& o) const { return value_ != o.value_; }
    bool operator<(const ObisCode& o) const { return value_ < o.value_; }
    std::array<uint8_t, SIZE> to_array() const { return value_; }

private:
    ValueType value_;
};

} // namespace dlms
namespace std {
template<> struct hash<dlms::ObisCode> {
    size_t operator()(const dlms::ObisCode& o) const noexcept {
        size_t h = 0;
        for (int i = 0; i < dlms::ObisCode::SIZE; ++i)
            h ^= std::hash<uint8_t>{}(o[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};
} // namespace std
