#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/byte_buffer.hpp>
#include <dlms/hdlc/address.hpp>
#include <vector>
#include <cstdint>

namespace dlms::hdlc {

enum class FrameType { SNRM, UA, RR, Information, Disconnect, UI, Unknown };

struct FrameFormat {
    uint16_t length = 0;
    bool segmented = false;

    static Result<FrameFormat> from_bytes(ByteBuffer& buf);
    void to_bytes(ByteBuffer& buf) const;
};

struct ControlField {
    uint8_t raw = 0;
    FrameType type = FrameType::Unknown;
    int send_seq = 0, recv_seq = 0;
    bool final = true;

    static ControlField from_byte(uint8_t b);
    uint8_t to_byte() const;
};

struct HdlcFrame {
    FrameFormat format;
    HdlcAddress dest;
    HdlcAddress source;
    ControlField control;
    std::vector<uint8_t> information;

    static Result<HdlcFrame> from_bytes(const uint8_t* data, size_t len);
    std::vector<uint8_t> to_bytes() const;

    // Frame constructors
    static HdlcFrame make_snrm(uint16_t client_addr, uint16_t server_addr);
    static HdlcFrame make_ua(uint16_t client_addr, uint16_t server_addr);
    static HdlcFrame make_rr(uint16_t client_addr, uint16_t server_addr, int recv_seq);
    static HdlcFrame make_information(uint16_t client_addr, uint16_t server_addr,
                                       int send_seq, int recv_seq, const std::vector<uint8_t>& payload);
    static HdlcFrame make_disconnect(uint16_t client_addr, uint16_t server_addr);

    FrameType type() const { return control.type; }
};

// HDLC byte stuffing
std::vector<uint8_t> hdlc_escape(const uint8_t* data, size_t len);
std::vector<uint8_t> hdlc_unescape(const uint8_t* data, size_t len);

} // namespace dlms::hdlc
