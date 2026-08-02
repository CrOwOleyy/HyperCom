#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Ouverture de session et authentification par defi-reponse.
//
// Deroule complet, au-dessus du canal Noise deja etabli :
//   1. hello_request     le client annonce sa cle publique
//   2. auth_challenge    le serveur renvoie un nonce de 32 octets
//   3. auth_response     le client signe le nonce avec sa cle privee
//   4. auth_accepted     le serveur verifie la signature
//
// Aucun mot de passe n'est transmis, stocke, ni meme existant cote serveur.
// Le retour false ferme la connexion, il ne signale pas une erreur metier --
// celles-ci partent en status_error et la session continue.

[[nodiscard]] bool handle_hello_request(handler_context &context,
                                        proto::byte_reader &reader);

[[nodiscard]] bool handle_auth_response(handler_context &context,
                                        proto::byte_reader &reader);

[[nodiscard]] bool handle_register_request(handler_context &context,
                                           proto::byte_reader &reader);

[[nodiscard]] bool handle_ping_request(handler_context &context,
                                       proto::byte_reader &reader);

} // namespace hypercom::server
