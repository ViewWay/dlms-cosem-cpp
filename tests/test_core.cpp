#include <gtest/gtest.h>
#include <dlms/core/types.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/byte_buffer.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <dlms/core/datetime.hpp>
#include <dlms/core/cosem_object.hpp>
#include <cstdint>
#include <string>
#include <vector>

using namespace dlms;

// ---- ObisCode Tests ----
TEST(ObisCode, DefaultConstructor) {
    ObisCode o;
    EXPECT_EQ(o[0], 0); EXPECT_EQ(o[5], 255);
}
TEST(ObisCode, ParameterizedConstructor) {
    ObisCode o(1,0,1,8,0,255);
    EXPECT_EQ(o[0], 1); EXPECT_EQ(o[4], 0);
}
TEST(ObisCode, FromBytes) {
    uint8_t d[] = {1,0,1,8,0,255};
    auto r = ObisCode::from_bytes(d, 6);
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(get_val(r)[0], 1);
}
TEST(ObisCode, FromBytesTooShort) {
    uint8_t d[] = {1,2};
    auto r = ObisCode::from_bytes(d, 2);
    EXPECT_FALSE(is_ok(r));
}
TEST(ObisCode, FromString) {
    auto r = ObisCode::from_string("1-0:1.8.0.255");
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(get_val(r)[0], 1); EXPECT_EQ(get_val(r)[2], 1);
}
TEST(ObisCode, FromStringDash) {
    auto r = ObisCode::from_string("1-0-1-8-0-255");
    ASSERT_TRUE(is_ok(r));
}
TEST(ObisCode, Equality) {
    ObisCode a(1,0,1,8,0,255), b(1,0,1,8,0,255), c(1,0,2,8,0,255);
    EXPECT_EQ(a, b); EXPECT_NE(a, c);
}
TEST(ObisCode, ToCanonical) {
    ObisCode o(1,0,1,8,0,255);
    EXPECT_EQ(o.to_canonical(), "1-0:1.8.0.255");
}
TEST(ObisCode, ToArray) {
    ObisCode o(1,2,3,4,5,6);
    auto a = o.to_array();
    EXPECT_EQ(a[2], 3);
}

// ---- ByteBuffer Tests ----
TEST(ByteBuffer, WriteReadU8) {
    ByteBuffer buf;
    buf.write_u8(0x42);
    auto r = buf.read_u8();
    ASSERT_TRUE(is_ok(r)); EXPECT_EQ(get_val(r), 0x42);
}
TEST(ByteBuffer, WriteReadI8) {
    ByteBuffer buf; buf.write_i8(-1);
    auto r = buf.read_i8();
    ASSERT_TRUE(is_ok(r)); EXPECT_EQ(get_val(r), -1);
}
TEST(ByteBuffer, WriteReadU16) {
    ByteBuffer buf; buf.write_u16(0x1234);
    auto r = buf.read_u16();
    ASSERT_TRUE(is_ok(r)); EXPECT_EQ(get_val(r), 0x1234);
}
TEST(ByteBuffer, WriteReadI16) {
    ByteBuffer buf; buf.write_i16(-1);
    auto r = buf.read_i16();
    ASSERT_TRUE(is_ok(r)); EXPECT_EQ(get_val(r), -1);
}
TEST(ByteBuffer, WriteReadU32) {
    ByteBuffer buf; buf.write_u32(0x12345678);
    auto r = buf.read_u32();
    ASSERT_TRUE(is_ok(r)); EXPECT_EQ(get_val(r), 0x12345678u);
}
TEST(ByteBuffer, WriteReadI32) {
    ByteBuffer buf; buf.write_i32(-12345);
    auto r = buf.read_i32();
    ASSERT_TRUE(is_ok(r)); EXPECT_EQ(get_val(r), -12345);
}
TEST(ByteBuffer, WriteReadU64) {
    ByteBuffer buf; buf.write_u64(0x0102030405060708ULL);
    auto r = buf.read_u64();
    ASSERT_TRUE(is_ok(r)); EXPECT_EQ(get_val(r), 0x0102030405060708ULL);
}
TEST(ByteBuffer, WriteReadI64) {
    ByteBuffer buf; buf.write_i64(-99999);
    auto r = buf.read_i64();
    ASSERT_TRUE(is_ok(r)); EXPECT_EQ(get_val(r), -99999);
}
TEST(ByteBuffer, ReadBytes) {
    ByteBuffer buf; buf.write_u16(0xABCD);
    auto r = buf.read_bytes(2);
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(get_val(r)[0], 0xAB);
}
TEST(ByteBuffer, ReadVariableLength_Short) {
    ByteBuffer buf; buf.write_u8(0x05);
    buf.set_position(0);
    auto r = buf.read_variable_length();
    ASSERT_TRUE(is_ok(r)); EXPECT_EQ(get_val(r), 5);
}
TEST(ByteBuffer, ReadVariableLength_Long) {
    ByteBuffer buf; buf.write_u8(0x81); buf.write_u8(0x00);
    buf.set_position(0);
    auto r = buf.read_variable_length();
    ASSERT_TRUE(is_ok(r)); EXPECT_EQ(get_val(r), 0);
}
TEST(ByteBuffer, WriteVariableLength) {
    ByteBuffer buf; buf.write_variable_length(128);
    EXPECT_EQ(buf.size(), 2); EXPECT_EQ(buf.vector()[0], 0x81);
}
TEST(ByteBuffer, WriteVariableLength_Short) {
    ByteBuffer buf; buf.write_variable_length(64);
    EXPECT_EQ(buf.size(), 1); EXPECT_EQ(buf.vector()[0], 64);
}
TEST(ByteBuffer, BufferUnderflow) {
    ByteBuffer buf;
    EXPECT_FALSE(is_ok(buf.read_u8()));
}
TEST(ByteBuffer, Position) {
    ByteBuffer buf; buf.write_u8(1); buf.write_u8(2);
    EXPECT_EQ(buf.size(), 2);
    buf.set_position(0);
    auto r = buf.read_u8();
    ASSERT_TRUE(is_ok(r)); EXPECT_EQ(get_val(r), 1);
    EXPECT_EQ(buf.position(), 1);
}
TEST(ByteBuffer, Remaining) {
    ByteBuffer buf; buf.write_u8(1); buf.write_u8(2);
    buf.set_position(1);
    EXPECT_EQ(buf.remaining(), 1);
}
TEST(ByteBuffer, HasData) {
    ByteBuffer buf; buf.write_u8(1);
    EXPECT_TRUE(buf.has_data());
    buf.read_u8();
    EXPECT_FALSE(buf.has_data());
}
TEST(ByteBuffer, Peek) {
    ByteBuffer buf; buf.write_u8(0x42);
    EXPECT_EQ(buf.peek(), 0x42);
    auto r = buf.read_u8(); (void)r;
    EXPECT_EQ(buf.peek(), 0);
}
TEST(ByteBuffer, Clear) {
    ByteBuffer buf; buf.write_u32(42);
    buf.clear();
    EXPECT_EQ(buf.size(), 0);
    EXPECT_FALSE(is_ok(buf.read_u8()));
}
TEST(ByteBuffer, FromData) {
    uint8_t d[] = {1,2,3};
    ByteBuffer buf(d, 3);
    auto r = buf.read_u8();
    ASSERT_TRUE(is_ok(r)); EXPECT_EQ(get_val(r), 1);
}

