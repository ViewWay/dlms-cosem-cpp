#include <dlms/security/sm4.hpp>
#include <dlms/core/types.hpp>
#include <cstring>
#include <algorithm>

namespace dlms::security {

// SM4 S-Box
static constexpr uint8_t SBOX[256] = {
    0xd6,0x90,0xe9,0xfe,0xcc,0xe1,0x3d,0xb7,0x16,0xb6,0x14,0xc2,0x28,0xfb,0x2c,0x05,
    0x2b,0x67,0x9a,0x76,0x2a,0xbe,0x04,0xc3,0xaa,0x44,0x13,0x26,0x49,0x86,0x06,0x99,
    0x9c,0x42,0x50,0xf4,0x91,0xef,0x98,0x7a,0x33,0x54,0x0b,0x43,0xed,0xcf,0xac,0x62,
    0xe4,0xb3,0x1c,0xa9,0xc9,0x08,0xe8,0x95,0x80,0xdf,0x94,0xfa,0x75,0x8f,0x3f,0xa6,
    0x47,0x07,0xa7,0xfc,0xf3,0x73,0x17,0xba,0x83,0x59,0x3c,0x19,0xe6,0x85,0x4f,0xa8,
    0x68,0x6b,0x81,0xb2,0x71,0x64,0xda,0x8b,0xf8,0xeb,0x0f,0x4b,0x70,0x56,0x9d,0x35,
    0x1e,0x24,0x0e,0x5e,0x63,0x58,0xd1,0xa2,0x25,0x22,0x7c,0x3b,0x01,0x21,0x78,0x87,
    0xd4,0x00,0x46,0x57,0x9f,0xd3,0x27,0x52,0x4c,0x36,0x02,0xe7,0xa0,0xc4,0xc8,0x9e,
    0xea,0xbf,0x8a,0xd2,0x40,0xc7,0x38,0xb5,0xa3,0xf7,0xf2,0xce,0xf9,0x61,0x15,0xa1,
    0xe0,0xae,0x5d,0xa4,0x9b,0x34,0x1a,0x55,0xad,0x93,0x32,0x30,0xf5,0x8c,0xb1,0xe3,
    0x1d,0xf6,0xe2,0x2e,0x82,0x66,0xca,0x60,0xc0,0x29,0x23,0xab,0x0d,0x53,0x4e,0x6f,
    0xd5,0xdb,0x37,0x45,0xde,0xfd,0x8e,0x2f,0x03,0xff,0x6a,0x72,0x6d,0x6c,0x5b,0x51,
    0x8d,0x1b,0xaf,0x92,0xbb,0xdd,0xbc,0x7f,0x11,0xd9,0x5c,0x41,0x1f,0x10,0x5a,0xd8,
    0x0a,0xc1,0x31,0x88,0xa5,0xcd,0x7b,0xbd,0x2d,0x74,0xd0,0x12,0xb8,0xe5,0xb4,0xb0,
    0x89,0x69,0x97,0x4a,0x0c,0x96,0x77,0x7e,0x65,0xb9,0xf1,0x09,0xc5,0x6e,0xc6,0x84,
    0x18,0xf0,0x7d,0xec,0x3a,0xdc,0x4d,0x20,0x79,0xee,0x5f,0x3e,0xd7,0xcb,0x39,0x48,
};

static constexpr uint8_t CK[32] = {
    0x00,0x0e,0x09,0x00,0x04,0x01,0x02,0x04,0x08,0x03,0x06,0x01,0x0c,0x05,0x02,0x07,
    0x0a,0x08,0x0f,0x09,0x0b,0x0e,0x03,0x0f,0x07,0x00,0x09,0x06,0x0d,0x0b,0x05,0x04,
};

static constexpr uint32_t FK[4] = {0xa3b1bac6, 0x56aa3350, 0x677d9197, 0xb27022dc};

static inline uint32_t rotl32(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }

uint32_t Sm4::tau(const std::array<uint8_t, 4>& in) const {
    return (uint32_t(SBOX[in[0]]) << 24) | (uint32_t(SBOX[in[1]]) << 16) |
           (uint32_t(SBOX[in[2]]) << 8) | SBOX[in[3]];
}

uint32_t Sm4::l_transform(const std::array<uint8_t, 4>& in) const {
    uint32_t b = tau(in);
    return b ^ rotl32(b, 2) ^ rotl32(b, 10) ^ rotl32(b, 18) ^ rotl32(b, 24);
}

uint32_t Sm4::feistel(uint32_t x) const {
    std::array<uint8_t, 4> b = {
        static_cast<uint8_t>((x >> 24) & 0xFF),
        static_cast<uint8_t>((x >> 16) & 0xFF),
        static_cast<uint8_t>((x >> 8) & 0xFF),
        static_cast<uint8_t>(x & 0xFF),
    };
    uint32_t t = tau(b);
    return t ^ rotl32(t, 13) ^ rotl32(t, 23);
}

void Sm4::key_expansion(const uint8_t key[KEY_SIZE]) {
    uint32_t mk[4];
    for (int i = 0; i < 4; ++i)
        mk[i] = (uint32_t(key[4*i]) << 24) | (uint32_t(key[4*i+1]) << 16) |
                 (uint32_t(key[4*i+2]) << 8) | key[4*i+3];

    uint32_t k[36];
    for (int i = 0; i < 4; ++i) k[i] = mk[i] ^ FK[i];

    for (int i = 0; i < 32; ++i) {
        uint32_t ki = k[i+1] ^ k[i+2] ^ k[i+3] ^ CK[i];
        std::array<uint8_t, 4> kb = {
            static_cast<uint8_t>((ki >> 24) & 0xFF),
            static_cast<uint8_t>((ki >> 16) & 0xFF),
            static_cast<uint8_t>((ki >> 8) & 0xFF),
            static_cast<uint8_t>(ki & 0xFF),
        };
        k[i+4] = k[i] ^ l_transform(kb);
        rk_[i] = k[i+4];
    }
}

Sm4::Sm4(const uint8_t key[KEY_SIZE]) { key_expansion(key); }

std::vector<uint8_t> Sm4::encrypt_block(const uint8_t block[BLOCK_SIZE]) const {
    uint32_t x[4];
    for (int i = 0; i < 4; ++i)
        x[i] = (uint32_t(block[4*i]) << 24) | (uint32_t(block[4*i+1]) << 16) |
                (uint32_t(block[4*i+2]) << 8) | block[4*i+3];

    for (int i = 0; i < 32; ++i) {
        uint32_t tmp = x[1] ^ x[2] ^ x[3] ^ rk_[i];
        x[0] ^= feistel(tmp);
        // Rotate
        uint32_t t = x[0]; x[0] = x[1]; x[1] = x[2]; x[2] = x[3]; x[3] = t;
    }

    // Reverse last round
    std::vector<uint8_t> out(BLOCK_SIZE);
    uint32_t y[4] = {x[3], x[2], x[1], x[0]};
    for (int i = 0; i < 4; ++i) {
        out[4*i]   = (y[i] >> 24) & 0xFF;
        out[4*i+1] = (y[i] >> 16) & 0xFF;
        out[4*i+2] = (y[i] >> 8) & 0xFF;
        out[4*i+3] = y[i] & 0xFF;
    }
    return out;
}

std::vector<uint8_t> Sm4::decrypt_block(const uint8_t block[BLOCK_SIZE]) const {
    uint32_t x[4];
    for (int i = 0; i < 4; ++i)
        x[i] = (uint32_t(block[4*i]) << 24) | (uint32_t(block[4*i+1]) << 16) |
                (uint32_t(block[4*i+2]) << 8) | block[4*i+3];

    for (int i = 0; i < 32; ++i) {
        uint32_t tmp = x[1] ^ x[2] ^ x[3] ^ rk_[31-i];
        x[0] ^= feistel(tmp);
        uint32_t t = x[0]; x[0] = x[1]; x[1] = x[2]; x[2] = x[3]; x[3] = t;
    }

    std::vector<uint8_t> out(BLOCK_SIZE);
    uint32_t y[4] = {x[3], x[2], x[1], x[0]};
    for (int i = 0; i < 4; ++i) {
        out[4*i]   = (y[i] >> 24) & 0xFF;
        out[4*i+1] = (y[i] >> 16) & 0xFF;
        out[4*i+2] = (y[i] >> 8) & 0xFF;
        out[4*i+3] = y[i] & 0xFF;
    }
    return out;
}

// Simple CTR mode for SM4-GCM (simplified - production should use proper GCM with GHASH)
std::vector<uint8_t> Sm4::gcm_encrypt(const std::vector<uint8_t>& nonce,
                                        const std::vector<uint8_t>& aad,
                                        const std::vector<uint8_t>& plaintext,
                                        int tag_len) {
    // CTR mode encryption
    std::vector<uint8_t> ciphertext(plaintext.size());
    uint8_t counter[BLOCK_SIZE] = {};
    std::copy(nonce.begin(), nonce.end(), counter);
    size_t offset = 0;
    while (offset < plaintext.size()) {
        auto keystream = encrypt_block(counter);
        for (size_t i = 0; i < BLOCK_SIZE && offset < plaintext.size(); ++i, ++offset) {
            ciphertext[offset] = plaintext[offset] ^ keystream[i];
        }
        // Increment counter (big-endian, last 4 bytes)
        for (int i = BLOCK_SIZE - 1; i >= BLOCK_SIZE - 4; --i) {
            if (++counter[i] != 0) break;
        }
    }

    // Simple tag computation (XOR of all blocks) - simplified GHASH
    std::vector<uint8_t> tag(tag_len, 0);
    for (size_t i = 0; i < plaintext.size(); ++i) {
        tag[i % tag_len] ^= plaintext[i];
    }
    for (auto b : aad) tag[b % tag_len] ^= b;
    ciphertext.insert(ciphertext.end(), tag.begin(), tag.end());
    return ciphertext;
}

Result<std::vector<uint8_t>> Sm4::gcm_decrypt(const std::vector<uint8_t>& nonce,
                                               const std::vector<uint8_t>& aad,
                                               const std::vector<uint8_t>& ciphertext,
                                               int tag_len) {
    if (ciphertext.size() < static_cast<size_t>(tag_len))
        return make_err<std::vector<uint8_t>>(DlmsError::DecryptionError);

    auto ct = std::vector<uint8_t>(ciphertext.begin(), ciphertext.end() - tag_len);
    auto tag = std::vector<uint8_t>(ciphertext.end() - tag_len, ciphertext.end());

    // CTR mode decryption
    std::vector<uint8_t> plaintext(ct.size());
    uint8_t counter[BLOCK_SIZE] = {};
    std::copy(nonce.begin(), nonce.end(), counter);
    size_t offset = 0;
    while (offset < ct.size()) {
        auto keystream = encrypt_block(counter);
        for (size_t i = 0; i < BLOCK_SIZE && offset < ct.size(); ++i, ++offset) {
            plaintext[offset] = ct[offset] ^ keystream[i];
        }
        for (int i = BLOCK_SIZE - 1; i >= BLOCK_SIZE - 4; --i) {
            if (++counter[i] != 0) break;
        }
    }

    // Verify tag (must use plaintext, matching encrypt logic)
    std::vector<uint8_t> computed(tag_len, 0);
    for (size_t i = 0; i < plaintext.size(); ++i) computed[i % tag_len] ^= plaintext[i];
    for (auto b : aad) computed[b % tag_len] ^= b;
    if (computed != tag) return make_err<std::vector<uint8_t>>(DlmsError::DecryptionError);
    return plaintext;
}

std::vector<uint8_t> Sm4::gmac(const std::vector<uint8_t>& nonce,
                                 const std::vector<uint8_t>& data, int tag_len) {
    auto result = gcm_encrypt(nonce, data, {}, tag_len);
    return std::vector<uint8_t>(result.begin(), result.end());
}

} // namespace dlms::security
