#pragma once

#include "server/admin/admin_command.hpp"
#include "server/admin/admin_context.hpp"

#include <string>
#include <vector>

namespace hypercom::server {

// Dispatch for admin commands.
//
// Every command returns text, even on failure: the CLI never leaves the
// administrator facing silence. close_requests collects the client sessions
// to close, applied by the event loop afterward.
[[nodiscard]] std::string
execute_admin_command(admin_context &context, admin_command const &command,
                      std::vector<int> &close_requests);

} // namespace hypercom::server
