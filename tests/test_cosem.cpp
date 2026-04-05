#include <gtest/gtest.h>
#include <dlms/cosem/data.hpp>
#include <dlms/cosem/register.hpp>
#include <dlms/cosem/extended_register.hpp>
#include <dlms/cosem/demand_register.hpp>
#include <dlms/cosem/clock.hpp>
#include <dlms/cosem/security_setup.hpp>
#include <dlms/cosem/profile_generic.hpp>
#include <dlms/cosem/tariff_plan.hpp>
#include <dlms/cosem/tariff_table.hpp>
#include <dlms/cosem/lp_setup.hpp>
#include <dlms/cosem/rs485_setup.hpp>
#include <dlms/cosem/nbiot_setup.hpp>
#include <dlms/cosem/lorawan_setup.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <dlms/core/obis.hpp>
#include <cstdint>

using namespace dlms;
using namespace dlms::cosem;

// ---- Data IC Tests ----
TEST(Data, ClassId) {
    Data d; d.set_logical_name(ObisCode(0,0,1,0,0,255));
    EXPECT_EQ(d.class_id(), 1);
    EXPECT_EQ(d.class_name(), "Data");
}
TEST(Data, GetLogicalName) {
    Data d; d.set_logical_name(ObisCode(1,0,1,8,0,255));
    auto r = d.get_attribute(1);
    ASSERT_TRUE(is_ok(r));
    EXPECT_TRUE(is_octet_string(get_val(r)));
}
TEST(Data, GetValue) {
    Data d; d.set_logical_name(ObisCode(0,0,0,0,0,255));
    d.value_ = DlmsValue(int32_t(42));
    auto r = d.get_attribute(2);
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(to_int64(get_val(r)), 42);
}
TEST(Data, SetValue) {
    Data d; d.set_logical_name(ObisCode(0,0,0,0,0,255));
    auto r = d.set_attribute(2, DlmsValue(int32_t(99)));
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(to_int64(d.value_), 99);
}
TEST(Data, UnknownAttribute) {
    Data d;
    auto r = d.get_attribute(99);
    EXPECT_FALSE(is_ok(r));
}

// ---- Register IC Tests ----
TEST(Register, ClassId) {
    Register r; EXPECT_EQ(r.class_id(), 3);
    EXPECT_EQ(r.class_name(), "Register");
}
TEST(Register, GetValue) {
    Register reg; reg.value_ = DlmsValue(int32_t(12345));
    auto r = reg.get_attribute(2);
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(to_int64(get_val(r)), 12345);
}
TEST(Register, GetScaler) {
    Register reg; reg.scaler_ = -2;
    auto r = reg.get_attribute(3);
    ASSERT_TRUE(is_ok(r));
}
TEST(Register, GetUnit) {
    Register reg; reg.unit_ = 30; // Wh
    auto r = reg.get_attribute(4);
    ASSERT_TRUE(is_ok(r));
}
TEST(Register, SetValue) {
    Register reg;
    auto r = reg.set_attribute(2, DlmsValue(int32_t(555)));
    ASSERT_TRUE(is_ok(r));
}
TEST(Register, SetScaler) {
    Register reg;
    auto r = reg.set_attribute(3, DlmsValue(int8_t(-1)));
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(reg.scaler_, -1);
}

// ---- ExtendedRegister Tests ----
TEST(ExtendedRegister, ClassId) {
    ExtendedRegister r; EXPECT_EQ(r.class_id(), 4);
}
TEST(ExtendedRegister, Status) {
    ExtendedRegister reg; reg.status_ = 5;
    auto r = reg.get_attribute(5);
    ASSERT_TRUE(is_ok(r));
}

// ---- DemandRegister Tests ----
TEST(DemandRegister, ClassId) {
    DemandRegister r; EXPECT_EQ(r.class_id(), 5);
}
TEST(DemandRegister, Period) {
    DemandRegister reg; reg.period_ = 900;
    auto r = reg.get_attribute(6);
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(to_int64(get_val(r)), 900);
}

