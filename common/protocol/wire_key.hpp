#pragma once

#include <array>
#include <cstdint>

#include "common/protocol/protocol_limits.hpp"

namespace hypercom::proto {

// Types de fil, independants de libsodium : le parseur doit pouvoir se
// compiler et se fuzzer sans lier de crypto. La conversion vers les types de
// common/crypto se fait a la frontiere.
using wire_public_key = std::array<std::uint8_t, PUBLIC_KEY_SIZE>;
using wire_signature = std::array<std::uint8_t, SIGNATURE_SIZE>;
using wire_nonce = std::array<std::uint8_t, AUTH_NONCE_SIZE>;

} // namespace hypercom::proto
