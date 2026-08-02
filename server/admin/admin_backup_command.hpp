#pragma once

#include <string>

#include "server/admin/admin_command.hpp"
#include "server/admin/admin_context.hpp"

namespace hypercom::server {

// `backup <chemin>` : instantane coherent de la base, serveur en marche.
[[nodiscard]] std::string run_backup_command(admin_context &context,
                                             admin_command const &command);

} // namespace hypercom::server
