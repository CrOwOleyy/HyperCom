#pragma once

#include <string>

#include "server/admin/admin_command.hpp"
#include "server/admin/admin_context.hpp"

namespace hypercom::server {

// `reports [list]` / `reports clear <id>` / `reports delete-post <id>`
//
// clear et delete-post restent deux actions distinctes et composables :
// effacer un post signale ne classe pas automatiquement le signalement, et
// classer un signalement ne touche jamais au post -- l'admin peut juger qu'il
// n'y avait rien a faire et classer quand meme.
[[nodiscard]] std::string run_reports_command(admin_context &context,
                                              admin_command const &command);

} // namespace hypercom::server
