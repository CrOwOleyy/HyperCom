#pragma once

#include "common/crypto/key_types.hpp"

#include <string>

namespace hypercom::server {

// The server's static X25519 keypair, the one the client pins (NK pattern).
//
// It's generated on first startup if the file doesn't exist, with 0600
// permissions. This key is the equivalent of a server certificate:
// replacing it breaks the pinning of every client, exactly as intended --
// that's what makes a server substitution visible instead of silent.
[[nodiscard]] bool load_or_create_server_key(
    std::string const &path, crypto::x25519_public_key &public_key,
    crypto::x25519_secret_key &secret_key, std::string &error_out);

} // namespace hypercom::server
