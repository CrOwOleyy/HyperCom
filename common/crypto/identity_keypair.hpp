#pragma once

#include "common/crypto/key_types.hpp"

#include <cstdint>
#include <span>

namespace hypercom::crypto {

// A user's identity IS this key pair. There is no email, no phone number,
// and no server-side password.
//
// Accepted trade-off in v1: lose the key, lose the account. No recovery
// procedure exists, and that is exactly what guarantees no administrator
// can impersonate one. A 12-word recovery phrase is planned for v2;
// derive_from_seed is already in place to support it without changing the
// storage format.
class identity_keypair {
public:
    [[nodiscard]] static bool generate_random(identity_keypair &out);

    [[nodiscard]] static bool derive_from_seed(ed25519_seed const &seed,
                                               identity_keypair &out);

    [[nodiscard]] bool sign_message(std::span<std::uint8_t const> message,
                                    ed25519_signature &out) const;

    [[nodiscard]] ed25519_public_key const &get_public_key() const;

    // Exposes the private key for the sole use of the encrypted keystore and
    // the conversion to X25519. Any other caller is a design error.
    [[nodiscard]] ed25519_secret_key const &get_secret_key() const;

private:
    ed25519_public_key public_key_{};
    ed25519_secret_key secret_key_{};
};

} // namespace hypercom::crypto
