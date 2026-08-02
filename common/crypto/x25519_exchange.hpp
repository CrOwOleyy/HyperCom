#pragma once

#include "common/crypto/key_types.hpp"

namespace hypercom::crypto {

// Echange Diffie-Hellman sur Curve25519, plus les conversions Ed25519 -> X25519
// qui permettent de reutiliser la cle d'identite pour l'accord de cles.

[[nodiscard]] bool generate_x25519_keypair(x25519_public_key &public_key,
                                           x25519_secret_key &secret_key);

// La cle publique se recalcule toujours a partir de la privee. Rien qui la
// contienne n'a donc besoin de la stocker : un fichier de cle ne garde que la
// partie secrete, et les deux moities ne peuvent pas se desynchroniser.
[[nodiscard]] bool compute_public_from_secret(
    x25519_secret_key const &secret_key, x25519_public_key &out);

// Echoue si le resultat est entierement nul, ce qui signale un point d'ordre
// faible envoye par le pair. Ne jamais utiliser un secret partage nul : il est
// identique pour tout le monde.
[[nodiscard]] bool compute_shared_secret(x25519_secret_key const &secret_key,
                                         x25519_public_key const &peer_public_key,
                                         symmetric_key &out);

[[nodiscard]] bool convert_identity_public_to_x25519(
    ed25519_public_key const &identity_public, x25519_public_key &out);

[[nodiscard]] bool convert_identity_secret_to_x25519(
    ed25519_secret_key const &identity_secret, x25519_secret_key &out);

} // namespace hypercom::crypto
