#pragma once

#include <string>

#include "server/admin/admin_command.hpp"
#include "server/admin/admin_context.hpp"

namespace hypercom::server {

// `ban <cle_hex>` / `unban <cle_hex>`
//
// Revoque ou restaure l'authentification, rien d'autre : voir
// user_repository::set_banned. Le contenu deja publie par le compte n'est pas
// touche -- classer un signalement de post reste une action separee.
[[nodiscard]] std::string run_ban_command(admin_context &context,
                                          admin_command const &command,
                                          bool banned);

} // namespace hypercom::server
