#pragma once

#include "common/crypto/noise_symmetric_state.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace hypercom::crypto {

// EncryptAndHash / DecryptAndHash from the Noise specification.
//
// The running handshake hash serves as associated data. This is what ties
// each message to everything exchanged before it: changing one byte of an
// earlier message invalidates the authentication of every message that
// follows. An attacker therefore cannot remove, reorder, or substitute a
// handshake step without the rest failing.

[[nodiscard]] bool encrypt_and_hash(noise_symmetric_state &state,
                                    std::span<std::uint8_t const> plaintext,
                                    std::vector<std::uint8_t> &out);

[[nodiscard]] bool decrypt_and_hash(noise_symmetric_state &state,
                                    std::span<std::uint8_t const> ciphertext,
                                    std::vector<std::uint8_t> &out);

} // namespace hypercom::crypto
