#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "common/crypto/key_types.hpp"

namespace hypercom::crypto {

// CipherState de la specification Noise : une cle, un compteur de nonce.
//
// Le compteur n'est JAMAIS remis a zero ni reutilise. Reutiliser un nonce avec
// ChaCha20-Poly1305 ne degrade pas la securite, il la detruit : deux messages
// sous le meme nonce revelent le XOR des clairs et permettent de forger.
// C'est la raison pour laquelle le compteur est interne et qu'aucune methode
// ne permet de le positionner.
class noise_cipher_state {
public:
    noise_cipher_state();

    void initialize_key(symmetric_key const &key);

    [[nodiscard]] bool has_key() const;

    // Sans cle, le clair est recopie tel quel : c'est le comportement impose
    // par la specification pour les etapes de handshake anterieures au premier
    // MixKey.
    [[nodiscard]] bool encrypt_with_ad(
        std::span<std::uint8_t const> associated_data,
        std::span<std::uint8_t const> plaintext,
        std::vector<std::uint8_t> &out);

    [[nodiscard]] bool decrypt_with_ad(
        std::span<std::uint8_t const> associated_data,
        std::span<std::uint8_t const> ciphertext,
        std::vector<std::uint8_t> &out);

private:
    void build_nonce(
        std::array<std::uint8_t, CHACHA_IETF_NONCE_SIZE> &out) const;

    symmetric_key key_;
    std::uint64_t nonce_counter_;
    bool has_key_;
};

} // namespace hypercom::crypto
