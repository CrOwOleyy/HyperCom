#pragma once

#include "common/crypto/key_types.hpp"

#include <cstdint>
#include <span>

namespace hypercom::crypto {

// Verification kept separate from identity_keypair, and that's not a
// minor detail: the server verifies signatures constantly and never holds
// any user private key. Giving it access to a type that contains one
// would be an invitation for mistakes.
[[nodiscard]] bool verify_signature(ed25519_public_key const &public_key,
                                    std::span<std::uint8_t const> message,
                                    ed25519_signature const &signature);

} // namespace hypercom::crypto
