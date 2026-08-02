#pragma once

#include <string>

#include "server/admin/admin_command.hpp"
#include "server/admin/admin_context.hpp"

namespace hypercom::server {

// `motd show` / `motd set <texte>` / `motd clear`
//
// L'annonce prend effet immediatement : elle est relue en base a chaque
// connexion, il n'y a donc rien a recharger ni a redemarrer.
[[nodiscard]] std::string run_motd_command(admin_context &context,
                                           admin_command const &command);

} // namespace hypercom::server
