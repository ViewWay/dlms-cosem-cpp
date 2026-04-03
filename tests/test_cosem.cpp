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
