#pragma once

#include "common/crypto/key_types.hpp"

#include <cstdint>
#include <span>

namespace hypercom::crypto {

// HKDF-SHA256 (RFC 5869) built on libsodium's HMAC-SHA256.
//
// Serves two clients: the Noise framework's HKDF function, and deriving
// session keys for private messages. Both have exactly the same needs,
// there's no reason to write two versions of it.

[[nodiscard]] bool
extract_pseudo_random_key(std::span<std::uint8_t const> salt,
                          std::span<std::uint8_t const> input_key_material,
                          symmetric_key &out);

// A single output block is enough: all our keys are 32 bytes, exactly
// SHA-256's output size. block_index starts at 1.
[[nodiscard]] bool expand_key_block(symmetric_key const &pseudo_random_key,
                                    std::span<std::uint8_t const> info,
                                    std::uint8_t block_index,
                                    symmetric_key &out);

// HKDF(chaining_key, ikm) -> two outputs, exactly the primitive described
// by the Noise specification as HKDF(..., num_outputs = 2).
[[nodiscard]] bool
derive_key_pair(symmetric_key const &chaining_key,
                std::span<std::uint8_t const> input_key_material,
                symmetric_key &first, symmetric_key &second);

} // namespace hypercom::crypto
