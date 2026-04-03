#pragma once
#include <dlms/core/types.hpp>
#include <vector>
#include <cstdint>
#include <string>
#include <functional>

namespace dlms::transport {

class ITransport {
public:
    virtual ~ITransport() = default;
    virtual VoidResult connect(const std::string& address, uint16_t port) = 0;
    virtual VoidResult disconnect() = 0;
    virtual Result<std::vector<uint8_t>> send_and_receive(const std::vector<uint8_t>& data, int timeout_ms = 5000) = 0;
    virtual bool is_connected() const = 0;
};

class TcpTransport : public ITransport {
public:
    ~TcpTransport() override { disconnect(); }
    VoidResult connect(const std::string& address, uint16_t port) override;
    VoidResult disconnect() override;
    Result<std::vector<uint8_t>> send_and_receive(const std::vector<uint8_t>& data, int timeout_ms = 5000) override;
    bool is_connected() const override;
private:
    int socket_fd_ = -1;
#ifdef _WIN32
    void* wsadata_ = nullptr;
#endif
};

class SerialTransport : public ITransport {
public:
    SerialTransport(const std::string& port, uint32_t baud_rate = 9600);
    ~SerialTransport() override { disconnect(); }
    VoidResult connect(const std::string& address, uint16_t port) override;
    VoidResult disconnect() override;
    Result<std::vector<uint8_t>> send_and_receive(const std::vector<uint8_t>& data, int timeout_ms = 5000) override;
    bool is_connected() const override;
private:
    std::string port_;
    uint32_t baud_rate_;
    bool connected_ = false;
};

} // namespace dlms::transport
