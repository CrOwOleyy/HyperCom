#pragma once

#include "server/admin/admin_command.hpp"
#include "server/admin/admin_context.hpp"

#include <string>

namespace hypercom::server {

// `backup <path>`: a consistent snapshot of the database while the server
// keeps running.
[[nodiscard]] std::string run_backup_command(admin_context &context,
                                             admin_command const &command);

} // namespace hypercom::server
