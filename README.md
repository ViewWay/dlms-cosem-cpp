# dlms-cosem-cpp

**Complete DLMS/COSEM protocol stack for C++17** — header-only COSEM IC classes with full ASN.1, A-XDR, HDLC, and security support.

[![Tests](https://img.shields.io/badge/tests-7%20suites%20passed-brightgreen)]()
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![License: BSL 1.1](https://img.shields.io/badge/license-BSL%201.1-orange.svg)]()

## Features

- **ASN.1 BER**: Tag-length-value encoding/decoding
- **A-XDR Codec**: DLMS data encoding
- **HDLC Framing**: LLC/MAC layer with CRC-16
- **COSEM IC Classes**: 20+ interface classes for smart metering
- **Security**: SM4 block cipher, AES-GCM, SM4-GMAC
- **Transport**: TCP and mock transports

## Project Structure

```
├── include/dlms/
│   ├── asn1/      # ASN.1 BER types
│   ├── axdr/      # A-XDR codec
│   ├── cosem/     # COSEM IC classes (header-only)
│   ├── hdlc/      # HDLC framing
│   └── security/  # Crypto (SM4, AES-GCM)
├── src/
│   └── dlms/      # Implementation files
└── tests/         # Test suites
```

## Building

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
ctest --output-on-failure
```

## COSEM IC Classes

Activity Calendar, Association LN, Disconnect Control, GPRS Modem Setup, Image Transfer, Limiter, M-Bus Client, Push Setup, Register Monitor, Special Days Table, and more.

## Multi-Language Family

| Language | Tests | Lines |
|----------|-------|-------|
| [Python](https://github.com/ViewWay/dlms-cosem) | 5,146 | 37K |
| [Rust](https://github.com/ViewWay/dlms-cosem-rust) | 739 | 21K |
| [Go](https://github.com/ViewWay/dlms-cosem-go) | 362 | 8K |
| **C++** | **280+** | **6.5K** |
| [C](https://github.com/ViewWay/dlms-cosem-c) | 36 | 6.2K |

## License

BSL 1.1
