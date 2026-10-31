#pragma once

#include "common/crypto/key_types.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace hypercom::crypto {

constexpr std::size_t RECOVERY_ENTROPY_SIZE = 16;
constexpr std::size_t RECOVERY_WORD_COUNT = 12;

using recovery_entropy = std::array<std::uint8_t, RECOVERY_ENTROPY_SIZE>;

// A 12-word recovery phrase for the master seed.
//
// Format: BIP-39 -- 128 bits of entropy plus a 4-bit SHA-256 checksum,
// cut into twelve 11-bit indexes into the English word list. Using the
// standard encoding means the phrase can be checked by existing tools and
// that a typo is caught by the checksum instead of silently producing a
// different (empty) identity.
//
// The master seed is NOT the entropy itself: it is derived from it
// (derive_master_seed), so the phrase and the 32-byte seed stay distinct
// secrets with a one-way link, and the seed's size does not dictate the
// phrase's length.

void generate_recovery_entropy(recovery_entropy &out);

[[nodiscard]] bool entropy_to_phrase(recovery_entropy const &entropy,
                                     std::string &out);

// Accepts any amount of whitespace and any letter case. Fails -- and
// writes nothing -- on a wrong word count, an unknown word, or a bad
// checksum.
[[nodiscard]] bool phrase_to_entropy(std::string_view phrase,
                                     recovery_entropy &out);

[[nodiscard]] bool derive_master_seed(recovery_entropy const &entropy,
                                      ed25519_seed &out);

} // namespace hypercom::crypto