// ---- Clock Tests ----
TEST(Clock, ClassId) {
    Clock c; EXPECT_EQ(c.class_id(), 8);
    EXPECT_EQ(c.class_name(), "Clock");
}
TEST(Clock, GetTime) {
    Clock c;
    auto r = c.get_attribute(2);
    ASSERT_TRUE(is_ok(r));
    EXPECT_TRUE(is_octet_string(get_val(r)));
}
TEST(Clock, GetTimezone) {
    Clock c; c.timezone_ = 480;
    auto r = c.get_attribute(3);
    ASSERT_TRUE(is_ok(r));
}
TEST(Clock, SetTimezone) {
    Clock c;
    auto r = c.set_attribute(3, DlmsValue(int16_t(480)));
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(c.timezone_, 480);
}
TEST(Clock, GetStatus) {
    Clock c; c.status_ = 0x01;
    auto r = c.get_attribute(4);
    ASSERT_TRUE(is_ok(r));
}

// ---- SecuritySetup Tests ----
TEST(SecuritySetup, ClassId) {
    SecuritySetup s; EXPECT_EQ(s.class_id(), 64);
}
TEST(SecuritySetup, GetPolicy) {
    SecuritySetup s; s.policy_ = SecurityPolicy::AuthenticatedEncrypted;
    auto r = s.get_attribute(2);
    ASSERT_TRUE(is_ok(r));
}
TEST(SecuritySetup, GetSuite) {
    SecuritySetup s; s.suite_ = 5;
    auto r = s.get_attribute(3);
    ASSERT_TRUE(is_ok(r));
}
TEST(SecuritySetup, SetSuite) {
    SecuritySetup s;
    auto r = s.set_attribute(3, DlmsValue(uint8_t(2)));
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(s.suite_, 2);
}
TEST(SecuritySetup, SystemTitle) {
    SecuritySetup s; s.system_title_ = {1,2,3,4,5,6,7,8};
    auto r = s.get_attribute(8);
    ASSERT_TRUE(is_ok(r));
}

// ---- ProfileGeneric Tests ----
TEST(ProfileGeneric, ClassId) {
    ProfileGeneric pg; EXPECT_EQ(pg.class_id(), 7);
}
TEST(ProfileGeneric, EntriesInUse) {
    ProfileGeneric pg; pg.entries_in_use_ = 100;
    auto r = pg.get_attribute(7);
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(to_int64(get_val(r)), 100);
}
TEST(ProfileGeneric, CapturePeriod) {
    ProfileGeneric pg; pg.capture_period_ = 60;
    auto r = pg.get_attribute(4);
    ASSERT_TRUE(is_ok(r));
}
TEST(ProfileGeneric, SetProfileEntries) {
    ProfileGeneric pg;
    auto r = pg.set_attribute(8, DlmsValue(uint32_t(500)));
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(pg.profile_entries_, 500);
}

// ---- Setup Classes Tests ----
TEST(LpSetup, ClassId) {
    LpSetup lp; EXPECT_EQ(lp.class_name(), "LPSetup");
}
TEST(LpSetup, BaudRate) {
    LpSetup lp; lp.baud_rate_ = 115200;
    auto r = lp.get_attribute(2);
    ASSERT_TRUE(is_ok(r));
}
TEST(Rs485Setup, ClassId) {
    Rs485Setup rs; EXPECT_EQ(rs.class_name(), "RS485Setup");
}
TEST(NbiotSetup, ClassId) {
    NbiotSetup nb; EXPECT_EQ(nb.class_name(), "NBIoTSetup");
}
TEST(LorawanSetup, ClassId) {
    LorawanSetup lr; EXPECT_EQ(lr.class_name(), "LoRaWANSetup");
}
TEST(TariffPlan, ClassId) {
    TariffPlan tp; EXPECT_EQ(tp.class_name(), "TariffPlan");
}
TEST(TariffTable, ClassId) {
    TariffTable tt; EXPECT_EQ(tt.class_name(), "TariffTable");
}

