#pragma once

#include "common/crypto/key_types.hpp"

namespace hypercom::crypto {

// Diffie-Hellman exchange over Curve25519, plus the Ed25519 -> X25519
// conversions that let the identity key be reused for key agreement.

[[nodiscard]] bool generate_x25519_keypair(x25519_public_key &public_key,
                                           x25519_secret_key &secret_key);

// The public key can always be recomputed from the private one. Nothing
// holding it therefore needs to store it: a key file keeps only the
// secret half, and the two halves can never fall out of sync.
[[nodiscard]] bool
compute_public_from_secret(x25519_secret_key const &secret_key,
                           x25519_public_key &out);

// Fails if the result is entirely zero, which signals a low-order point
// sent by the peer. Never use a zero shared secret: it is identical for
// everyone.
[[nodiscard]] bool
compute_shared_secret(x25519_secret_key const &secret_key,
                      x25519_public_key const &peer_public_key,
                      symmetric_key &out);

[[nodiscard]] bool
convert_identity_public_to_x25519(ed25519_public_key const &identity_public,
                                  x25519_public_key &out);

[[nodiscard]] bool
convert_identity_secret_to_x25519(ed25519_secret_key const &identity_secret,
                                  x25519_secret_key &out);

} // namespace hypercom::crypto
