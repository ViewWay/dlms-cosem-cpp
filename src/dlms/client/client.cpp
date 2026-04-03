#include <dlms/client/client.hpp>
#include <dlms/hdlc/crc.hpp>
#include <dlms/config.hpp>

namespace dlms::client {

DlmsClient::DlmsClient(const DlmsConnectionConfig& config) : config_(config) {}

DlmsClient::~DlmsClient() { disconnect(); }

VoidResult DlmsClient::connect() {
    if (config_.transport_type == "tcp") {
        transport_ = std::make_unique<transport::TcpTransport>();
    } else {
        transport_ = std::make_unique<transport::SerialTransport>(config_.address);
    }

    auto r = transport_->connect(config_.address, config_.port);
    if (!is_ok(r)) return r;

    // Send SNRM
    auto snrm = hdlc::HdlcFrame::make_snrm(config_.client_address, config_.server_address);
    auto snrm_bytes = snrm.to_bytes();
    auto resp = transport_->send_and_receive(snrm_bytes);
    if (!is_ok(resp)) return make_err<std::monostate>(get_err(resp));

    // Parse UA
    auto ua = hdlc::HdlcFrame::from_bytes(get_val(resp).data(), get_val(resp).size());
    if (!is_ok(ua)) return make_err<std::monostate>(get_err(ua));

    connected_ = true;
    send_seq_ = 0;
    recv_seq_ = 0;
    return make_void_ok();
}

VoidResult DlmsClient::disconnect() {
    if (connected_ && transport_) {
        auto disc = hdlc::HdlcFrame::make_disconnect(config_.client_address, config_.server_address);
        transport_->send_and_receive(disc.to_bytes());
    }
    if (transport_) transport_->disconnect();
    connected_ = false;
    return make_void_ok();
}

bool DlmsClient::is_connected() const { return connected_; }

Result<std::vector<uint8_t>> DlmsClient::send_hdlc(const std::vector<uint8_t>& payload) {
    if (!transport_) return make_err<std::vector<uint8_t>>(DlmsError::ConnectionError);
    auto frame = hdlc::HdlcFrame::make_information(
        config_.client_address, config_.server_address,
        send_seq_, recv_seq_, payload);
    auto bytes = frame.to_bytes();
    auto resp = transport_->send_and_receive(bytes);
    if (!is_ok(resp)) return make_err<std::vector<uint8_t>>(get_err(resp));
    send_seq_ = (send_seq_ + 1) & 0x07;
    return get_val(resp);
}

Result<std::vector<uint8_t>> DlmsClient::receive_hdlc() {
    auto resp = transport_->send_and_receive({});
    if (!is_ok(resp)) return make_err<std::vector<uint8_t>>(get_err(resp));
    auto frame = hdlc::HdlcFrame::from_bytes(get_val(resp).data(), get_val(resp).size());
    if (!is_ok(frame)) return make_err<std::vector<uint8_t>>(get_err(frame));
    return get_val(frame).information;
}

Result<DlmsValue> DlmsClient::get(const ObisCode& obis, uint16_t class_id, int attribute_id) {
    axdr::AxdEncoder enc;
    // Get request: tag 0xC0 (get request normal)
    enc.buffer().write_u8(0xC0); // Get-Request-Normal
    // Invoke ID and priority
    enc.buffer().write_u8(0x01); // invoke_id = 1, priority = high
    // CosemAttributeDescriptor
    enc.buffer().write_u16(class_id);
    auto ob = obis.to_array();
    enc.buffer().write_bytes(ob.data(), ob.size());
    enc.buffer().write_u8(static_cast<uint8_t>(attribute_id));

    auto resp = send_hdlc(enc.data());
    if (!is_ok(resp)) return make_err<DlmsValue>(get_err(resp));

    // Parse response
    axdr::AxdDecoder dec(get_val(resp));
    return dec.decode();
}

VoidResult DlmsClient::set(const ObisCode& obis, uint16_t class_id, int attribute_id, const DlmsValue& value) {
    axdr::AxdEncoder enc;
    enc.buffer().write_u8(0x01); // Set-Request-Normal
    enc.buffer().write_u8(0x01); // invoke_id = 1
    enc.buffer().write_u16(class_id);
    auto ob = obis.to_array();
    enc.buffer().write_bytes(ob.data(), ob.size());
    enc.buffer().write_u8(static_cast<uint8_t>(attribute_id));
    enc.encode_value(value);

    auto resp = send_hdlc(enc.data());
    if (!is_ok(resp)) return make_err<std::monostate>(get_err(resp));
    return make_void_ok();
}

Result<DlmsValue> DlmsClient::action(const ObisCode& obis, uint16_t class_id, int method_id, const DlmsValue& args) {
    axdr::AxdEncoder enc;
    enc.buffer().write_u8(0x06); // Action-Request-Normal
    enc.buffer().write_u8(0x01); // invoke_id = 1
    enc.buffer().write_u16(class_id);
    auto ob = obis.to_array();
    enc.buffer().write_bytes(ob.data(), ob.size());
    enc.buffer().write_u8(static_cast<uint8_t>(method_id));
    enc.encode_value(args);

    auto resp = send_hdlc(enc.data());
    if (!is_ok(resp)) return make_err<DlmsValue>(get_err(resp));

    axdr::AxdDecoder dec(get_val(resp));
    return dec.decode();
}

} // namespace dlms::client