// ---- IsStaticAttribute Tests ----
TEST(CosemObject, StaticAttribute1) {
    Data d;
    EXPECT_TRUE(d.is_static_attribute(1));
    EXPECT_FALSE(d.is_static_attribute(2));
}
TEST(CosemObject, StaticAttributeRegister) {
    Register r;
    EXPECT_TRUE(r.is_static_attribute(1));
    EXPECT_TRUE(r.is_static_attribute(3));
    EXPECT_FALSE(r.is_static_attribute(2));
}
TEST(CosemObject, StaticAttributeClock) {
    Clock c;
    EXPECT_TRUE(c.is_static_attribute(1));
    EXPECT_TRUE(c.is_static_attribute(3));
    EXPECT_FALSE(c.is_static_attribute(2));
}
TEST(CosemObject, StaticAttributeSecurity) {
    SecuritySetup s;
    EXPECT_TRUE(s.is_static_attribute(1));
    EXPECT_TRUE(s.is_static_attribute(2));
}
TEST(CosemObject, StaticAttributeUnknown) {
    Data d;
    EXPECT_FALSE(d.is_static_attribute(99));
}
TEST(CosemObject, GetAttributes) {
    Register r;
    auto attrs = r.get_attributes();
    EXPECT_GE(attrs.size(), 2u);
}
TEST(CosemObject, GetAttributesData) {
    Data d;
    auto attrs = d.get_attributes();
    EXPECT_GE(attrs.size(), 1u);
}
TEST(CosemObject, GetAttributesClock) {
    Clock c;
    auto attrs = c.get_attributes();
    EXPECT_GE(attrs.size(), 3u);
}
TEST(CosemObject, GetAttributesProfile) {
    ProfileGeneric pg;
    auto attrs = pg.get_attributes();
    EXPECT_GE(attrs.size(), 4u);
}
TEST(CosemObject, GetAttributesDemand) {
    DemandRegister dr;
    auto attrs = dr.get_attributes();
    EXPECT_GE(attrs.size(), 4u);
}
TEST(CosemObject, GetAttributesExtended) {
    ExtendedRegister er;
    auto attrs = er.get_attributes();
    EXPECT_GE(attrs.size(), 3u);
}
TEST(CosemObject, GetAttributesSecuritySetup) {
    SecuritySetup ss;
    auto attrs = ss.get_attributes();
    EXPECT_GE(attrs.size(), 4u);
}
TEST(CosemObject, GetAttributesLpSetup) {
    LpSetup lp;
    auto attrs = lp.get_attributes();
    EXPECT_GE(attrs.size(), 2u);
}
TEST(CosemObject, GetAttributesRS485) {
    Rs485Setup rs;
    auto attrs = rs.get_attributes();
    EXPECT_GE(attrs.size(), 2u);
}
TEST(CosemObject, GetAttributesNBIoT) {
    NbiotSetup nb;
    auto attrs = nb.get_attributes();
    EXPECT_GE(attrs.size(), 2u);
}
TEST(CosemObject, GetAttributesLoRaWAN) {
    LorawanSetup lr;
    auto attrs = lr.get_attributes();
    EXPECT_GE(attrs.size(), 2u);
}

// ---- Version Tests ----
TEST(Version, AllZero) {
    Data d; EXPECT_EQ(d.version(), 0);
    Register r; EXPECT_EQ(r.version(), 0);
    Clock c; EXPECT_EQ(c.version(), 0);
    ProfileGeneric pg; EXPECT_EQ(pg.version(), 0);
}

// ---- New IC Classes Tests ----
#include <dlms/cosem/association_ln.hpp>
#include <dlms/cosem/disconnect_control.hpp>
#include <dlms/cosem/push_setup.hpp>
#include <dlms/cosem/image_transfer.hpp>
#include <dlms/cosem/activity_calendar.hpp>
#include <dlms/cosem/special_days_table.hpp>
#include <dlms/cosem/register_monitor.hpp>
#include <dlms/cosem/limiter.hpp>
#include <dlms/cosem/mbus_client.hpp>
#include <dlms/cosem/gprs_modem_setup.hpp>

// ---- AssociationLN Tests (IC15) ----
TEST(AssociationLN, ClassId) {
    AssociationLN a; EXPECT_EQ(a.class_id(), 15);
    EXPECT_EQ(a.class_name(), "AssociationLN");
}
TEST(AssociationLN, DefaultPartners) {
    AssociationLN a;
    auto [client, server] = a.get_partners();
    EXPECT_EQ(client, 16);
    EXPECT_EQ(server, 1);
}
TEST(AssociationLN, SetPartners) {
    AssociationLN a;
    a.set_partners(32, 2);
    auto [client, server] = a.get_partners();
    EXPECT_EQ(client, 32);
    EXPECT_EQ(server, 2);
}
TEST(AssociationLN, AddObject) {
    AssociationLN a;
    a.add_object(1, ObisCode(0, 0, 1, 0, 0, 255));
    a.add_object(3, ObisCode(1, 0, 1, 8, 0, 255));
    EXPECT_EQ(a.object_list_.size(), 2u);
}
TEST(AssociationLN, Authentication) {
    AssociationLN a;
    EXPECT_FALSE(a.is_authenticated());
    a.set_authenticated(true);
    EXPECT_TRUE(a.is_authenticated());
}
TEST(AssociationLN, GetAttributes) {
    AssociationLN a;
    auto attrs = a.get_attributes();
    EXPECT_GE(attrs.size(), 5u);
}

