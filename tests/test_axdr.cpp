#include <gtest/gtest.h>
#include <dlms/axdr/encoder.hpp>
#include <dlms/axdr/decoder.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <vector>
#include <cstdint>
#include <cstring>

using namespace dlms;

void round_trip_test(const DlmsValue& value) {
    axdr::AxdEncoder enc;
    enc.encode_value(value);
    auto data = enc.data();

    axdr::AxdDecoder dec(data);
    auto result = dec.decode();
    ASSERT_TRUE(is_ok(result)) << "Failed to decode";

    auto& decoded = get_val(result);
    EXPECT_EQ(get_tag(decoded), get_tag(value));
}

TEST(AxdEncoder, Null) {
    axdr::AxdEncoder enc;
    enc.encode_null();
    EXPECT_EQ(enc.data()[0], 0x00);
}
TEST(AxdEncoder, Bool) {
    axdr::AxdEncoder enc;
    enc.encode_bool(true);
    EXPECT_EQ(enc.data()[0], 0x03);
    EXPECT_EQ(enc.data()[1], 0x01);
}
TEST(AxdEncoder, Int8) {
    axdr::AxdEncoder enc;
    enc.encode_int8(-5);
    EXPECT_EQ(enc.data()[0], 0x0F);
}
TEST(AxdEncoder, Int16) {
    axdr::AxdEncoder enc;
    enc.encode_int16(1000);
    EXPECT_EQ(enc.data()[0], 0x10);
}
TEST(AxdEncoder, Int32) {
    axdr::AxdEncoder enc;
    enc.encode_int32(-100000);
    EXPECT_EQ(enc.data()[0], 0x05);
}
TEST(AxdEncoder, Int64) {
    axdr::AxdEncoder enc;
    enc.encode_int64(1234567890123LL);
    EXPECT_EQ(enc.data()[0], 0x14);
}
TEST(AxdEncoder, UInt8) {
    axdr::AxdEncoder enc;
    enc.encode_uint8(200);
    EXPECT_EQ(enc.data()[0], 0x11);
}
TEST(AxdEncoder, UInt16) {
    axdr::AxdEncoder enc;
    enc.encode_uint16(60000);
    EXPECT_EQ(enc.data()[0], 0x12);
}
TEST(AxdEncoder, UInt32) {
    axdr::AxdEncoder enc;
    enc.encode_uint32(0xFFFFFFFF);
    EXPECT_EQ(enc.data()[0], 0x06);
}
TEST(AxdEncoder, UInt64) {
    axdr::AxdEncoder enc;
    enc.encode_uint64(0xFFFFFFFFFFFFFFFFULL);
    EXPECT_EQ(enc.data()[0], 0x15);
}
TEST(AxdEncoder, Float32) {
    axdr::AxdEncoder enc;
    float f = 3.14f;
    enc.encode_float32(f);
    EXPECT_EQ(enc.data()[0], 0x17);
}
TEST(AxdEncoder, Float64) {
    axdr::AxdEncoder enc;
    enc.encode_float64(2.71828);
    EXPECT_EQ(enc.data()[0], 0x18);
}
TEST(AxdEncoder, OctetString) {
    axdr::AxdEncoder enc;
    enc.encode_octet_string(std::vector<uint8_t>{1,2,3});
    EXPECT_EQ(enc.data()[0], 0x09);
    EXPECT_EQ(enc.data()[1], 0x03);
}
TEST(AxdEncoder, OctetStringEmpty) {
    axdr::AxdEncoder enc;
    enc.encode_octet_string(std::vector<uint8_t>{});
    EXPECT_EQ(enc.data()[0], 0x09);
    EXPECT_EQ(enc.data()[1], 0x00);
}
TEST(AxdEncoder, String) {
    axdr::AxdEncoder enc;
    enc.encode_string("hello");
    EXPECT_EQ(enc.data()[0], 0x0A);
}
TEST(AxdEncoder, Enum) {
    axdr::AxdEncoder enc;
    enc.encode_enum(42);
    EXPECT_EQ(enc.data()[0], 0x16);
    EXPECT_EQ(enc.data()[1], 42);
}
TEST(AxdEncoder, Date) {
    axdr::AxdEncoder enc;
    uint8_t d[5] = {0x07,0xE9,0x01,0x0F,0xFF};
    enc.encode_date(d);
    EXPECT_EQ(enc.data()[0], 0x1A);
}
TEST(AxdEncoder, Time) {
    axdr::AxdEncoder enc;
    uint8_t t[4] = {0x0E,0x1E,0x2A,0x00};
    enc.encode_time(t);
    EXPECT_EQ(enc.data()[0], 0x1B);
}
TEST(AxdEncoder, DateTime) {
    axdr::AxdEncoder enc;
    uint8_t dt[12] = {};
    enc.encode_datetime(dt);
    EXPECT_EQ(enc.data()[0], 0x19);
}
TEST(AxdEncoder, ArrayHeader) {
    axdr::AxdEncoder enc;
    enc.encode_array_header(5);
    EXPECT_EQ(enc.data()[0], 0x01);
    EXPECT_EQ(enc.data()[1], 0x05);
}
TEST(AxdEncoder, StructureHeader) {
    axdr::AxdEncoder enc;
    enc.encode_structure_header(3);
    EXPECT_EQ(enc.data()[0], 0x02);
}
TEST(AxdEncoder, DontCare) {
    axdr::AxdEncoder enc;
    enc.encode_dont_care();
    EXPECT_EQ(enc.data()[0], 0xFF);
}

