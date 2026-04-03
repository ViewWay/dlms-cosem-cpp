#include <dlms/asn1/ber.hpp>

namespace dlms::asn1 {

void BerEncoder::encode(uint8_t tag, const uint8_t* value, size_t len) {
    buf_.write_u8(tag);
    buf_.write_variable_length(len);
    if (len > 0) buf_.write_bytes(value, len);
}

void BerEncoder::encode(uint8_t tag, const std::vector<uint8_t>& v) {
    encode(tag, v.data(), v.size());
}

void BerEncoder::encode(uint8_t tag, uint8_t value) { encode(tag, &value, 1); }
void BerEncoder::encode(uint8_t tag, uint16_t value) {
    uint8_t b[2] = {(uint8_t)(value>>8), (uint8_t)value};
    encode(tag, b, 2);
}
void BerEncoder::encode(uint8_t tag, uint32_t value) {
    uint8_t b[4] = {(uint8_t)(value>>24),(uint8_t)(value>>16),(uint8_t)(value>>8),(uint8_t)value};
    encode(tag, b, 4);
}

void BerEncoder::encode_context(uint8_t context, bool constructed, const uint8_t* value, size_t len) {
    uint8_t tag = 0x80 | context | (constructed ? 0x20 : 0x00);
    encode(tag, value, len);
}

BerDecoder::BerDecoder(const uint8_t* data, size_t len) : buf_(data, len) {}
BerDecoder::BerDecoder(const std::vector<uint8_t>& data) : buf_(data.data(), data.size()) {}

Result<BerTlv> BerDecoder::read_tlv() {
    auto tag = buf_.read_u8(); if(!is_ok(tag)) return make_err<BerTlv>(get_err(tag));
    auto len = buf_.read_variable_length(); if(!is_ok(len)) return make_err<BerTlv>(get_err(len));
    auto val = buf_.read_bytes(get_val(len)); if(!is_ok(val)) return make_err<BerTlv>(get_err(val));
    BerTlv tlv; tlv.tag = get_val(tag); tlv.value = get_val(val);
    return tlv;
}

Result<std::vector<uint8_t>> BerDecoder::read_value(uint8_t expected_tag) {
    auto tlv = read_tlv(); if(!is_ok(tlv)) return make_err<std::vector<uint8_t>>(get_err(tlv));
    if (get_val(tlv).tag != expected_tag) return make_err<std::vector<uint8_t>>(DlmsError::InvalidTag);
    return get_val(tlv).value;
}

Result<std::vector<BerTlv>> BerDecoder::read_all() {
    std::vector<BerTlv> result;
    while (buf_.has_data()) {
        auto tlv = read_tlv(); if(!is_ok(tlv)) return make_err<std::vector<BerTlv>>(get_err(tlv));
        result.push_back(get_val(tlv));
    }
    return result;
}

std::vector<uint8_t> AarqApdu::encode() const {
    BerEncoder enc;
    // tag 0x60 = AARQ
    ByteBuffer inner;
    // application-context-name (context 1)
    if (!application_context_name.empty())
        inner.write_u8(0xA1), inner.write_variable_length(application_context_name.size()),
        inner.write_bytes(application_context_name);
    // called-AP-title not used
    // mechanism-name (context 7) + calling-auth-value (context 8)
    if (!mechanism_name.empty()) {
        inner.write_u8(0xA7); inner.write_variable_length(mechanism_name.size()); inner.write_bytes(mechanism_name);
    }
    if (!calling_auth_value.empty()) {
        inner.write_u8(0x88); inner.write_variable_length(calling_auth_value.size()); inner.write_bytes(calling_auth_value);
    }
    // user-information (context 0x80 | 0x20 = 0xBE... actually 0xA8 for context 8)
    if (!user_information.empty()) {
        inner.write_u8(0xBE); inner.write_variable_length(user_information.size()); inner.write_bytes(user_information);
    }
    enc.encode(0x60, inner.data(), inner.size());
    return enc.data();
}

Result<AarqApdu> AarqApdu::decode(const uint8_t* data, size_t len) {
    BerDecoder dec(data, len);
    auto tlv = dec.read_tlv(); if(!is_ok(tlv)) return make_err<AarqApdu>(get_err(tlv));
    if (get_val(tlv).tag != 0x60) return make_err<AarqApdu>(DlmsError::InvalidTag);
    AarqApdu aarq;
    BerDecoder inner(get_val(tlv).value);
    while (inner.has_data()) {
        auto item = inner.read_tlv(); if(!is_ok(item)) return make_err<AarqApdu>(get_err(item));
        auto& t = get_val(item);
        if (t.tag == 0xA1) aarq.application_context_name = t.value;
        else if (t.tag == 0xA7) aarq.mechanism_name = t.value;
        else if (t.tag == 0x88) aarq.calling_auth_value = t.value;
        else if (t.tag == 0xBE) aarq.user_information = t.value;
    }
    return aarq;
}

std::vector<uint8_t> AareApdu::encode() const {
    BerEncoder enc;
    ByteBuffer inner;
    // result (context 0, primitive)
    inner.write_u8(0x82); inner.write_u8(1); inner.write_u8(result);
    if (!application_context_name.empty()) {
        inner.write_u8(0xA1); inner.write_variable_length(application_context_name.size());
        inner.write_bytes(application_context_name);
    }
    if (!responding_auth_value.empty()) {
        inner.write_u8(0x89); inner.write_variable_length(responding_auth_value.size());
        inner.write_bytes(responding_auth_value);
    }
    if (!user_information.empty()) {
        inner.write_u8(0xBE); inner.write_variable_length(user_information.size());
        inner.write_bytes(user_information);
    }
    enc.encode(0x61, inner.data(), inner.size());
    return enc.data();
}

Result<AareApdu> AareApdu::decode(const uint8_t* data, size_t len) {
    BerDecoder dec(data, len);
    auto tlv = dec.read_tlv(); if(!is_ok(tlv)) return make_err<AareApdu>(get_err(tlv));
    if (get_val(tlv).tag != 0x61) return make_err<AareApdu>(DlmsError::InvalidTag);
    AareApdu aare;
    BerDecoder inner(get_val(tlv).value);
    while (inner.has_data()) {
        auto item = inner.read_tlv(); if(!is_ok(item)) return make_err<AareApdu>(get_err(item));
        auto& t = get_val(item);
        if (t.tag == 0x82 && !t.value.empty()) aare.result = t.value[0];
        else if (t.tag == 0xA1) aare.application_context_name = t.value;
        else if (t.tag == 0x89) aare.responding_auth_value = t.value;
        else if (t.tag == 0xBE) aare.user_information = t.value;
    }
    return aare;
}

} // namespace dlms::asn1
