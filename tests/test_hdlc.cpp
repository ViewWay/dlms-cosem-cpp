#include <gtest/gtest.h>
#include <dlms/hdlc/crc.hpp>
#include <dlms/hdlc/address.hpp>
#include <dlms/hdlc/frame.hpp>
#include <vector>
#include <cstdint>

using namespace dlms;
using namespace dlms::hdlc;

// ---- CRC Tests ----
TEST(Crc16, Calculate) {
    uint8_t data[] = {0x41, 0x42};
    auto crc = Crc16::calculate(data, 2);
    EXPECT_EQ(crc.size(), 2);
}
TEST(Crc16, Empty) {
    auto crc = Crc16::calculate(nullptr, 0);
    EXPECT_EQ(crc.size(), 2);
}
TEST(Crc16, Verify) {
    uint8_t data[] = {0x41, 0x42, 0x43};
    auto crc = Crc16::calculate(data, 3);
    EXPECT_TRUE(Crc16::verify(data, 3, crc.data()));
}
TEST(Crc16, DetectCorruption) {
    uint8_t data[] = {0x41};
    auto crc = Crc16::calculate(data, 1);
    uint8_t bad_crc[] = {static_cast<uint8_t>(crc[0] ^ 0xFF), crc[1]};
    EXPECT_FALSE(Crc16::verify(data, 1, bad_crc));
}
TEST(Crc16, MSBFirst) {
    uint8_t data[] = {0x01, 0x02};
    auto lsb = Crc16::calculate(data, 2, true);
    auto msb = Crc16::calculate(data, 2, false);
    EXPECT_EQ(lsb[0], msb[1]);
    EXPECT_EQ(lsb[1], msb[0]);
}
TEST(Crc16, Consistency) {
    uint8_t data[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
    auto c1 = Crc16::calculate(data, 8);
    auto c2 = Crc16::calculate(data, 8);
    EXPECT_EQ(c1[0], c2[0]); EXPECT_EQ(c1[1], c2[1]);
}
TEST(Crc16, SingleByte) {
    uint8_t data[] = {0x00};
    auto crc = Crc16::calculate(data, 1);
    EXPECT_NE(crc[0], 0); EXPECT_NE(crc[1], 0);
}
TEST(Crc16, AllFF) {
    std::vector<uint8_t> data(10, 0xFF);
    auto crc = Crc16::calculate(data.data(), data.size());
    EXPECT_EQ(crc.size(), 2);
}
TEST(Crc16, AllZero) {
    std::vector<uint8_t> data(10, 0x00);
    auto crc = Crc16::calculate(data.data(), data.size());
    EXPECT_EQ(crc.size(), 2);
}

// ---- HDLC Escape Tests ----
TEST(HdlcEscape, Flag) {
    uint8_t data[] = {0x7E};
    auto out = hdlc_escape(data, 1);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], 0x7D); EXPECT_EQ(out[1], 0x5E);
}
TEST(HdlcEscape, Escape) {
    uint8_t data[] = {0x7D};
    auto out = hdlc_escape(data, 1);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[1], 0x5D);
}
TEST(HdlcEscape, Normal) {
    uint8_t data[] = {0x01, 0x02};
    auto out = hdlc_escape(data, 2);
    EXPECT_EQ(out.size(), 2u);
}
TEST(HdlcUnescape, RoundTrip) {
    uint8_t data[] = {0x7E, 0x01, 0x7D, 0x02, 0x7E};
    auto escaped = hdlc_escape(data + 1, 3);
    auto unescaped = hdlc_unescape(escaped.data(), escaped.size());
    ASSERT_EQ(unescaped.size(), 3u);
    EXPECT_EQ(unescaped[0], 0x01); EXPECT_EQ(unescaped[1], 0x7D); EXPECT_EQ(unescaped[2], 0x02);
}
TEST(HdlcUnescape, Empty) {
    auto out = hdlc_unescape(nullptr, 0);
    EXPECT_TRUE(out.empty());
}