// ---- DlmsVariant Tests ----
TEST(DlmsVariant, Null) {
    DlmsValue v(DlmsNone{});
    EXPECT_TRUE(is_null(v));
    EXPECT_EQ(get_tag(v), DlmsTag::Null);
}
TEST(DlmsVariant, Bool) {
    DlmsValue v(true);
    EXPECT_TRUE(is_bool(v));
    EXPECT_EQ(get_tag(v), DlmsTag::Boolean);
}
TEST(DlmsVariant, Integers) {
    DlmsValue v8(int8_t(-5)), v16(int16_t(-300)), v32(int32_t(-70000)), v64(int64_t(-1));
    EXPECT_EQ(to_int64(v8), -5);
    EXPECT_EQ(to_int64(v16), -300);
    EXPECT_EQ(to_int64(v32), -70000);
    EXPECT_EQ(to_int64(v64), -1);
}
TEST(DlmsVariant, UnsignedIntegers) {
    DlmsValue v8(uint8_t(200)), v16(uint16_t(60000)), v32(uint32_t(3000000000));
    EXPECT_EQ(to_int64(v8), 200);
    EXPECT_EQ(to_int64(v16), 60000);
}
TEST(DlmsVariant, Floats) {
    DlmsValue f32(3.14f), f64(2.71828);
    EXPECT_EQ(get_tag(f32), DlmsTag::Float32);
    EXPECT_EQ(get_tag(f64), DlmsTag::Float64);
}
TEST(DlmsVariant, OctetString) {
    DlmsValue v(std::vector<uint8_t>{1,2,3});
    EXPECT_TRUE(is_octet_string(v));
    EXPECT_EQ(get_tag(v), DlmsTag::OctetString);
}
TEST(DlmsVariant, String) {
    DlmsValue v(std::string("hello"));
    EXPECT_TRUE(is_string(v));
    EXPECT_EQ(get_tag(v), DlmsTag::VisibleString);
}
TEST(DlmsVariant, Array) {
    DlmsArray arr; arr.items.push_back(std::make_shared<DlmsValue>(DlmsValue(int32_t(42))));
    DlmsValue v(arr);
    EXPECT_TRUE(is_array(v));
    EXPECT_EQ(get_tag(v), DlmsTag::Array);
}
TEST(DlmsVariant, Structure) {
    DlmsStructure st; st.elements.push_back(std::make_shared<DlmsValue>(DlmsValue(true)));
    DlmsValue v(st);
    EXPECT_TRUE(is_structure(v));
    EXPECT_EQ(get_tag(v), DlmsTag::Structure);
}
TEST(DlmsVariant, ToInt64FromNull) {
    DlmsValue v(DlmsNone{});
    EXPECT_EQ(to_int64(v), 0);
}

