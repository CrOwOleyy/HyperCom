#pragma once

#include <cstdint>
#include <span>

#include "common/crypto/key_types.hpp"

namespace hypercom::crypto {

// Verification separee de identity_keypair, et ce n'est pas un detail : le
// serveur verifie des signatures en permanence et ne detient jamais la moindre
// cle privee d'utilisateur. Lui donner acces a un type qui en contient une
// serait une invitation a l'erreur.
[[nodiscard]] bool verify_signature(ed25519_public_key const &public_key,
                                    std::span<std::uint8_t const> message,
                                    ed25519_signature const &signature);

} // namespace hypercom::crypto
