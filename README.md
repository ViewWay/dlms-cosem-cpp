# DLMS/COSEM C++ Library

A complete C++17 implementation of the DLMS/COSEM protocol stack for smart metering.

## Features

- **Core Types**: OBIS codes, DLMS variant types, date/time, byte buffer
- **HDLC**: Full frame encoding/decoding (SNRM, UA, RR, I-frame, Disconnect), CRC-16, byte stuffing
- **AXDR**: Complete A-XDR encoder/decoder for all DLMS data types
- **ASN.1 BER**: BER-TLV encoder/decoder, AARQ/AARE APDU support
- **Security**: Security Suite 0-5, AES-GCM (OpenSSL), SM4-GCM/GMAC (pure C++)
- **COSEM IC Classes**: Data, Register, ExtendedRegister, DemandRegister, Clock,
  ProfileGeneric, SecuritySetup, TariffPlan, TariffTable, LPSetup, RS485Setup,
  NBIoTSetup, LoRaWANSetup
- **Client**: DlmsClient with TCP and Serial transport support
- **Cross-platform**: Linux, macOS, Windows
- **No exceptions**: Uses `Result<T>` (variant-based) instead of exceptions
- **Header-only** core with optional compiled SM4 implementation

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
ctest --output-on-failure
```

## Dependencies

- CMake 3.16+
- C++17 compiler
- OpenSSL (optional, for AES-GCM)
- Google Test (optional, for tests)

## Quick Example

```cpp
#include <dlms/dlms.hpp>

int main() {
    // Create OBIS code
    auto obis = dlms::ObisCode(1, 0, 1, 8, 0, 255);
    std::cout << obis.to_canonical() << std::endl; // "1-0:1.8.0.255"

    // Encode/decode DLMS data
    dlms::axdr::AxdEncoder enc;
    enc.encode_uint32(123456);
    auto data = enc.data();

    dlms::axdr::AxdDecoder dec(data);
    auto result = dec.decode();
    if (dlms::is_ok(result)) {
        auto val = std::get<uint32_t>(dlms::get_val(result));
        std::cout << "Decoded: " << val << std::endl;
    }

    // SM4 encryption (pure C++, no dependencies)
    uint8_t key[16] = {/* your key */};
    dlms::security::Sm4 sm4(key);
    uint8_t block[16] = {/* plaintext */};
    auto encrypted = sm4.encrypt_block(block);
    auto decrypted = sm4.decrypt_block(encrypted.data());

    return 0;
}
```

## License

MIT
