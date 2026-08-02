#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Messages prives chiffres de bout en bout -- le deuxieme bloc de la v1.
//
// Ces trois fonctions constituent tout ce que le serveur sait faire d'un DM :
// le ranger, le rendre a son destinataire, l'effacer quand il l'a recu. Aucune
// ne peut lire le contenu, aucune ne le pourrait meme si on le voulait : il
// n'existe nulle part sur cette machine de cle permettant de l'ouvrir.
//
// Ce qu'il voit malgre tout, et qu'il faut dire aux utilisateurs plutot que le
// taire : qui ecrit a qui, et quand.

[[nodiscard]] bool handle_dm_send_request(handler_context &context,
                                          proto::byte_reader &reader);

[[nodiscard]] bool handle_dm_fetch_request(handler_context &context,
                                           proto::byte_reader &reader);

[[nodiscard]] bool handle_dm_ack_request(handler_context &context,
                                         proto::byte_reader &reader);

} // namespace hypercom::server