// ---- DisconnectControl Tests (IC70) ----
TEST(DisconnectControl, ClassId) {
    DisconnectControl d; EXPECT_EQ(d.class_id(), 70);
    EXPECT_EQ(d.class_name(), "DisconnectControl");
}
TEST(DisconnectControl, DefaultState) {
    DisconnectControl d;
    EXPECT_EQ(d.control_state(), DisconnectState::Connected);
}
TEST(DisconnectControl, SetState) {
    DisconnectControl d;
    d.set_control_state(DisconnectState::Disconnected);
    EXPECT_EQ(d.control_state(), DisconnectState::Disconnected);
}
TEST(DisconnectControl, MethodDisconnect) {
    DisconnectControl d;
    auto r = d.execute_method(1);
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(d.control_state(), DisconnectState::Disconnected);
}
TEST(DisconnectControl, MethodReconnect) {
    DisconnectControl d;
    d.execute_method(1); // disconnect first
    auto r = d.execute_method(2);
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(d.control_state(), DisconnectState::Connected);
}

// ---- PushSetup Tests (IC40) ----
TEST(PushSetup, ClassId) {
    PushSetup p; EXPECT_EQ(p.class_id(), 40);
    EXPECT_EQ(p.class_name(), "PushSetup");
}
TEST(PushSetup, AddObject) {
    PushSetup p;
    p.add_push_object(8, ObisCode(0, 0, 1, 0, 0, 255), 2);
    p.add_push_object(3, ObisCode(1, 0, 1, 8, 0, 255), 2);
    EXPECT_EQ(p.object_count(), 2u);
}
TEST(PushSetup, DefaultRetries) {
    PushSetup p;
    EXPECT_EQ(p.number_of_retries_, 3);
}
TEST(PushSetup, SetService) {
    PushSetup p;
    p.service_ = 1234;
    auto r = p.get_attribute(3);
    ASSERT_TRUE(is_ok(r));
}

// ---- ImageTransfer Tests (IC18) ----
TEST(ImageTransfer, ClassId) {
    ImageTransfer i; EXPECT_EQ(i.class_id(), 18);
    EXPECT_EQ(i.class_name(), "ImageTransfer");
}
TEST(ImageTransfer, DefaultStatus) {
    ImageTransfer i;
    EXPECT_EQ(i.status(), ImageTransferStatus::NotInitiated);
}
TEST(ImageTransfer, Initiate) {
    ImageTransfer i;
    auto r = i.initiate({1, 2, 3, 4}, 1024);
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(i.status(), ImageTransferStatus::Initiated);
}
TEST(ImageTransfer, Verify) {
    ImageTransfer i;
    i.initiate({}, 10);
    i.image_data_.resize(10); // Simulate complete transfer
    auto r = i.verify();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(i.status(), ImageTransferStatus::VerificationSuccessful);
}
TEST(ImageTransfer, Activate) {
    ImageTransfer i;
    i.initiate({}, 10);
    i.image_data_.resize(10);
    i.verify();
    auto r = i.activate();
    ASSERT_TRUE(is_ok(r));
    EXPECT_EQ(i.status(), ImageTransferStatus::ActivationSuccessful);
}

// ---- ActivityCalendar Tests (IC20) ----
TEST(ActivityCalendar, ClassId) {
    ActivityCalendar a; EXPECT_EQ(a.class_id(), 20);
    EXPECT_EQ(a.class_name(), "ActivityCalendar");
}
TEST(ActivityCalendar, CalendarName) {
    ActivityCalendar a;
    a.calendar_name_active_ = "Summer2024";
    auto r = a.get_attribute(2);
    ASSERT_TRUE(is_ok(r));
}
TEST(ActivityCalendar, GetAttributes) {
    ActivityCalendar a;
    auto attrs = a.get_attributes();
    EXPECT_GE(attrs.size(), 5u);
}

