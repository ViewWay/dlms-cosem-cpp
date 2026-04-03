#include <dlms/security/security.hpp>
#include <dlms/config.hpp>
#include <cstring>

namespace dlms::security {

SecuritySuite SecuritySuite::from_number(int n) {
    switch (n) {
    case 0: case 1: return {n, 16, "AES-128-GCM"};
    case 2: return {n, 32, "AES-256-GCM"};
    case 3: return {n, 16, "AES-128-CCM"};
    case 4: return {n, 32, "AES-256-CCM"};
    case 5: return {n, 16, "SM4-GCM"};
    default: return {0, 16, "Unknown"};
    }
}

bool SecuritySuite::validate_key(const std::vector<uint8_t>& key) const {
    return key.size() == static_cast<size_t>(key_length);
}

#ifdef DLMS_HAS_OPENSSL
#include <openssl/evp.h>

Result<std::vector<uint8_t>> aes_gcm_encrypt(
    const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv,
    const std::vector<uint8_t>& aad, const std::vector<uint8_t>& plaintext, int tag_len)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return make_err<std::vector<uint8_t>>(DlmsError::EncryptionError);

    const EVP_CIPHER* cipher = (key.size() == 32) ? EVP_aes_256_gcm() : EVP_aes_128_gcm();
    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return make_err<std::vector<uint8_t>>(DlmsError::EncryptionError);
    }
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), nullptr);
    if (!aad.empty()) {
        EVP_EncryptUpdate(ctx, nullptr, nullptr, aad.data(), aad.size());
    }
    std::vector<uint8_t> ct(plaintext.size());
    int outlen = 0;
    EVP_EncryptUpdate(ctx, ct.data(), &outlen, plaintext.data(), plaintext.size());
    EVP_EncryptFinal_ex(ctx, ct.data() + outlen, &outlen);
    std::vector<uint8_t> tag(tag_len);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, tag_len, tag.data());
    EVP_CIPHER_CTX_free(ctx);
    ct.insert(ct.end(), tag.begin(), tag.end());
    return ct;
}

Result<std::vector<uint8_t>> aes_gcm_decrypt(
    const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv,
    const std::vector<uint8_t>& aad, const std::vector<uint8_t>& ciphertext, int tag_len)
{
    if (ciphertext.size() < static_cast<size_t>(tag_len))
        return make_err<std::vector<uint8_t>>(DlmsError::DecryptionError);

    auto ct = std::vector<uint8_t>(ciphertext.begin(), ciphertext.end() - tag_len);
    auto tag = std::vector<uint8_t>(ciphertext.end() - tag_len, ciphertext.end());

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return make_err<std::vector<uint8_t>>(DlmsError::DecryptionError);

    const EVP_CIPHER* cipher = (key.size() == 32) ? EVP_aes_256_gcm() : EVP_aes_128_gcm();
    if (EVP_DecryptInit_ex(ctx, cipher, nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return make_err<std::vector<uint8_t>>(DlmsError::DecryptionError);
    }
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), nullptr);
    if (!aad.empty()) {
        EVP_DecryptUpdate(ctx, nullptr, nullptr, aad.data(), aad.size());
    }
    std::vector<uint8_t> pt(ct.size());
    int outlen = 0;
    EVP_DecryptUpdate(ctx, pt.data(), &outlen, ct.data(), ct.size());
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag_len, tag.data());
    int rv = EVP_DecryptFinal_ex(ctx, pt.data() + outlen, &outlen);
    EVP_CIPHER_CTX_free(ctx);
    if (rv != 1) return make_err<std::vector<uint8_t>>(DlmsError::DecryptionError);
    return pt;
}
#endif // DLMS_HAS_OPENSSL

static std::vector<uint8_t> make_iv(const std::vector<uint8_t>& system_title, uint32_t ic) {
    std::vector<uint8_t> iv = system_title;
    iv.push_back((ic >> 24) & 0xFF);
    iv.push_back((ic >> 16) & 0xFF);
    iv.push_back((ic >> 8) & 0xFF);
    iv.push_back(ic & 0xFF);
    return iv;
}

Result<std::vector<uint8_t>> encrypt(
    const SecurityControlField& sc, const std::vector<uint8_t>& system_title,
    uint32_t invocation_counter, const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& auth_key)
{
#ifdef DLMS_HAS_OPENSSL
    auto iv = make_iv(system_title, invocation_counter);
    std::vector<uint8_t> aad;
    aad.push_back(sc.to_byte());
    aad.insert(aad.end(), auth_key.begin(), auth_key.end());
    return aes_gcm_encrypt(key, iv, aad, plaintext);
#else
    (void)sc; (void)system_title; (void)invocation_counter; (void)key; (void)plaintext; (void)auth_key;
    return make_err<std::vector<uint8_t>>(DlmsError::NotSupported);
#endif
}

Result<std::vector<uint8_t>> decrypt(
    const SecurityControlField& sc, const std::vector<uint8_t>& system_title,
    uint32_t invocation_counter, const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& auth_key)
{
#ifdef DLMS_HAS_OPENSSL
    auto iv = make_iv(system_title, invocation_counter);
    std::vector<uint8_t> aad;
    aad.push_back(sc.to_byte());
    aad.insert(aad.end(), auth_key.begin(), auth_key.end());
    return aes_gcm_decrypt(key, iv, aad, ciphertext);
#else
    (void)sc; (void)system_title; (void)invocation_counter; (void)key; (void)ciphertext; (void)auth_key;
    return make_err<std::vector<uint8_t>>(DlmsError::NotSupported);
#endif
}

Result<std::vector<uint8_t>> gmac(
    const SecurityControlField& sc, const std::vector<uint8_t>& system_title,
    uint32_t invocation_counter, const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& auth_key, const std::vector<uint8_t>& challenge)
{
    return encrypt(sc, system_title, invocation_counter, key, {}, auth_key);
    // Note: GMAC is GCM with empty plaintext; the result is just the tag.
    // The encrypt function returns ciphertext+tag, but since pt is empty, result = tag.
}

std::vector<uint8_t> kdf_hmac_sha256(
    const std::vector<uint8_t>& key, const std::vector<uint8_t>& context, size_t length)
{
#ifdef DLMS_HAS_OPENSSL
    std::vector<uint8_t> result;
    uint32_t counter = 1;
    while (result.size() < length) {
        // HMAC-SHA256(counter || context) - simplified; in practice use OpenSSL HMAC
        // For now return placeholder
        counter++;
    }
    result.resize(length);
    return result;
#else
    (void)key; (void)context; (void)length;
    return {};
#endif
}

std::vector<uint8_t> derive_encryption_key(const std::vector<uint8_t>& mk, const std::vector<uint8_t>& st, int suite) {
    auto ctx = st; ctx.insert(ctx.begin(), {'E','N','C'});
    int kl = (suite == 2 || suite == 4) ? 32 : 16;
    return kdf_hmac_sha256(mk, ctx, kl);
}

std::vector<uint8_t> derive_authentication_key(const std::vector<uint8_t>& mk, const std::vector<uint8_t>& st, int suite) {
    auto ctx = st; ctx.insert(ctx.begin(), {'M','A','C'});
    int kl = (suite == 2 || suite == 4) ? 32 : 16;
    return kdf_hmac_sha256(mk, ctx, kl);
}

} // namespace dlms::security
