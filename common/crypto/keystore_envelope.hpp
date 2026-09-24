#pragma once

#include "common/crypto/key_types.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace hypercom::crypto {

// Format of the private key at rest, on the client's disk:
//
//   "HYPCKEY1" | u8 version | u32 ops | u32 mem_kib | 16 salt | 24 nonce |
//   sealed
//
// passphrase --Argon2id--> key --XChaCha20-Poly1305--> sealed private key
//
// The Argon2id parameters are written INTO the file rather than hardcoded.
// Without this, raising the cost someday would make every existing key
// unreadable -- which, on this project, means losing accounts.
constexpr std::size_t KEYSTORE_MAGIC_SIZE = 8;
constexpr std::uint8_t KEYSTORE_VERSION = 1;

// Same format, for content of any size. Used for the master seed
// (32 bytes) and the server registry (variable size): the list of servers
// one connects to reveals affiliations, so it does not stay in plaintext
// on disk.
[[nodiscard]] bool seal_blob(std::string_view passphrase,
                             std::span<std::uint8_t const> plaintext,
                             std::vector<std::uint8_t> &out);

[[nodiscard]] bool open_blob(std::string_view passphrase,
                             std::span<std::uint8_t const> sealed,
                             std::vector<std::uint8_t> &out);

[[nodiscard]] bool seal_identity_secret(std::string_view passphrase,
                                        ed25519_secret_key const &secret,
                                        std::vector<std::uint8_t> &out);

// Returns false both for a wrong passphrase and for a tampered file:
// poly1305 does not distinguish between the two, and that is exactly as
// it should be.
[[nodiscard]] bool open_identity_secret(std::string_view passphrase,
                                        std::span<std::uint8_t const> sealed,
                                        ed25519_secret_key &out);

} // namespace hypercom::crypto