// ---- HDLC Address Tests ----
TEST(HdlcAddress, ClientAddress) {
    HdlcAddress addr;
    addr.type = HdlcAddress::Type::Client;
    addr.logical = 0x10;
    auto bytes = addr.to_bytes();
    ASSERT_EQ(bytes.size(), 1u);
    EXPECT_EQ(bytes[0], 0x21);
}
TEST(HdlcAddress, ServerAddress) {
    HdlcAddress addr;
    addr.type = HdlcAddress::Type::Server;
    addr.logical = 0x01;
    auto bytes = addr.to_bytes();
    ASSERT_EQ(bytes.size(), 1u);
    EXPECT_EQ(bytes[0], 0x03);
}
TEST(HdlcAddress, ServerWithPhysical) {
    HdlcAddress addr;
    addr.type = HdlcAddress::Type::Server;
    addr.logical = 0x01;
    addr.physical = 0x01;
    auto bytes = addr.to_bytes();
    EXPECT_GE(bytes.size(), 2u);
}
TEST(HdlcAddress, Length) {
    HdlcAddress addr;
    addr.type = HdlcAddress::Type::Client;
    addr.logical = 0x01;
    EXPECT_EQ(addr.length(), 1u);
}
TEST(HdlcAddress, ParseClient) {
    uint8_t data[] = {static_cast<uint8_t>(0x01)}; // logical=0
    auto addr = HdlcAddress::parse(data, 1, HdlcAddress::Type::Client);
    EXPECT_EQ(addr.logical, 0);
}
TEST(HdlcAddress, ParseServer1Byte) {
    uint8_t data[] = {0x03}; // logical=1
    auto addr = HdlcAddress::parse(data, 1, HdlcAddress::Type::Server);
    EXPECT_EQ(addr.logical, 1);
}

// ---- FrameFormat Tests ----
TEST(FrameFormat, FromBytes) {
    ByteBuffer buf;
    buf.write_u16(0xA008); // format=0xA, length=8, not segmented
    buf.set_position(0);
    auto fmt = FrameFormat::from_bytes(buf);
    ASSERT_TRUE(is_ok(fmt));
    EXPECT_EQ(get_val(fmt).length, 8);
    EXPECT_FALSE(get_val(fmt).segmented);
}
TEST(FrameFormat, Segmented) {
    ByteBuffer buf;
    buf.write_u16(0xA808); // segmented bit set
    buf.set_position(0);
    auto fmt = FrameFormat::from_bytes(buf);
    ASSERT_TRUE(is_ok(fmt));
    EXPECT_TRUE(get_val(fmt).segmented);
}
TEST(FrameFormat, InvalidFormat) {
    ByteBuffer buf;
    buf.write_u16(0x0008); // wrong format bits
    buf.set_position(0);
    auto fmt = FrameFormat::from_bytes(buf);
    EXPECT_FALSE(is_ok(fmt));
}
TEST(FrameFormat, RoundTrip) {
    FrameFormat f{100, true};
    ByteBuffer buf; f.to_bytes(buf);
    buf.set_position(0);
    auto fmt = FrameFormat::from_bytes(buf);
    ASSERT_TRUE(is_ok(fmt));
    EXPECT_EQ(get_val(fmt).length, 100);
}

// ---- ControlField Tests ----
TEST(ControlField, SNRM) {
    auto cf = ControlField::from_byte(0x93);
    EXPECT_EQ(cf.type, FrameType::SNRM);
    EXPECT_TRUE(cf.final);
}
TEST(ControlField, UA) {
    auto cf = ControlField::from_byte(0x73);
    EXPECT_EQ(cf.type, FrameType::UA);
}
TEST(ControlField, RR) {
    auto cf = ControlField::from_byte(0x01);
    EXPECT_EQ(cf.type, FrameType::RR);
}
TEST(ControlField, Information) {
    auto cf = ControlField::from_byte(0x00);
    EXPECT_EQ(cf.type, FrameType::Information);
    EXPECT_EQ(cf.send_seq, 0);
}
TEST(ControlField, InformationWithSeq) {
    auto cf = ControlField::from_byte(0x12); // send_seq=1, recv_seq=0
    EXPECT_EQ(cf.type, FrameType::Information);
    EXPECT_EQ(cf.send_seq, 1);
}
TEST(ControlField, Disconnect) {
    auto cf = ControlField::from_byte(0x53);
    EXPECT_EQ(cf.type, FrameType::Disconnect);
}
TEST(ControlField, RoundTripSNRM) {
    ControlField cf; cf.type = FrameType::SNRM; cf.final = true;
    auto b = cf.to_byte();
    auto cf2 = ControlField::from_byte(b);
    EXPECT_EQ(cf2.type, FrameType::SNRM);
}
TEST(ControlField, RoundTripInfo) {
    ControlField cf; cf.type = FrameType::Information; cf.send_seq = 3; cf.recv_seq = 5;
    auto b = cf.to_byte();
    auto cf2 = ControlField::from_byte(b);
    EXPECT_EQ(cf2.send_seq, 3); EXPECT_EQ(cf2.recv_seq, 5);
}

