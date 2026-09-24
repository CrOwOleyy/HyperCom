#pragma once

#include "common/crypto/key_types.hpp"

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace hypercom::crypto {

// CipherState from the Noise specification: a key, a nonce counter.
//
// The counter is NEVER reset or reused. Reusing a nonce with
// ChaCha20-Poly1305 doesn't degrade security, it destroys it: two messages
// under the same nonce reveal the XOR of the plaintexts and enable
// forgery. This is why the counter is internal and no method lets you set
// it.
class noise_cipher_state {
public:
    noise_cipher_state();

    void initialize_key(symmetric_key const &key);

    [[nodiscard]] bool has_key() const;

    // Without a key, the plaintext is copied through unchanged: this is the
    // behavior mandated by the specification for handshake steps before the
    // first MixKey.
    [[nodiscard]] bool
    encrypt_with_ad(std::span<std::uint8_t const> associated_data,
                    std::span<std::uint8_t const> plaintext,
                    std::vector<std::uint8_t> &out);

    [[nodiscard]] bool
    decrypt_with_ad(std::span<std::uint8_t const> associated_data,
                    std::span<std::uint8_t const> ciphertext,
                    std::vector<std::uint8_t> &out);

private:
    void
    build_nonce(std::array<std::uint8_t, CHACHA_IETF_NONCE_SIZE> &out) const;

    symmetric_key key_;
    std::uint64_t nonce_counter_;
    bool has_key_;
};

} // namespace hypercom::crypto
