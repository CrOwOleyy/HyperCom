#pragma once

#include "common/crypto/identity_keypair.hpp"
#include "common/crypto/key_types.hpp"

namespace hypercom::client {

// Derives the identity specific to ONE server from the master seed.
//
// The server's static key acts as domain separation: two servers yield
// two identities with no computable link between them. Two administrators
// comparing their databases would see nothing but two arbitrary-looking
// public keys -- which is what makes the non-correlation technical rather
// than a mere promise.
//
// Derivation is deterministic: the same seed and the same server key
// always give back the same identity. This is what lets you back up only
// a single secret, no matter how many servers you've joined, and recover
// everything from that one seed.
[[nodiscard]] bool
derive_server_identity(crypto::ed25519_seed const &master_seed,
                       crypto::x25519_public_key const &server_key,
                       crypto::identity_keypair &out);

} // namespace hypercom::client
