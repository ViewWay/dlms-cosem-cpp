#pragma once
#include <dlms/core/types.hpp>
#include <cstdint>
#include <vector>
#include <optional>

namespace dlms::security {

struct SecurityControlField {
    uint8_t security_suite = 0;
    bool authenticated = false;
    bool encrypted = false;
    bool broadcast_key = false;
    bool compressed = false;

    static SecurityControlField from_bytes(uint8_t b) {
        SecurityControlField sc;
        sc.security_suite = b & 0x0F;
        sc.authenticated = (b >> 4) & 0x01;
        sc.encrypted = (b >> 5) & 0x01;
        sc.broadcast_key = (b >> 6) & 0x01;
        sc.compressed = (b >> 7) & 0x01;
        return sc;
    }
    uint8_t to_byte() const {
        uint8_t b = security_suite & 0x0F;
        if (authenticated) b |= 0x10;
        if (encrypted) b |= 0x20;
        if (broadcast_key) b |= 0x40;
        if (compressed) b |= 0x80;
        return b;
    }
};

struct SecuritySuite {
    int number;
    int key_length;
    const char* algorithm;

    static SecuritySuite from_number(int n);
    bool validate_key(const std::vector<uint8_t>& key) const;
};

// AES-GCM encrypt/decrypt (requires OpenSSL)
#ifdef DLMS_HAS_OPENSSL
Result<std::vector<uint8_t>> aes_gcm_encrypt(
    const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv,
    const std::vector<uint8_t>& aad, const std::vector<uint8_t>& plaintext,
    int tag_len = DLMS_GCM_TAG_LENGTH);

Result<std::vector<uint8_t>> aes_gcm_decrypt(
    const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv,
    const std::vector<uint8_t>& aad, const std::vector<uint8_t>& ciphertext,
    int tag_len = DLMS_GCM_TAG_LENGTH);
#endif

// DLMS encrypt/decrypt wrappers
Result<std::vector<uint8_t>> encrypt(
    const SecurityControlField& sc,
    const std::vector<uint8_t>& system_title,
    uint32_t invocation_counter,
    const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& plaintext,
    const std::vector<uint8_t>& auth_key);

Result<std::vector<uint8_t>> decrypt(
    const SecurityControlField& sc,
    const std::vector<uint8_t>& system_title,
    uint32_t invocation_counter,
    const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& ciphertext,
    const std::vector<uint8_t>& auth_key);

Result<std::vector<uint8_t>> gmac(
    const SecurityControlField& sc,
    const std::vector<uint8_t>& system_title,
    uint32_t invocation_counter,
    const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& auth_key,
    const std::vector<uint8_t>& challenge);

// HMAC-SHA256 based KDF for HLS-ISM
std::vector<uint8_t> kdf_hmac_sha256(
    const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& context,
    size_t length);

std::vector<uint8_t> derive_encryption_key(
    const std::vector<uint8_t>& master_key,
    const std::vector<uint8_t>& system_title,
    int suite);

std::vector<uint8_t> derive_authentication_key(
    const std::vector<uint8_t>& master_key,
    const std::vector<uint8_t>& system_title,
    int suite);

} // namespace dlms::security
