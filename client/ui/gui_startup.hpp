#pragma once

#include <string>

#include "client/cli/cli_options.hpp"
#include "common/crypto/identity_keypair.hpp"
#include "common/crypto/key_types.hpp"

namespace hypercom::client {

// Tout ce qui se passe avant qu'une fenetre existe : lire la cle du serveur,
// ouvrir le keystore, deriver l'identite.

[[nodiscard]] bool prepare_session(cli_options const &options,
                                   crypto::identity_keypair &identity,
                                   crypto::x25519_public_key &server_key,
                                   std::string &error_out);

// Sous Windows, le client est lie en sous-systeme graphique : il n'a pas de
// console, donc un message sur stderr disparait sans laisser de trace. C'est
// exactement le symptome « la fenetre noire clignote puis se ferme ». Ici on
// passe donc par une boite de dialogue, et par stderr partout ailleurs.
void report_startup_failure(std::string const &message);

} // namespace hypercom::client
