#include "common/crypto/x25519_exchange.hpp"

#include <sodium.h>

namespace hypercom::crypto {

bool generate_x25519_keypair(x25519_public_key &public_key,
                             x25519_secret_key &secret_key)
{
    return crypto_box_keypair(public_key.data(), secret_key.data()) == 0;
}

bool compute_public_from_secret(x25519_secret_key const &secret_key,
                                x25519_public_key &out)
{
    return crypto_scalarmult_base(out.data(), secret_key.data()) == 0;
}

bool compute_shared_secret(x25519_secret_key const &secret_key,
                           x25519_public_key const &peer_public_key,
                           symmetric_key &out)
{
    // crypto_scalarmult rend -1 lorsque la sortie serait entierement nulle,
    // c'est-a-dire lorsque le pair a fourni un point d'ordre faible. Propager
    // cet echec est essentiel : accepter un secret nul reviendrait a accepter
    // une session que n'importe qui peut reproduire.
    return crypto_scalarmult(out.data(), secret_key.data(),
                             peer_public_key.data())
        == 0;
}

bool convert_identity_public_to_x25519(
    ed25519_public_key const &identity_public, x25519_public_key &out)
{
    return crypto_sign_ed25519_pk_to_curve25519(out.data(),
                                                identity_public.data())
        == 0;
}

bool convert_identity_secret_to_x25519(
    ed25519_secret_key const &identity_secret, x25519_secret_key &out)
{
    return crypto_sign_ed25519_sk_to_curve25519(out.data(),
                                                identity_secret.data())
        == 0;
}

} // namespace hypercom::crypto