// ---- Decoder Tests ----
TEST(AxdDecoder, Null) {
    uint8_t data[] = {0x00};
    axdr::AxdDecoder dec(data, 1);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    EXPECT_TRUE(is_null(get_val(r)));
}
TEST(AxdDecoder, Bool) {
    uint8_t data[] = {0x03, 0x01};
    axdr::AxdDecoder dec(data, 2);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    EXPECT_TRUE(std::holds_alternative<bool>(get_val(r)));
    EXPECT_TRUE(std::get<bool>(get_val(r)));
}
TEST(AxdDecoder, BoolFalse) {
    uint8_t data[] = {0x03, 0x00};
    axdr::AxdDecoder dec(data, 2);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    EXPECT_FALSE(std::get<bool>(get_val(r)));
}
TEST(AxdDecoder, Int8) {
    uint8_t data[] = {0x0F, 0xFB};
    axdr::AxdDecoder dec(data, 2);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(std::get<int8_t>(get_val(r)), -5);
}
TEST(AxdDecoder, Int16) {
    uint8_t data[] = {0x10, 0x03, 0xE8};
    axdr::AxdDecoder dec(data, 3);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(std::get<int16_t>(get_val(r)), 1000);
}
TEST(AxdDecoder, Int32) {
    uint8_t data[] = {0x05, 0x00, 0x00, 0x00, 0x2A};
    axdr::AxdDecoder dec(data, 5);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(std::get<int32_t>(get_val(r)), 42);
}
TEST(AxdDecoder, Int64) {
    uint8_t data[] = {0x14, 0,0,0,0,0,0,0,0x2A};
    axdr::AxdDecoder dec(data, 9);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(std::get<int64_t>(get_val(r)), 42);
}
TEST(AxdDecoder, UInt8) {
    uint8_t data[] = {0x11, 0xC8};
    axdr::AxdDecoder dec(data, 2);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(std::get<uint8_t>(get_val(r)), 200);
}
TEST(AxdDecoder, UInt16) {
    uint8_t data[] = {0x12, 0xEA, 0x60};
    axdr::AxdDecoder dec(data, 3);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(std::get<uint16_t>(get_val(r)), 60000);
}
TEST(AxdDecoder, UInt32) {
    uint8_t data[] = {0x06, 0xFF, 0xFF, 0xFF, 0xFF};
    axdr::AxdDecoder dec(data, 5);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(std::get<uint32_t>(get_val(r)), 0xFFFFFFFF);
}
TEST(AxdDecoder, OctetString) {
    uint8_t data[] = {0x09, 0x03, 0x01, 0x02, 0x03};
    axdr::AxdDecoder dec(data, 5);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    auto& v = std::get<std::vector<uint8_t>>(get_val(r));
    EXPECT_EQ(v.size(), 3u);
}
TEST(AxdDecoder, String) {
    uint8_t data[] = {0x0A, 0x05, 'h','e','l','l','o'};
    axdr::AxdDecoder dec(data, 7);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(std::get<std::string>(get_val(r)), "hello");
}
TEST(AxdDecoder, Enum) {
    uint8_t data[] = {0x16, 0x2A};
    axdr::AxdDecoder dec(data, 2);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(std::get<uint8_t>(get_val(r)), 42);
}
TEST(AxdDecoder, Date) {
    uint8_t data[] = {0x1A, 0x07,0xE9,0x01,0x0F,0xFF};
    axdr::AxdDecoder dec(data, 6);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
}
TEST(AxdDecoder, Time) {
    uint8_t data[] = {0x1B, 0x0E,0x1E,0x2A,0x00};
    axdr::AxdDecoder dec(data, 5);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
}
TEST(AxdDecoder, DateTime) {
    uint8_t data[13] = {0x19};
    for(int i=1;i<13;++i) data[i] = 0;
    axdr::AxdDecoder dec(data, 13);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
}
TEST(AxdDecoder, Array) {
    uint8_t data[] = {0x01, 0x02, 0x0F, 0x05, 0x0F, 0x0A};
    axdr::AxdDecoder dec(data, 6);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    auto& arr = std::get<DlmsArray>(get_val(r));
    EXPECT_EQ(arr.items.size(), 2u);
}
TEST(AxdDecoder, Structure) {
    uint8_t data[] = {0x02, 0x02, 0x03, 0x01, 0x0F, 0x2A};
    axdr::AxdDecoder dec(data, 6);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    auto& st = std::get<DlmsStructure>(get_val(r));
    EXPECT_EQ(st.elements.size(), 2u);
}
TEST(AxdDecoder, DontCare) {
    uint8_t data[] = {0xFF};
    axdr::AxdDecoder dec(data, 1);
    auto r = dec.decode();
    ASSERT_TRUE(is_ok(r));
    EXPECT_TRUE(is_null(get_val(r)));
}
TEST(AxdDecoder, Empty) {
    axdr::AxdDecoder dec(nullptr, 0);
    EXPECT_FALSE(dec.has_data());
    EXPECT_FALSE(is_ok(dec.decode()));
}
TEST(AxdDecoder, InvalidTag) {
    uint8_t data[] = {0x07}; // tag 7 not defined
    axdr::AxdDecoder dec(data, 1);
    auto r = dec.decode();
    EXPECT_FALSE(is_ok(r));
}

