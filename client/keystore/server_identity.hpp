#pragma once

#include "common/crypto/identity_keypair.hpp"
#include "common/crypto/key_types.hpp"

namespace hypercom::client {

// Derive l'identite propre a UN serveur depuis la graine maitresse.
//
// La cle statique du serveur sert de separation de domaine : deux serveurs
// donnent deux identites sans lien calculable entre elles. Deux administrateurs
// qui compareraient leurs bases n'y verraient que deux cles publiques
// quelconques -- c'est ce qui rend la non-correlation technique plutot que
// declarative.
//
// La derivation est deterministe : la meme graine et la meme cle serveur
// redonnent toujours la meme identite. C'est ce qui permet de ne sauvegarder
// qu'un seul secret, quel que soit le nombre de serveurs rejoints, et de tout
// retrouver depuis cette seule graine.
[[nodiscard]] bool derive_server_identity(
    crypto::ed25519_seed const &master_seed,
    crypto::x25519_public_key const &server_key,
    crypto::identity_keypair &out);

} // namespace hypercom::client
