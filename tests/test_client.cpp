#include <gtest/gtest.h>
#include <dlms/client/client.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <dlms/cosem/data.hpp>
#include <cstdint>

using namespace dlms;
using namespace dlms::client;

// ---- DlmsConnectionConfig Tests ----
TEST(DlmsConnectionConfig, Defaults) {
    DlmsConnectionConfig cfg;
    EXPECT_EQ(cfg.transport_type, "tcp");
    EXPECT_EQ(cfg.port, 4059);
    EXPECT_EQ(cfg.client_address, 1);
    EXPECT_EQ(cfg.auth, AuthenticationMechanism::None);
}
TEST(DlmsConnectionConfig, Custom) {
    DlmsConnectionConfig cfg;
    cfg.address = "192.168.1.1";
    cfg.port = 10000;
    cfg.security_suite = 5;
    EXPECT_EQ(cfg.port, 10000);
    EXPECT_EQ(cfg.security_suite, 5);
}
TEST(DlmsConnectionConfig, Serial) {
    DlmsConnectionConfig cfg;
    cfg.transport_type = "serial";
    cfg.address = "/dev/ttyUSB0";
    EXPECT_EQ(cfg.transport_type, "serial");
}

// ---- DlmsClient Tests ----
TEST(DlmsClient, Constructor) {
    DlmsConnectionConfig cfg;
    DlmsClient client(cfg);
    EXPECT_FALSE(client.is_connected());
}
TEST(DlmsClient, DisconnectWhenNotConnected) {
    DlmsConnectionConfig cfg;
    DlmsClient client(cfg);
    auto r = client.disconnect();
    EXPECT_TRUE(is_ok(r));
}
TEST(DlmsClient, GetWhenNotConnected) {
    DlmsConnectionConfig cfg;
    DlmsClient client(cfg);
    auto r = client.get(ObisCode(0,0,1,8,0,255), 1, 2);
    EXPECT_FALSE(is_ok(r));
}
TEST(DlmsClient, SetWhenNotConnected) {
    DlmsConnectionConfig cfg;
    DlmsClient client(cfg);
    auto r = client.set(ObisCode(0,0,1,8,0,255), 1, 2, DlmsValue(int32_t(0)));
    EXPECT_FALSE(is_ok(r));
}
TEST(DlmsClient, ActionWhenNotConnected) {
    DlmsConnectionConfig cfg;
    DlmsClient client(cfg);
    auto r = client.action(ObisCode(0,0,0,0,0,255), 1, 1);
    EXPECT_FALSE(is_ok(r));
}
TEST(DlmsClient, ConnectInvalidAddress) {
    DlmsConnectionConfig cfg;
    cfg.address = "256.256.256.256";
    DlmsClient client(cfg);
    auto r = client.connect();
    EXPECT_FALSE(is_ok(r));
}

// ---- Transport Tests ----
TEST(TcpTransport, NotConnected) {
    transport::TcpTransport tcp;
    EXPECT_FALSE(tcp.is_connected());
}
TEST(SerialTransport, NotConnected) {
    transport::SerialTransport serial("/dev/ttyUSB0");
    EXPECT_FALSE(serial.is_connected());
    auto r = serial.send_and_receive({0x01});
    EXPECT_FALSE(is_ok(r));
}
TEST(SerialTransport, ConnectNotSupported) {
    transport::SerialTransport serial("/dev/ttyUSB0");
    auto r = serial.connect("", 0);
    EXPECT_FALSE(is_ok(r));
}

// ---- HdlcFrame Client Integration Tests ----
TEST(HdlcFrame, MakeClientFrames) {
    auto snrm = hdlc::HdlcFrame::make_snrm(1, 1);
    auto bytes = snrm.to_bytes();
    EXPECT_EQ(bytes.front(), 0x7E);
    EXPECT_EQ(bytes.back(), 0x7E);

    auto ua = hdlc::HdlcFrame::make_ua(1, 1);
    auto ua_bytes = ua.to_bytes();
    EXPECT_EQ(ua_bytes.front(), 0x7E);

    auto info = hdlc::HdlcFrame::make_information(1, 1, 0, 0, {0xC0, 0x01});
    auto info_bytes = info.to_bytes();
    EXPECT_EQ(info_bytes.front(), 0x7E);

    auto disc = hdlc::HdlcFrame::make_disconnect(1, 1);
    auto disc_bytes = disc.to_bytes();
    EXPECT_EQ(disc_bytes.front(), 0x7E);
}
TEST(HdlcFrame, MultipleInfoFrames) {
    for (int i = 0; i < 8; ++i) {
        auto f = hdlc::HdlcFrame::make_information(1, 1, i, i, {});
        EXPECT_EQ(f.control.send_seq, i);
        EXPECT_EQ(f.control.recv_seq, i);
    }
}

// ---- CosemObject Client Tests ----
TEST(CosemObject, GetAttributeNotFound) {
    dlms::cosem::Data d;
    auto r = d.get_attribute(99);
    EXPECT_FALSE(is_ok(r));
    EXPECT_EQ(get_err(r), DlmsError::ObjectUndefined);
}
TEST(CosemObject, SetAttributeDenied) {
    dlms::cosem::Data d;
    auto r = d.set_attribute(99, DlmsValue(int32_t(0)));
    EXPECT_FALSE(is_ok(r));
    EXPECT_EQ(get_err(r), DlmsError::AccessDenied);
}
