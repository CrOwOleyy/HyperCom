#pragma once

#include "common/protocol/protocol_limits.hpp"

#include <array>
#include <cstdint>

namespace hypercom::proto {

// Wire types, independent of libsodium: the parser must be able to
// compile and be fuzzed without linking any crypto. Conversion to
// common/crypto's types happens at the boundary.
using wire_public_key = std::array<std::uint8_t, PUBLIC_KEY_SIZE>;
using wire_signature = std::array<std::uint8_t, SIGNATURE_SIZE>;
using wire_nonce = std::array<std::uint8_t, AUTH_NONCE_SIZE>;

} // namespace hypercom::proto
