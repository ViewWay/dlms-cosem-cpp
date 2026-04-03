#include <dlms/axdr/decoder.hpp>
#include <cstring>

namespace dlms::axdr {

AxdDecoder::AxdDecoder(const uint8_t* data, size_t len) : buf_(data, len) {}
AxdDecoder::AxdDecoder(const std::vector<uint8_t>& data) : buf_(data.data(), data.size()) {}

Result<DlmsValue> AxdDecoder::decode() {
    auto result = decode_one();
    if (!is_ok(result)) return result;
    return get_val(result);
}

Result<DlmsValue> AxdDecoder::decode_one() {
    auto tag_b = buf_.read_u8();
    if (!is_ok(tag_b)) return make_err<DlmsValue>(get_err(tag_b));
    return decode_by_tag(get_val(tag_b));
}

Result<DlmsValue> AxdDecoder::decode_by_tag(uint8_t tag) {
    switch (static_cast<DlmsTag>(tag)) {
    case DlmsTag::Null: return DlmsValue(DlmsNone{});

    case DlmsTag::Boolean: {
        auto v = buf_.read_u8(); if(!is_ok(v)) return make_err<DlmsValue>(get_err(v));
        return DlmsValue(get_val(v) != 0);
    }
    case DlmsTag::Integer: {
        auto v = buf_.read_i8(); if(!is_ok(v)) return make_err<DlmsValue>(get_err(v));
        return DlmsValue(get_val(v));
    }
    case DlmsTag::Long: {
        auto v = buf_.read_i16(); if(!is_ok(v)) return make_err<DlmsValue>(get_err(v));
        return DlmsValue(get_val(v));
    }
    case DlmsTag::DoubleLong: {
        auto v = buf_.read_i32(); if(!is_ok(v)) return make_err<DlmsValue>(get_err(v));
        return DlmsValue(get_val(v));
    }
    case DlmsTag::Long64: {
        auto v = buf_.read_i64(); if(!is_ok(v)) return make_err<DlmsValue>(get_err(v));
        return DlmsValue(get_val(v));
    }
    case DlmsTag::UnsignedInteger: {
        auto v = buf_.read_u8(); if(!is_ok(v)) return make_err<DlmsValue>(get_err(v));
        return DlmsValue(get_val(v));
    }
    case DlmsTag::UnsignedLong: {
        auto v = buf_.read_u16(); if(!is_ok(v)) return make_err<DlmsValue>(get_err(v));
        return DlmsValue(get_val(v));
    }
    case DlmsTag::DoubleLongUnsigned: {
        auto v = buf_.read_u32(); if(!is_ok(v)) return make_err<DlmsValue>(get_err(v));
        return DlmsValue(get_val(v));
    }
    case DlmsTag::UnsignedLong64: {
        auto v = buf_.read_u64(); if(!is_ok(v)) return make_err<DlmsValue>(get_err(v));
        return DlmsValue(get_val(v));
    }
    case DlmsTag::Float32: {
        auto b = buf_.read_bytes(4); if(!is_ok(b)) return make_err<DlmsValue>(get_err(b));
        float v; memcpy(&v, get_val(b).data(), 4); return DlmsValue(v);
    }
    case DlmsTag::Float64: {
        auto b = buf_.read_bytes(8); if(!is_ok(b)) return make_err<DlmsValue>(get_err(b));
        double v; memcpy(&v, get_val(b).data(), 8); return DlmsValue(v);
    }
    case DlmsTag::OctetString:
    case DlmsTag::BitString: {
        auto len = buf_.read_variable_length(); if(!is_ok(len)) return make_err<DlmsValue>(get_err(len));
        auto b = buf_.read_bytes(get_val(len)); if(!is_ok(b)) return make_err<DlmsValue>(get_err(b));
        return DlmsValue(get_val(b));
    }
    case DlmsTag::VisibleString: {
        auto len = buf_.read_variable_length(); if(!is_ok(len)) return make_err<DlmsValue>(get_err(len));
        auto b = buf_.read_bytes(get_val(len)); if(!is_ok(b)) return make_err<DlmsValue>(get_err(b));
        return DlmsValue(std::string(get_val(b).begin(), get_val(b).end()));
    }
    case DlmsTag::UTF8String: {
        auto len = buf_.read_variable_length(); if(!is_ok(len)) return make_err<DlmsValue>(get_err(len));
        auto b = buf_.read_bytes(get_val(len)); if(!is_ok(b)) return make_err<DlmsValue>(get_err(b));
        return DlmsValue(std::string(get_val(b).begin(), get_val(b).end()));
    }
    case DlmsTag::Enum: {
        auto v = buf_.read_u8(); if(!is_ok(v)) return make_err<DlmsValue>(get_err(v));
        return DlmsValue(get_val(v)); // stored as uint8_t
    }
    case DlmsTag::Date: {
        auto b = buf_.read_bytes(5); if(!is_ok(b)) return make_err<DlmsValue>(get_err(b));
        return DlmsValue(get_val(b)); // raw bytes
    }
    case DlmsTag::Time: {
        auto b = buf_.read_bytes(4); if(!is_ok(b)) return make_err<DlmsValue>(get_err(b));
        return DlmsValue(get_val(b));
    }
    case DlmsTag::DateTime: {
        auto b = buf_.read_bytes(12); if(!is_ok(b)) return make_err<DlmsValue>(get_err(b));
        return DlmsValue(get_val(b));
    }
    case DlmsTag::Array: {
        auto count = buf_.read_variable_length(); if(!is_ok(count)) return make_err<DlmsValue>(get_err(count));
        DlmsArray arr;
        for (size_t i = 0; i < get_val(count) && buf_.has_data(); ++i) {
            auto item = decode_one(); if(!is_ok(item)) return item;
            arr.items.push_back(std::make_shared<DlmsValue>(std::move(get_val(item))));
        }
        return DlmsValue(std::move(arr));
    }
    case DlmsTag::Structure: {
        auto count = buf_.read_variable_length(); if(!is_ok(count)) return make_err<DlmsValue>(get_err(count));
        DlmsStructure st;
        for (size_t i = 0; i < get_val(count) && buf_.has_data(); ++i) {
            auto el = decode_one(); if(!is_ok(el)) return el;
            st.elements.push_back(std::make_shared<DlmsValue>(std::move(get_val(el))));
        }
        return DlmsValue(std::move(st));
    }
    case DlmsTag::BCD: {
        auto len = buf_.read_variable_length(); if(!is_ok(len)) return make_err<DlmsValue>(get_err(len));
        auto b = buf_.read_bytes(get_val(len)); if(!is_ok(b)) return make_err<DlmsValue>(get_err(b));
        return DlmsValue(get_val(b)); // raw BCD bytes
    }
    case DlmsTag::CompactArray:
    case DlmsTag::DontCare:
        return DlmsValue(DlmsNone{});
    default:
        return make_err<DlmsValue>(DlmsError::InvalidTag);
    }
}

} // namespace dlms::axdr
