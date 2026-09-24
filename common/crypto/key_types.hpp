#pragma once

#include <array>
#include <cstdint>

namespace hypercom::crypto {

// Sizes fixed by libsodium, redeclared here so the project's headers don't
// force <sodium.h> on everyone. The values are checked with static_assert
// in sodium_runtime.cpp: a mismatch breaks the build instead of producing
// a silent overflow.

constexpr std::size_t ED25519_PUBLIC_KEY_SIZE = 32;
constexpr std::size_t ED25519_SECRET_KEY_SIZE = 64;
constexpr std::size_t ED25519_SIGNATURE_SIZE = 64;
constexpr std::size_t ED25519_SEED_SIZE = 32;
constexpr std::size_t X25519_PUBLIC_KEY_SIZE = 32;
constexpr std::size_t X25519_SECRET_KEY_SIZE = 32;
constexpr std::size_t SYMMETRIC_KEY_SIZE = 32;
constexpr std::size_t CHACHA_IETF_NONCE_SIZE = 12;
constexpr std::size_t XCHACHA_NONCE_SIZE = 24;
constexpr std::size_t AEAD_TAG_SIZE = 16;
constexpr std::size_t ARGON2ID_SALT_SIZE = 16;

using ed25519_public_key = std::array<std::uint8_t, ED25519_PUBLIC_KEY_SIZE>;
using ed25519_secret_key = std::array<std::uint8_t, ED25519_SECRET_KEY_SIZE>;
using ed25519_signature = std::array<std::uint8_t, ED25519_SIGNATURE_SIZE>;
using ed25519_seed = std::array<std::uint8_t, ED25519_SEED_SIZE>;
using x25519_public_key = std::array<std::uint8_t, X25519_PUBLIC_KEY_SIZE>;
using x25519_secret_key = std::array<std::uint8_t, X25519_SECRET_KEY_SIZE>;
using symmetric_key = std::array<std::uint8_t, SYMMETRIC_KEY_SIZE>;

} // namespace hypercom::crypto
