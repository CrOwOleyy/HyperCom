#include "common/crypto/noise_cipher_state.hpp"

#include <limits>

#include <sodium.h>

namespace hypercom::crypto {

noise_cipher_state::noise_cipher_state()
    : key_{}, nonce_counter_{0}, has_key_{false}
{
}

void noise_cipher_state::initialize_key(symmetric_key const &key)
{
    key_ = key;
    nonce_counter_ = 0;
    has_key_ = true;
}

bool noise_cipher_state::has_key() const
{
    return has_key_;
}

void noise_cipher_state::build_nonce(
    std::array<std::uint8_t, CHACHA_IETF_NONCE_SIZE> &out) const
{
    // Specification Noise pour ChaChaPoly : quatre octets nuls, puis le
    // compteur sur 8 octets en petit-boutiste.
    out.fill(0);
    for (std::size_t index = 0; index < sizeof(nonce_counter_); ++index) {
        out[4 + index] =
            static_cast<std::uint8_t>((nonce_counter_ >> (index * 8U)) & 0xFFU);
    }
}

bool noise_cipher_state::encrypt_with_ad(
    std::span<std::uint8_t const> associated_data,
    std::span<std::uint8_t const> plaintext, std::vector<std::uint8_t> &out)
{
    if (!has_key_) {
        out.assign(plaintext.begin(), plaintext.end());
        return true;
    }
    if (nonce_counter_ == std::numeric_limits<std::uint64_t>::max()) {
        return false;
    }
    std::array<std::uint8_t, CHACHA_IETF_NONCE_SIZE> nonce{};
    build_nonce(nonce);
    out.resize(plaintext.size() + AEAD_TAG_SIZE);
    unsigned long long written = 0;
    if (crypto_aead_chacha20poly1305_ietf_encrypt(
            out.data(), &written, plaintext.data(), plaintext.size(),
            associated_data.data(), associated_data.size(), nullptr,
            nonce.data(), key_.data())
        != 0) {
        return false;
    }
    out.resize(static_cast<std::size_t>(written));
    ++nonce_counter_;
    return true;
}

bool noise_cipher_state::decrypt_with_ad(
    std::span<std::uint8_t const> associated_data,
    std::span<std::uint8_t const> ciphertext, std::vector<std::uint8_t> &out)
{
    if (!has_key_) {
        out.assign(ciphertext.begin(), ciphertext.end());
        return true;
    }
    if (ciphertext.size() < AEAD_TAG_SIZE) {
        return false;
    }
    if (nonce_counter_ == std::numeric_limits<std::uint64_t>::max()) {
        return false;
    }
    std::array<std::uint8_t, CHACHA_IETF_NONCE_SIZE> nonce{};
    build_nonce(nonce);
    std::vector<std::uint8_t> decoded(ciphertext.size() - AEAD_TAG_SIZE);
    unsigned long long written = 0;
    if (crypto_aead_chacha20poly1305_ietf_decrypt(
            decoded.data(), &written, nullptr, ciphertext.data(),
            ciphertext.size(), associated_data.data(), associated_data.size(),
            nonce.data(), key_.data())
        != 0) {
        return false;
    }
    decoded.resize(static_cast<std::size_t>(written));
    out = std::move(decoded);
    // Le compteur n'avance qu'en cas de succes : un message rejete ne doit pas
    // faire sauter un nonce et desynchroniser la session.
    ++nonce_counter_;
    return true;
}

} // namespace hypercom::crypto
