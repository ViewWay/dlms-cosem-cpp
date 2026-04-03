#include <dlms/transport/transport.hpp>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t = SOCKET;
#define INVALID_SOCK INVALID_SOCKET
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
using socket_t = int;
#define INVALID_SOCK (-1)
#define closesocket close
#endif

namespace dlms::transport {

VoidResult TcpTransport::connect(const std::string& address, uint16_t port) {
#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
#endif
    socket_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ == INVALID_SOCK) return make_err<std::monostate>(DlmsError::ConnectionError);

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

    if (::connect(socket_fd_, (struct sockaddr*)&addr, sizeof(addr)) != 0)
        return make_err<std::monostate>(DlmsError::ConnectionError);
    return make_void_ok();
}

VoidResult TcpTransport::disconnect() {
    if (socket_fd_ != INVALID_SOCK) { closesocket(socket_fd_); socket_fd_ = INVALID_SOCK; }
    return make_void_ok();
}

Result<std::vector<uint8_t>> TcpTransport::send_and_receive(const std::vector<uint8_t>& data, int timeout_ms) {
    if (socket_fd_ == INVALID_SOCK) return make_err<std::vector<uint8_t>>(DlmsError::ConnectionError);

    ::send(socket_fd_, data.data(), data.size(), 0);

    std::vector<uint8_t> response(4096);
#ifndef _WIN32
    struct pollfd pfd = {socket_fd_, POLLIN, 0};
    int ready = poll(&pfd, 1, timeout_ms);
    if (ready <= 0) return make_err<std::vector<uint8_t>>(DlmsError::Timeout);
#endif
    int n = ::recv(socket_fd_, response.data(), response.size(), 0);
    if (n <= 0) return make_err<std::vector<uint8_t>>(DlmsError::ConnectionError);
    response.resize(n);
    return response;
}

bool TcpTransport::is_connected() const { return socket_fd_ != INVALID_SOCK; }

SerialTransport::SerialTransport(const std::string& port, uint32_t baud_rate)
    : port_(port), baud_rate_(baud_rate) {}

VoidResult SerialTransport::connect(const std::string&, uint16_t) {
    return make_err<std::monostate>(DlmsError::NotSupported);
}

VoidResult SerialTransport::disconnect() { connected_ = false; return make_void_ok(); }

Result<std::vector<uint8_t>> SerialTransport::send_and_receive(const std::vector<uint8_t>&, int) {
    return make_err<std::vector<uint8_t>>(DlmsError::NotSupported);
}

bool SerialTransport::is_connected() const { return connected_; }

} // namespace dlms::transport
