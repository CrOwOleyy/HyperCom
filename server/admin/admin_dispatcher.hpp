#pragma once

#include <string>
#include <vector>

#include "server/admin/admin_command.hpp"
#include "server/admin/admin_context.hpp"

namespace hypercom::server {

// Aiguillage des commandes d'administration.
//
// Toute commande rend du texte, y compris en cas d'echec : la CLI ne laisse
// jamais l'administrateur devant un silence. close_requests recueille les
// sessions client a fermer, appliquees par la boucle d'evenements apres coup.
[[nodiscard]] std::string execute_admin_command(
    admin_context &context, admin_command const &command,
    std::vector<int> &close_requests);

} // namespace hypercom::server
