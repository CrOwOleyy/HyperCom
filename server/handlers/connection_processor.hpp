#pragma once

#include "server/handlers/handler_context.hpp"
#include "server/net/rate_limiter.hpp"

namespace hypercom::server {

// Traite tout ce qui est arrive sur une connexion : handshake Noise tant qu'il
// n'est pas termine, puis trames applicatives dechiffrees.
//
// Renvoie false quand la connexion doit etre fermee. Aucune tentative de
// recuperation n'est faite sur un flux invalide : un flux desynchronise ne se
// rattrape pas, et deviner ou reprendre est le genre de code qui finit
// exploite.
[[nodiscard]] bool process_connection_input(handler_context &context,
                                            rate_limiter const &limiter);

} // namespace hypercom::server
