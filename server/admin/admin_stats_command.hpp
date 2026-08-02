#pragma once

#include <string>

#include "server/admin/admin_context.hpp"

namespace hypercom::server {

// `stats` : etat du serveur et volumetrie de la base.
//
// Rien de nominatif ici, que des agregats. Un administrateur doit pouvoir
// surveiller la sante du service sans rien apprendre sur qui l'utilise.
[[nodiscard]] std::string run_stats_command(admin_context &context);

} // namespace hypercom::server
