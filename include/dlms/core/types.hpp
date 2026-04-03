#pragma once
#include <cstdint>
#include <string>
#include <array>
#include <variant>
#include <optional>
#include <vector>
#include <chrono>
#include <functional>
#include <system_error>

namespace dlms {

enum class DlmsError {
    Success = 0, InvalidData, BufferOverflow, BufferUnderflow,
    InvalidTag, InvalidLength, InvalidObis, InvalidAddress,
    CrcError, EncryptionError, DecryptionError, AuthError,
    NotSupported, Timeout, ConnectionError, ObjectUndefined,
    TypeMismatch, AccessDenied, HardwareFault, TemporaryFailure,
};

inline std::string dlms_error_message(DlmsError e) {
    switch (e) {
        case DlmsError::Success: return "success";
        case DlmsError::InvalidData: return "invalid data";
        case DlmsError::BufferOverflow: return "buffer overflow";
        case DlmsError::BufferUnderflow: return "buffer underflow";
        case DlmsError::InvalidTag: return "invalid tag";
        case DlmsError::InvalidLength: return "invalid length";
        case DlmsError::InvalidObis: return "invalid OBIS code";
        case DlmsError::InvalidAddress: return "invalid address";
        case DlmsError::CrcError: return "CRC error";
        case DlmsError::EncryptionError: return "encryption error";
        case DlmsError::DecryptionError: return "decryption error";
        case DlmsError::AuthError: return "authentication error";
        case DlmsError::NotSupported: return "not supported";
        case DlmsError::Timeout: return "timeout";
        case DlmsError::ConnectionError: return "connection error";
        case DlmsError::ObjectUndefined: return "object undefined";
        case DlmsError::TypeMismatch: return "type mismatch";
        case DlmsError::AccessDenied: return "access denied";
        case DlmsError::HardwareFault: return "hardware fault";
        case DlmsError::TemporaryFailure: return "temporary failure";
        default: return "unknown error";
    }
}

struct DlmsErrorCategory : std::error_category {
    const char* name() const noexcept override { return "dlms"; }
    std::string message(int ev) const override { return dlms_error_message(static_cast<DlmsError>(ev)); }
};
inline const DlmsErrorCategory& dlms_error_category() { static DlmsErrorCategory c; return c; }
inline std::error_code make_error_code(DlmsError e) { return {static_cast<int>(e), dlms_error_category()}; }

template<typename T> using Result = std::variant<T, DlmsError>;
template<typename T> bool is_ok(const Result<T>& r) { return std::holds_alternative<T>(r); }
template<typename T> T& get_val(Result<T>& r) { return std::get<T>(r); }
template<typename T> const T& get_val(const Result<T>& r) { return std::get<T>(r); }
template<typename T> DlmsError get_err(const Result<T>& r) { return std::get<DlmsError>(r); }
template<typename T> Result<T> make_ok(T val) { return val; }
template<typename T> Result<T> make_err(DlmsError e) { return e; }

// VoidResult specialization - use std::monostate as void
using VoidResult = std::variant<std::monostate, DlmsError>;
inline bool is_ok(const VoidResult& r) { return std::holds_alternative<std::monostate>(r); }
inline DlmsError get_err(const VoidResult& r) { return std::get<DlmsError>(r); }
template<>
inline VoidResult make_err<std::monostate>(DlmsError e) { return e; }
inline VoidResult make_ok_void() { return std::monostate{}; }
inline VoidResult make_void_ok(DlmsError e = DlmsError::Success) { (void)e; return std::monostate{}; }

enum class DlmsTag : uint8_t {
    Null=0, Array=1, Structure=2, Boolean=3, BitString=4,
    DoubleLong=5, DoubleLongUnsigned=6, OctetString=9,
    VisibleString=10, UTF8String=12, BCD=13,
    Integer=15, Long=16, UnsignedInteger=17, UnsignedLong=18,
    CompactArray=19, Long64=20, UnsignedLong64=21, Enum=22,
    Float32=23, Float64=24, DateTime=25, Date=26, Time=27, DontCare=255,
};

enum class CosemInterface : uint16_t {
    Data=1, Register=3, ExtendedRegister=4, DemandRegister=5,
    RegisterActivation=6, ProfileGeneric=7, Clock=8, ScriptTable=9,
    Schedule=10, SpecialDaysTable=11, AssociationSN=12, AssociationLN=15,
    SapAssignment=17, ImageTransfer=18, IecLocalPortSetup=19,
    ActivityCalendar=20, RegisterMonitor=21, SecuritySetup=64,
    UtilityTables=26, GprsModemSetup=45, TcpUdpSetup=41,
    IecHdlcSetup=23, LoRaWANSetup=128,
};

enum class AuthenticationMechanism : uint8_t {
    None=0, LLS=1, HLS=2, HLS_MD5=3, HLS_SHA1=4,
    HLS_GMAC=5, HLS_SHA256=6, HLS_ECDSA=7,
};

enum class DataAccessResult : uint8_t {
    Success=0, HardwareFault=1, TemporaryFailure=2, ReadWriteDenied=3,
    ObjectUndefined=4, ObjectClassInconsistent=9, ObjectUnavailable=11,
    TypeUnmatched=12, ScopeOfAccessViolated=13, DataBlockUnavailable=14,
    LongGetAborted=15, NoLongGetInProgress=16, LongSetAborted=17,
    NoLongSetInProgress=18, DataBlockNumberInvalid=19, OtherReason=250,
};

enum class GetRequestType : uint8_t { Normal=1, Next=2, WithList=3 };
enum class GetResponseType : uint8_t { Normal=1, WithBlock=2, WithList=3 };
enum class SetRequestType : uint8_t { Normal=1, WithFirstBlock=2, WithBlock=3, WithList=4, FirstBlockWithList=5 };
enum class SetResponseType : uint8_t { Normal=1, WithBlock=2, WithLastBlock=3, LastBlockWithList=4, WithList=5 };

enum class SecuritySuiteNumber : uint8_t {
    Suite0=0, Suite1=1, Suite2=2, Suite3=3, Suite4=4, Suite5=5,
};

enum class SecurityPolicy : uint8_t {
    None=0, AuthenticatedOnly=1, EncryptedOnly=2,
    AuthenticatedEncrypted=3, CertificateBased=4, DigitallySigned=5,
};

enum class CipherAlgorithm : uint8_t {
    AES_128_GCM=0, AES_256_GCM=1, SM4_GCM=2, SM4_GMAC=3,
};

} // namespace dlms
