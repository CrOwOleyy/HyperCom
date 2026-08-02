#pragma once

#include <string>

#include "common/util/logger.hpp"
#include "server/db/database_handle.hpp"

namespace hypercom::server {

// Applique les migrations manquantes de db/migrations/NNNN_description.sql au
// demarrage. Le collaborateur fait evoluer le schema sans
// recompiler quoi que ce soit.
//
// Chaque migration s'execute dans une transaction : elle passe entierement ou
// pas du tout. Une migration a moitie appliquee laisserait une base dont
// personne ne connait l'etat.
[[nodiscard]] bool apply_pending_migrations(database_handle &database,
                                            std::string const &directory,
                                            util::logger &logger,
                                            std::string &error_out);

} // namespace hypercom::server
