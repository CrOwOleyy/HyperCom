#pragma once

#include <cstdint>
#include <span>

#include "common/crypto/key_types.hpp"

namespace hypercom::crypto {

// L'identite d'un utilisateur EST cette paire de cles. Il n'y a ni email, ni
// telephone, ni mot de passe cote serveur (BRIEF.md 2).
//
// Contrepartie assumee en v1 : la cle perdue, le compte est perdu. Aucune
// procedure de recuperation n'existe, et c'est ce qui garantit qu'aucun
// administrateur ne peut en usurper une. La phrase de recuperation a 12 mots
// est prevue en v2 ; derive_from_seed est deja la pour l'accueillir sans
// changer le format de stockage.
class identity_keypair {
public:
    [[nodiscard]] static bool generate_random(identity_keypair &out);

    [[nodiscard]] static bool derive_from_seed(ed25519_seed const &seed,
                                               identity_keypair &out);

    [[nodiscard]] bool sign_message(std::span<std::uint8_t const> message,
                                    ed25519_signature &out) const;

    [[nodiscard]] ed25519_public_key const &get_public_key() const;

    // Expose la cle privee pour le seul usage du keystore chiffre et de la
    // conversion vers X25519. Tout autre appelant est une erreur de conception.
    [[nodiscard]] ed25519_secret_key const &get_secret_key() const;

private:
    ed25519_public_key public_key_{};
    ed25519_secret_key secret_key_{};
};

} // namespace hypercom::crypto
