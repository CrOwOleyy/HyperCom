#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/message_type.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Aiguillage par type de message.
//
// Le type a deja ete valide par decode_frame_header : le routeur ne raisonne
// jamais sur un octet arbitraire. Un type connu mais hors contexte -- une
// reponse serveur envoyee par un client, par exemple -- est refuse ici.
//
// Renvoie false pour fermer la connexion. C'est reserve aux fautes de
// protocole ; une erreur metier part en status_error et la session continue.
[[nodiscard]] bool route_message(handler_context &context,
                                 proto::message_type type,
                                 proto::byte_reader &reader);

} // namespace hypercom::server
