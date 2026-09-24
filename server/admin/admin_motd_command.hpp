#pragma once

#include "server/admin/admin_command.hpp"
#include "server/admin/admin_context.hpp"

#include <string>

namespace hypercom::server {

// `motd show` / `motd set <text>` / `motd clear`
//
// The announcement takes effect immediately: it's read back from the
// database on every connection, so there's nothing to reload or restart.
[[nodiscard]] std::string run_motd_command(admin_context &context,
                                           admin_command const &command);

} // namespace hypercom::server
