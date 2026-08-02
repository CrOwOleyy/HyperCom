#pragma once

#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Verrou d'entree de tous les handlers metier.
//
// Deux etats distincts sont refuses ici : la session non authentifiee, et la
// session authentifiee dont la cle n'a pas encore de compte (user_id == 0).
// Le second cas existe parce que la signature du defi prouve la possession
// d'une cle bien avant qu'un pseudo n'ait ete choisi.
//
// Emet elle-meme l'erreur protocolaire, pour qu'aucun handler n'ait a se
// souvenir de le faire.
[[nodiscard]] bool require_registered_session(handler_context &context);

} // namespace hypercom::server