// ---- ClockStatus Tests ----
TEST(ClockStatus, FromBytes) {
    auto s = ClockStatus::from_bytes(0x83);
    EXPECT_TRUE(s.invalid); EXPECT_TRUE(s.doubtful); EXPECT_FALSE(s.different_base);
}
TEST(ClockStatus, ToByte) {
    ClockStatus s; s.invalid = true; s.daylight_saving_active = true;
    EXPECT_EQ(s.to_byte(), 0x81);
}
TEST(ClockStatus, RoundTrip) {
    ClockStatus s; s.doubtful = true; s.invalid_status = true;
    auto b = s.to_byte();
    auto s2 = ClockStatus::from_bytes(b);
    EXPECT_EQ(s2.doubtful, s.doubtful);
    EXPECT_EQ(s2.invalid_status, s.invalid_status);
}

// ---- CosemDate Tests ----
TEST(CosemDate, FromBytes) {
    uint8_t d[] = {0x07,0xE9,0x01,0x0F,0xFF}; // 2025-1-15
    auto date = CosemDate::from_bytes(d);
    EXPECT_EQ(date.year, 2025); EXPECT_EQ(date.month, 1); EXPECT_EQ(date.day, 15);
    EXPECT_FALSE(date.day_of_week.has_value());
}
TEST(CosemDate, NotSpecified) {
    uint8_t d[] = {0xFF,0xFF,0xFF,0xFF,0xFF};
    auto date = CosemDate::from_bytes(d);
    EXPECT_FALSE(date.year.has_value());
}
TEST(CosemDate, RoundTrip) {
    uint8_t d[5] = {0x07,0xE9,0x06,0x01,0xFF};
    auto date = CosemDate::from_bytes(d);
    uint8_t out[5]; date.to_bytes(out);
    EXPECT_EQ(out[0], 0x07); EXPECT_EQ(out[3], 0x01);
}

// ---- CosemTime Tests ----
TEST(CosemTime, FromBytes) {
    uint8_t d[] = {0x0E,0x1E,0x2A,0x00}; // 14:30:42.00
    auto t = CosemTime::from_bytes(d);
    EXPECT_EQ(t.hour, 14); EXPECT_EQ(t.minute, 30); EXPECT_EQ(t.second, 42);
}
TEST(CosemTime, NotSpecified) {
    uint8_t d[] = {0xFF,0xFF,0xFF,0xFF};
    auto t = CosemTime::from_bytes(d);
    EXPECT_FALSE(t.hour.has_value());
}
TEST(CosemTime, RoundTrip) {
    uint8_t d[4] = {0x12,0x30,0x45,0x50};
    auto t = CosemTime::from_bytes(d);
    uint8_t out[4]; t.to_bytes(out);
    EXPECT_EQ(out[1], 0x30);
}

// ---- CosemDateTime Tests ----
TEST(CosemDateTime, FromBytes) {
    uint8_t data[12] = {0x07,0xE9,0x01,0x0F,0xFF, 0x0E,0x1E,0x00,0x00, 0xFF,0xD0, 0x00};
    auto dt = CosemDateTime::from_bytes(data);
    EXPECT_EQ(dt.date.year, 2025);
    EXPECT_EQ(dt.time.hour, 14);
    EXPECT_TRUE(dt.deviation.has_value());
}
TEST(CosemDateTime, ByteSize) {
    EXPECT_EQ(CosemDateTime::BYTE_SIZE, 12);
}
TEST(CosemDateTime, RoundTrip) {
    uint8_t data[12] = {0x07,0xE9,0x06,0x15,0x05, 0x10,0x20,0x30,0x00, 0xFF,0xD0, 0x00};
    auto dt = CosemDateTime::from_bytes(data);
    uint8_t out[12]; dt.to_bytes(out);
    for(int i=0;i<12;++i) EXPECT_EQ(out[i], data[i]);
}

// ---- DlmsError Tests ----
TEST(DlmsError, Message) {
    EXPECT_NE(dlms_error_message(DlmsError::Success).size(), 0u);
    EXPECT_NE(dlms_error_message(DlmsError::CrcError).size(), 0u);
}
TEST(DlmsError, ErrorCode) {
    auto ec = make_error_code(DlmsError::InvalidData);
    EXPECT_EQ(ec.message(), "invalid data");
}

// ---- Result Tests ----
TEST(Result, Ok) {
    Result<int> r = 42;
    EXPECT_TRUE(is_ok(r));
    EXPECT_EQ(get_val(r), 42);
}
TEST(Result, Err) {
    Result<int> r = make_err<int>(DlmsError::BufferUnderflow);
    EXPECT_FALSE(is_ok(r));
    EXPECT_EQ(get_err(r), DlmsError::BufferUnderflow);
}
