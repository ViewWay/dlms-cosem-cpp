#include <gtest/gtest.h>
#include <dlms/asn1/ber.hpp>
#include <vector>
#include <cstdint>

using namespace dlms::asn1;

// ---- BER Encoder Tests ----
TEST(BerEncoder, SingleByte) {
    BerEncoder enc;
    uint8_t val = 0x42;
    enc.encode(0x02, &val, 1);
    auto data = enc.data();
    ASSERT_EQ(data.size(), 3u);
    EXPECT_EQ(data[0], 0x02); EXPECT_EQ(data[1], 0x01); EXPECT_EQ(data[2], 0x42);
}
TEST(BerEncoder, EmptyValue) {
    BerEncoder enc;
    enc.encode(0x05, nullptr, 0);
    auto data = enc.data();
    ASSERT_EQ(data.size(), 2u);
    EXPECT_EQ(data[1], 0x00);
}
TEST(BerEncoder, U16) {
    BerEncoder enc;
    enc.encode(0x02, uint16_t(0x1234));
    auto data = enc.data();
    EXPECT_EQ(data.size(), 4u);
}
TEST(BerEncoder, U32) {
    BerEncoder enc;
    enc.encode(0x02, uint32_t(0x12345678));
    auto data = enc.data();
    EXPECT_EQ(data.size(), 6u);
}
TEST(BerEncoder, Vector) {
    BerEncoder enc;
    enc.encode(0x04, std::vector<uint8_t>{1,2,3,4});
    auto data = enc.data();
    EXPECT_EQ(data.size(), 6u);
}
TEST(BerEncoder, ContextConstructed) {
    BerEncoder enc;
    uint8_t val = 0x01;
    enc.encode_context(1, true, &val, 1);
    auto data = enc.data();
    EXPECT_EQ(data[0], 0xA1);
}
TEST(BerEncoder, ContextPrimitive) {
    BerEncoder enc;
    uint8_t val = 0x02;
    enc.encode_context(7, false, &val, 1);
    auto data = enc.data();
    EXPECT_EQ(data[0], 0x87);
}
TEST(BerEncoder, LongLength) {
    BerEncoder enc;
    std::vector<uint8_t> val(200, 0x00);
    enc.encode(0x04, val);
    auto data = enc.data();
    EXPECT_EQ(data[0], 0x04);
    EXPECT_EQ(data[1], 0x81);
}

// ---- BER Decoder Tests ----
TEST(BerDecoder, SingleTlv) {
    uint8_t data[] = {0x02, 0x01, 0x42};
    BerDecoder dec(data, 3);
    auto r = dec.read_tlv();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(get_val(r).tag, 0x02);
    ASSERT_EQ(get_val(r).value.size(), 1u);
    EXPECT_EQ(get_val(r).value[0], 0x42);
}
TEST(BerDecoder, EmptyTlv) {
    uint8_t data[] = {0x05, 0x00};
    BerDecoder dec(data, 2);
    auto r = dec.read_tlv();
    ASSERT_TRUE(is_ok(r));
    EXPECT_TRUE(get_val(r).value.empty());
}
TEST(BerDecoder, MultiByteLength) {
    std::vector<uint8_t> data = {0x04, 0x81, 0x80};
    data.resize(3 + 128, 0x00);
    BerDecoder dec(data);
    auto r = dec.read_tlv();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(get_val(r).value.size(), 128u);
}
TEST(BerDecoder, ReadValue) {
    uint8_t data[] = {0x02, 0x01, 0x05};
    BerDecoder dec(data, 3);
    auto r = dec.read_value(0x02);
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(get_val(r)[0], 0x05);
}
TEST(BerDecoder, ReadValueWrongTag) {
    uint8_t data[] = {0x02, 0x01, 0x05};
    BerDecoder dec(data, 3);
    auto r = dec.read_value(0x04);
    EXPECT_FALSE(is_ok(r));
}
TEST(BerDecoder, ReadAll) {
    uint8_t data[] = {0x02, 0x01, 0x05, 0x04, 0x02, 0xAB, 0xCD};
    BerDecoder dec(data, 7);
    auto r = dec.read_all();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(get_val(r).size(), 2u);
}
TEST(BerDecoder, Empty) {
    BerDecoder dec(nullptr, 0);
    EXPECT_FALSE(dec.has_data());
    EXPECT_FALSE(is_ok(dec.read_tlv()));
}
TEST(BerDecoder, BufferUnderflow) {
    uint8_t data[] = {0x02};
    BerDecoder dec(data, 1);
    EXPECT_FALSE(is_ok(dec.read_tlv()));
}

// ---- AARQ Tests ----
TEST(AarqApdu, Encode) {
    AarqApdu aarq;
    aarq.application_context_name = {0x06, 0x00, 0x00, 0x01};
    auto data = aarq.encode();
    EXPECT_EQ(data[0], 0x60);
}
TEST(AarqApdu, EncodeWithAuth) {
    AarqApdu aarq;
    aarq.calling_auth_value = {0x31, 0x32, 0x33};
    auto data = aarq.encode();
    EXPECT_EQ(data[0], 0x60);
}
TEST(AarqApdu, Decode) {
    uint8_t data[] = {0x60, 0x08, 0xA1, 0x06, 0x06, 0x00, 0x00, 0x01, 0x00, 0x00};
    auto r = AarqApdu::decode(data, 10);
    ASSERT_TRUE(is_ok(r));
    EXPECT_FALSE(get_val(r).application_context_name.empty());
}
TEST(AarqApdu, DecodeInvalidTag) {
    uint8_t data[] = {0x61, 0x02, 0x02, 0x01, 0x00};
    auto r = AarqApdu::decode(data, 5);
    EXPECT_FALSE(is_ok(r));
}

// ---- AARE Tests ----
TEST(AareApdu, Encode) {
    AareApdu aare; aare.result = 0;
    auto data = aare.encode();
    EXPECT_EQ(data[0], 0x61);
}
TEST(AareApdu, Decode) {
    uint8_t data[] = {0x61, 0x03, 0x82, 0x01, 0x00};
    auto r = AareApdu::decode(data, 5);
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(get_val(r).result, 0);
}
TEST(AareApdu, DecodeRejected) {
    uint8_t data[] = {0x61, 0x03, 0x82, 0x01, 0x01};
    auto r = AareApdu::decode(data, 5);
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(get_val(r).result, 1);
}
TEST(AareApdu, RoundTrip) {
    AareApdu aare; aare.result = 0;
    aare.application_context_name = {0x06, 0x00, 0x00, 0x01};
    auto data = aare.encode();
    auto r = AareApdu::decode(data.data(), data.size());
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(get_val(r).result, 0);
    EXPECT_FALSE(get_val(r).application_context_name.empty());
}

// ---- BER Round Trip ----
TEST(BerRoundTrip, SingleByte) {
    BerEncoder enc;
    enc.encode(0x02, uint8_t(0x42));
    auto data = enc.data();
    BerDecoder dec(data);
    auto r = dec.read_tlv();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(get_val(r).value[0], 0x42);
}
TEST(BerRoundTrip, MultiByte) {
    BerEncoder enc;
    enc.encode(0x04, std::vector<uint8_t>{0xDE, 0xAD, 0xBE, 0xEF});
    auto data = enc.data();
    BerDecoder dec(data);
    auto r = dec.read_tlv();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(get_val(r).value.size(), 4u);
}