// Round-trip tests
TEST(AxdRoundTrip, Null) { round_trip_test(DlmsValue(DlmsNone{})); }
TEST(AxdRoundTrip, Bool) { round_trip_test(DlmsValue(true)); }
TEST(AxdRoundTrip, Int8) { round_trip_test(DlmsValue(int8_t(-100))); }
TEST(AxdRoundTrip, Int16) { round_trip_test(DlmsValue(int16_t(-30000))); }
TEST(AxdRoundTrip, Int32) { round_trip_test(DlmsValue(int32_t(-2000000000))); }
TEST(AxdRoundTrip, UInt8) { round_trip_test(DlmsValue(uint8_t(255))); }
TEST(AxdRoundTrip, UInt16) { round_trip_test(DlmsValue(uint16_t(65535))); }
TEST(AxdRoundTrip, UInt32) { round_trip_test(DlmsValue(uint32_t(4000000000))); }
TEST(AxdRoundTrip, Float32) { round_trip_test(DlmsValue(3.14f)); }
TEST(AxdRoundTrip, Float64) { round_trip_test(DlmsValue(2.71828)); }
TEST(AxdRoundTrip, OctetString) { round_trip_test(DlmsValue(std::vector<uint8_t>{1,2,3,4,5})); }
TEST(AxdRoundTrip, String) { round_trip_test(DlmsValue(std::string("DLMS/COSEM"))); }
TEST(AxdRoundTrip, Array) {
    DlmsArray arr; arr.items.push_back(std::make_shared<DlmsValue>(DlmsValue(int32_t(1)))); arr.items.push_back(std::make_shared<DlmsValue>(DlmsValue(int32_t(2))));
    round_trip_test(DlmsValue(arr));
}
TEST(AxdRoundTrip, Structure) {
    DlmsStructure st; st.elements.push_back(std::make_shared<DlmsValue>(DlmsValue(true))); st.elements.push_back(std::make_shared<DlmsValue>(DlmsValue(int32_t(42))));
    round_trip_test(DlmsValue(st));
}
TEST(AxdRoundTrip, LargeArray) {
    DlmsArray arr;
    for (int i = 0; i < 200; ++i) arr.items.push_back(std::make_shared<DlmsValue>(DlmsValue(int32_t(i))));
    round_trip_test(DlmsValue(arr));
}
TEST(AxdRoundTrip, NestedStructure) {
    DlmsStructure inner; inner.elements.push_back(std::make_shared<DlmsValue>(DlmsValue(int32_t(99))));
    DlmsStructure outer;
    outer.elements.push_back(std::make_shared<DlmsValue>(DlmsValue(inner)));
    outer.elements.push_back(std::make_shared<DlmsValue>(DlmsValue(std::string("test"))));
    round_trip_test(DlmsValue(outer));
}
TEST(AxdRoundTrip, EmptyArray) {
    round_trip_test(DlmsValue(DlmsArray{}));
}
TEST(AxdRoundTrip, EmptyStructure) {
    round_trip_test(DlmsValue(DlmsStructure{}));
}
TEST(AxdRoundTrip, EmptyOctetString) {
    round_trip_test(DlmsValue(std::vector<uint8_t>{}));
}
TEST(AxdRoundTrip, EmptyString) {
    round_trip_test(DlmsValue(std::string("")));
}
TEST(AxdRoundTrip, ZeroInt32) { round_trip_test(DlmsValue(int32_t(0))); }
TEST(AxdRoundTrip, NegativeInt64) { round_trip_test(DlmsValue(int64_t(-1))); }
TEST(AxdRoundTrip, MaxUInt64) { round_trip_test(DlmsValue(uint64_t(0xFFFFFFFFFFFFFFFFULL))); }
