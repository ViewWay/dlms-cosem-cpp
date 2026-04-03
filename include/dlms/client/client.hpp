#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <dlms/hdlc/frame.hpp>
#include <dlms/axdr/encoder.hpp>
#include <dlms/axdr/decoder.hpp>
#include <dlms/asn1/ber.hpp>
#include <dlms/security/security.hpp>
#include <dlms/transport/transport.hpp>
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace dlms::client {

struct DlmsConnectionConfig {
    std::string transport_type = "tcp"; // "tcp" or "serial"
    std::string address = "127.0.0.1";
    uint16_t port = 4059;
    uint16_t client_address = 1;
    uint16_t server_address = 1;
    AuthenticationMechanism auth = AuthenticationMechanism::None;
    std::vector<uint8_t> password;
    int security_suite = 0;
    std::vector<uint8_t> encryption_key;
    std::vector<uint8_t> authentication_key;
    std::vector<uint8_t> system_title;
    uint32_t invocation_counter = 0;
};

class DlmsClient {
public:
    explicit DlmsClient(const DlmsConnectionConfig& config);
    ~DlmsClient();

    VoidResult connect();
    VoidResult disconnect();
    bool is_connected() const;

    Result<DlmsValue> get(const ObisCode& obis, uint16_t class_id, int attribute_id);
    VoidResult set(const ObisCode& obis, uint16_t class_id, int attribute_id, const DlmsValue& value);
    Result<DlmsValue> action(const ObisCode& obis, uint16_t class_id, int method_id, const DlmsValue& args = DlmsValue(DlmsNone{}));

private:
    Result<std::vector<uint8_t>> send_hdlc(const std::vector<uint8_t>& payload);
    Result<std::vector<uint8_t>> receive_hdlc();

    DlmsConnectionConfig config_;
    std::unique_ptr<transport::ITransport> transport_;
    bool connected_ = false;
    int send_seq_ = 0;
    int recv_seq_ = 0;
};

} // namespace dlms::client
