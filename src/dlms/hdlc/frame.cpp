#include <dlms/hdlc/frame.hpp>
#include <dlms/hdlc/crc.hpp>
#include <dlms/config.hpp>

namespace dlms::hdlc {

Result<FrameFormat> FrameFormat::from_bytes(ByteBuffer& buf) {
    auto b0 = buf.read_u8(); if(!is_ok(b0)) return make_err<FrameFormat>(get_err(b0));
    auto b1 = buf.read_u8(); if(!is_ok(b1)) return make_err<FrameFormat>(get_err(b1));
    uint16_t val = (uint16_t(get_val(b0)) << 8) | get_val(b1);
    if ((val & 0xF000) != 0xA000) return make_err<FrameFormat>(DlmsError::InvalidData);
    FrameFormat f;
    f.length = val & 0x07FF;
    f.segmented = (val & 0x0800) != 0;
    return f;
}

void FrameFormat::to_bytes(ByteBuffer& buf) const {
    uint16_t val = 0xA000 | (length & 0x07FF);
    if (segmented) val |= 0x0800;
    buf.write_u16(val);
}

ControlField ControlField::from_byte(uint8_t b) {
    ControlField cf; cf.raw = b;
    bool lsb = b & 0x01;
    if (!lsb) { // Information frame
        cf.type = FrameType::Information;
        cf.send_seq = (b >> 1) & 0x07;
        cf.final = (b >> 4) & 0x01;
        cf.recv_seq = (b >> 5) & 0x07;
    } else {
        cf.final = (b >> 4) & 0x01;
        // DLMS uses: SNRM=0x93, UA=0x73, DISC=0x53, RR=0x01/0x11/0x21, UI=0x03
        if ((b & 0xEF) == 0x83) cf.type = FrameType::SNRM; // 0x93 or 0x83
        else if ((b & 0xEF) == 0x63) cf.type = FrameType::UA;  // 0x73 or 0x63
        else if ((b & 0xEF) == 0x43) cf.type = FrameType::Disconnect; // 0x53 or 0x43
        else if ((b & 0xEF) == 0x03) cf.type = FrameType::UI;  // 0x03 or 0x13
        else if ((b & 0x03) == 0x01) { cf.recv_seq = (b >> 5) & 0x07; cf.type = FrameType::RR; }
        else cf.type = FrameType::Unknown;
    }
    return cf;
}

uint8_t ControlField::to_byte() const {
    switch (type) {
    case FrameType::Information:
        return (send_seq & 0x07) << 1 | ((final?1:0) << 4) | (recv_seq & 0x07) << 5;
    case FrameType::SNRM: return 0x83 | (final?0x10:0); // bit 7 set for DLMS
    case FrameType::UA: return 0x63 | (final?0x10:0);
    case FrameType::RR: return 0x01 | (recv_seq << 5) | (final?0x10:0);
    case FrameType::Disconnect: return 0x43 | (final?0x10:0);
    case FrameType::UI: return 0x03 | (final?0x10:0);
    default: return 0;
    }
}

std::vector<uint8_t> hdlc_escape(const uint8_t* data, size_t len) {
    std::vector<uint8_t> out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        if (data[i] == HDLC_FLAG_BYTE || data[i] == HDLC_ESCAPE_BYTE || data[i] == 0x5E || data[i] == 0x5D) {
            out.push_back(HDLC_ESCAPE_BYTE);
            out.push_back(data[i] ^ 0x20);
        } else {
            out.push_back(data[i]);
        }
    }
    return out;
}

std::vector<uint8_t> hdlc_unescape(const uint8_t* data, size_t len) {
    std::vector<uint8_t> out;
    out.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        if (data[i] == HDLC_ESCAPE_BYTE && i + 1 < len) {
            out.push_back(data[++i] ^ 0x20);
        } else {
            out.push_back(data[i]);
        }
    }
    return out;
}

HdlcFrame HdlcFrame::make_snrm(uint16_t client_addr, uint16_t server_addr) {
    HdlcFrame f;
    f.control.type = FrameType::SNRM; f.control.final = true;
    f.dest.type = HdlcAddress::Type::Server; f.dest.logical = server_addr;
    f.source.type = HdlcAddress::Type::Client; f.source.logical = client_addr;
    return f;
}

HdlcFrame HdlcFrame::make_ua(uint16_t client_addr, uint16_t server_addr) {
    HdlcFrame f;
    f.control.type = FrameType::UA; f.control.final = true;
    f.dest.type = HdlcAddress::Type::Client; f.dest.logical = client_addr;
    f.source.type = HdlcAddress::Type::Server; f.source.logical = server_addr;
    return f;
}

HdlcFrame HdlcFrame::make_rr(uint16_t client_addr, uint16_t server_addr, int recv_seq) {
    HdlcFrame f;
    f.control.type = FrameType::RR; f.control.final = true; f.control.recv_seq = recv_seq;
    f.dest.type = HdlcAddress::Type::Client; f.dest.logical = client_addr;
    f.source.type = HdlcAddress::Type::Server; f.source.logical = server_addr;
    return f;
}

