#pragma once
#include <dlms/core/types.hpp>
#include <dlms/config.hpp>
#include <vector>
#include <cstdint>
#include <array>

namespace dlms::security {

class Sm4 {
public:
    static constexpr size_t BLOCK_SIZE = 16;
    static constexpr size_t KEY_SIZE = 16;

    explicit Sm4(const uint8_t key[KEY_SIZE]);
    std::vector<uint8_t> encrypt_block(const uint8_t block[BLOCK_SIZE]) const;
    std::vector<uint8_t> decrypt_block(const uint8_t block[BLOCK_SIZE]) const;

    // GCM mode (CTR + GHASH)
    std::vector<uint8_t> gcm_encrypt(const std::vector<uint8_t>& nonce,
                                      const std::vector<uint8_t>& aad,
                                      const std::vector<uint8_t>& plaintext,
                                      int tag_len = DLMS_GCM_TAG_LENGTH);

    Result<std::vector<uint8_t>> gcm_decrypt(const std::vector<uint8_t>& nonce,
                                               const std::vector<uint8_t>& aad,
                                               const std::vector<uint8_t>& ciphertext,
                                               int tag_len = DLMS_GCM_TAG_LENGTH);

    // GMAC (authentication only)
    std::vector<uint8_t> gmac(const std::vector<uint8_t>& nonce,
                               const std::vector<uint8_t>& data,
                               int tag_len = DLMS_GCM_TAG_LENGTH);

private:
    void key_expansion(const uint8_t key[KEY_SIZE]);
    uint32_t tau(const std::array<uint8_t, 4>& in) const;
    uint32_t l_transform(const std::array<uint8_t, 4>& in) const;
    uint32_t feistel(uint32_t x) const;

    std::array<uint32_t, 32> rk_; // round keys
};

} // namespace dlms::security