// ---- SpecialDaysTable Tests (IC11) ----
TEST(SpecialDaysTable, ClassId) {
    SpecialDaysTable s; EXPECT_EQ(s.class_id(), 11);
    EXPECT_EQ(s.class_name(), "SpecialDaysTable");
}
TEST(SpecialDaysTable, AddEntry) {
    SpecialDaysTable s;
    s.add_entry(1, {0, 0, 1, 1, 0, 0}, 10);
    s.add_entry(2, {0, 0, 25, 12, 0, 0}, 20);
    EXPECT_EQ(s.entry_count(), 2u);
}
TEST(SpecialDaysTable, ClearEntries) {
    SpecialDaysTable s;
    s.add_entry(1, {}, 1);
    s.clear_entries();
    EXPECT_EQ(s.entry_count(), 0u);
}

// ---- RegisterMonitor Tests (IC21) ----
TEST(RegisterMonitor, ClassId) {
    RegisterMonitor r; EXPECT_EQ(r.class_id(), 21);
    EXPECT_EQ(r.class_name(), "RegisterMonitor");
}
TEST(RegisterMonitor, AddThreshold) {
    RegisterMonitor r;
    r.add_threshold(DlmsValue(int32_t(100)));
    r.add_threshold(DlmsValue(int32_t(200)));
    EXPECT_EQ(r.threshold_count(), 2u);
}
TEST(RegisterMonitor, SetMonitoredValue) {
    RegisterMonitor r;
    r.set_monitored_value(3, ObisCode(1, 0, 1, 8, 0, 255), 2);
    EXPECT_EQ(r.monitored_value_.class_id, 3);
}
TEST(RegisterMonitor, Capture) {
    RegisterMonitor r;
    r.capture(DlmsValue(int32_t(12345)));
    auto v = r.get_attribute(4);
    ASSERT_TRUE(is_ok(v));
}

// ---- Limiter Tests (IC30) ----
TEST(Limiter, ClassId) {
    Limiter l; EXPECT_EQ(l.class_id(), 30);
    EXPECT_EQ(l.class_name(), "Limiter");
}
TEST(Limiter, SetThresholds) {
    Limiter l;
    l.threshold_active_ = DlmsValue(int32_t(1000));
    l.threshold_normal_ = DlmsValue(int32_t(800));
    auto r = l.get_attribute(3);
    ASSERT_TRUE(is_ok(r));
}
TEST(Limiter, DurationSettings) {
    Limiter l;
    l.min_over_threshold_duration_ = 60;
    l.min_under_threshold_duration_ = 30;
    EXPECT_EQ(l.min_over_threshold_duration_, 60);
}
TEST(Limiter, CheckThreshold) {
    Limiter l;
    l.threshold_active_ = DlmsValue(int32_t(100));
    EXPECT_TRUE(l.is_over_threshold(DlmsValue(int32_t(150))));
    EXPECT_FALSE(l.is_over_threshold(DlmsValue(int32_t(50))));
}

// ---- MbusClient Tests (IC57) ----
TEST(MbusClient, ClassId) {
    MbusClient m; EXPECT_EQ(m.class_id(), 57);
    EXPECT_EQ(m.class_name(), "MbusClient");
}
TEST(MbusClient, AddSlave) {
    MbusClient m;
    m.add_mbus_slave(1, {0x01, 0x02, 0x03, 0x04}, 1, 3);
    m.add_mbus_slave(2, {0x05, 0x06, 0x07, 0x08}, 2, 4);
    EXPECT_EQ(m.slave_count(), 2u);
}
TEST(MbusClient, ClearSlaves) {
    MbusClient m;
    m.add_mbus_slave(1, {}, 1, 1);
    m.clear_slaves();
    EXPECT_EQ(m.slave_count(), 0u);
}

// ---- GprsModemSetup Tests (IC45) ----
TEST(GprsModemSetup, ClassId) {
    GprsModemSetup g; EXPECT_EQ(g.class_id(), 45);
    EXPECT_EQ(g.class_name(), "GprsModemSetup");
}
TEST(GprsModemSetup, SetApn) {
    GprsModemSetup g;
    g.set_apn("internet.provider.com");
    EXPECT_EQ(g.apn(), "internet.provider.com");
}
TEST(GprsModemSetup, SetPin) {
    GprsModemSetup g;
    g.set_pin("1234");
    EXPECT_EQ(g.pin_code(), "1234");
}
TEST(GprsModemSetup, AddPdpContext) {
    GprsModemSetup g;
    g.add_pdp_context(1, "IP", "0.0.0.0", "apn.example.com");
    EXPECT_EQ(g.pdp_contexts_.size(), 1u);
}