HdlcFrame HdlcFrame::make_information(uint16_t client_addr, uint16_t server_addr,
                                       int send_seq, int recv_seq, const std::vector<uint8_t>& payload) {
    HdlcFrame f;
    f.control.type = FrameType::Information; f.control.final = true;
    f.control.send_seq = send_seq; f.control.recv_seq = recv_seq;
    f.dest.type = HdlcAddress::Type::Server; f.dest.logical = server_addr;
    f.source.type = HdlcAddress::Type::Client; f.source.logical = client_addr;
    f.information = payload;
    return f;
}

HdlcFrame HdlcFrame::make_disconnect(uint16_t client_addr, uint16_t server_addr) {
    HdlcFrame f;
    f.control.type = FrameType::Disconnect; f.control.final = true;
    f.dest.type = HdlcAddress::Type::Server; f.dest.logical = server_addr;
    f.source.type = HdlcAddress::Type::Client; f.source.logical = client_addr;
    return f;
}

Result<HdlcFrame> HdlcFrame::from_bytes(const uint8_t* data, size_t len) {
    if (len < 6 || data[0] != HDLC_FLAG_BYTE || data[len-1] != HDLC_FLAG_BYTE)
        return make_err<HdlcFrame>(DlmsError::InvalidData);

    // Unescape content between flags
    auto content = hdlc_unescape(data + 1, len - 2);
    ByteBuffer buf(content.data(), content.size());

    auto fmt = FrameFormat::from_bytes(buf);
    if (!is_ok(fmt)) return make_err<HdlcFrame>(get_err(fmt));
    auto frame = HdlcFrame{};
    frame.format = get_val(fmt);

    // Parse addresses
    // Destination first, then source
    std::vector<uint8_t> dest_bytes, src_bytes;
    {
        ByteBuffer addr_buf(buf.remaining_bytes().data(), buf.remaining_bytes().size());
        // Read destination
        while (addr_buf.has_data()) {
            auto b = addr_buf.read_u8(); if(!is_ok(b)) break;
            dest_bytes.push_back(get_val(b));
            if (get_val(b) & 0x01) break;
        }
        // Read source
        while (addr_buf.has_data()) {
            auto b = addr_buf.read_u8(); if(!is_ok(b)) break;
            src_bytes.push_back(get_val(b));
            if (get_val(b) & 0x01) break;
        }
    }

    frame.dest = HdlcAddress::parse(dest_bytes.data(), dest_bytes.size(), HdlcAddress::Type::Client);
    frame.source = HdlcAddress::parse(src_bytes.data(), src_bytes.size(), HdlcAddress::Type::Server);

    // Skip past addresses in main buffer
    buf.set_position(2 + dest_bytes.size() + src_bytes.size());

    auto ctrl_b = buf.read_u8();
    if (!is_ok(ctrl_b)) return make_err<HdlcFrame>(get_err(ctrl_b));
    frame.control = ControlField::from_byte(get_val(ctrl_b));

    // HCS for I-frames and U-frames with info
    if (!frame.information.empty() || frame.control.type == FrameType::Information || frame.control.type == FrameType::UI) {
        auto hcs = buf.read_u16();
        if (!is_ok(hcs)) return make_err<HdlcFrame>(get_err(hcs));
        // TODO: verify HCS
    }

    // Remaining is information (before FCS)
    size_t info_start = buf.position();
    size_t fcs_pos = content.size() - 2;
    if (info_start < fcs_pos) {
        frame.information.assign(content.begin() + info_start, content.begin() + fcs_pos);
    }

    // FCS verification
    auto fcs = Crc16::calculate(content.data(), content.size() - 2);
    if (fcs[0] != content[content.size()-2] || fcs[1] != content[content.size()-1])
        return make_err<HdlcFrame>(DlmsError::CrcError);

    return frame;
}

std::vector<uint8_t> HdlcFrame::to_bytes() const {
    ByteBuffer buf;
    format.to_bytes(buf);
    auto db = dest.to_bytes();
    auto sb = source.to_bytes();
    buf.write_bytes(db);
    buf.write_bytes(sb);
    buf.write_u8(control.to_byte());

    // HCS for frames with information
    bool has_info = !information.empty();
    if (has_info) {
        auto content = buf.vector();
        auto hcs = Crc16::calculate(content.data(), content.size());
        buf.write_u8(hcs[0]);
        buf.write_u8(hcs[1]);
        buf.write_bytes(information);
    }

    // Calculate FCS
    auto content = buf.vector();
    auto fcs = Crc16::calculate(content.data(), content.size());
    buf.write_u8(fcs[0]);
    buf.write_u8(fcs[1]);

    // Build final frame with escaping and flags
    auto escaped = hdlc_escape(buf.data(), buf.size());
    std::vector<uint8_t> frame;
    frame.push_back(HDLC_FLAG_BYTE);
    frame.insert(frame.end(), escaped.begin(), escaped.end());
    frame.push_back(HDLC_FLAG_BYTE);

    return frame;
}

} // namespace dlms::hdlc
