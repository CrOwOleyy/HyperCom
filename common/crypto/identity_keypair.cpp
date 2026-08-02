#include "common/crypto/identity_keypair.hpp"

#include <sodium.h>

namespace hypercom::crypto {

bool identity_keypair::generate_random(identity_keypair &out)
{
    return crypto_sign_keypair(out.public_key_.data(), out.secret_key_.data())
        == 0;
}

bool identity_keypair::derive_from_seed(ed25519_seed const &seed,
                                        identity_keypair &out)
{
    return crypto_sign_seed_keypair(out.public_key_.data(),
                                    out.secret_key_.data(), seed.data())
        == 0;
}

bool identity_keypair::sign_message(std::span<std::uint8_t const> message,
                                    ed25519_signature &out) const
{
    return crypto_sign_detached(out.data(), nullptr, message.data(),
                                message.size(), secret_key_.data())
        == 0;
}

ed25519_public_key const &identity_keypair::get_public_key() const
{
    return public_key_;
}

ed25519_secret_key const &identity_keypair::get_secret_key() const
{
    return secret_key_;
}

} // namespace hypercom::crypto