// ---- HdlcFrame Tests ----
TEST(HdlcFrame, MakeSNRM) {
    auto f = HdlcFrame::make_snrm(1, 1);
    EXPECT_EQ(f.type(), FrameType::SNRM);
    EXPECT_EQ(f.control.type, FrameType::SNRM);
}
TEST(HdlcFrame, MakeUA) {
    auto f = HdlcFrame::make_ua(1, 1);
    EXPECT_EQ(f.type(), FrameType::UA);
}
TEST(HdlcFrame, MakeRR) {
    auto f = HdlcFrame::make_rr(1, 1, 3);
    EXPECT_EQ(f.type(), FrameType::RR);
    EXPECT_EQ(f.control.recv_seq, 3);
}
TEST(HdlcFrame, MakeInformation) {
    std::vector<uint8_t> payload = {0x01, 0x02};
    auto f = HdlcFrame::make_information(1, 1, 0, 0, payload);
    EXPECT_EQ(f.type(), FrameType::Information);
    EXPECT_EQ(f.information, payload);
}
TEST(HdlcFrame, MakeDisconnect) {
    auto f = HdlcFrame::make_disconnect(1, 1);
    EXPECT_EQ(f.type(), FrameType::Disconnect);
}
TEST(HdlcFrame, ToBytesStartsWithFlag) {
    auto f = HdlcFrame::make_snrm(1, 1);
    auto bytes = f.to_bytes();
    ASSERT_FALSE(bytes.empty());
    EXPECT_EQ(bytes.front(), 0x7E);
    EXPECT_EQ(bytes.back(), 0x7E);
}
TEST(HdlcFrame, InformationToBytes) {
    std::vector<uint8_t> payload = {0xC0, 0x01, 0x00, 0x01, 0x08, 0x00, 0xFF};
    auto f = HdlcFrame::make_information(1, 1, 0, 0, payload);
    auto bytes = f.to_bytes();
    EXPECT_EQ(bytes.front(), 0x7E);
    EXPECT_EQ(bytes.back(), 0x7E);
}
TEST(HdlcFrame, EmptyInformationToBytes) {
    auto f = HdlcFrame::make_snrm(1, 1);
    auto bytes = f.to_bytes();
    EXPECT_GE(bytes.size(), 8u); // flag + format + addr + addr + ctrl + fcs + flag
}
TEST(HdlcFrame, SNTMToBytesEndsWithFlag) {
    auto f = HdlcFrame::make_snrm(1, 1);
    auto bytes = f.to_bytes();
    EXPECT_EQ(bytes.back(), 0x7E);
}
TEST(HdlcFrame, UAToBytes) {
    auto f = HdlcFrame::make_ua(1, 1);
    auto bytes = f.to_bytes();
    EXPECT_EQ(bytes.front(), 0x7E);
    EXPECT_EQ(bytes.back(), 0x7E);
}
TEST(HdlcFrame, FromBytesInvalidFlag) {
    uint8_t data[] = {0x00, 0x01};
    auto r = HdlcFrame::from_bytes(data, 2);
    EXPECT_FALSE(is_ok(r));
}
TEST(HdlcFrame, SeqNumbersWrap) {
    auto f = HdlcFrame::make_information(1, 1, 7, 7, {});
    EXPECT_EQ(f.control.send_seq, 7);
}
