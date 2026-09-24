#pragma once

#include "server/admin/admin_command.hpp"
#include "server/admin/admin_context.hpp"

#include <string>

namespace hypercom::server {

// `ban <hex_key>` / `unban <hex_key>`
//
// Revokes or restores authentication, nothing else: see
// user_repository::set_banned. Content already published by the account is
// left untouched -- resolving a post report remains a separate action.
[[nodiscard]] std::string run_ban_command(admin_context &context,
                                          admin_command const &command,
                                          bool banned);

} // namespace hypercom::server
