#pragma once

#include <cstdint>

#include "common/util/logger.hpp"
#include "server/config/server_config.hpp"
#include "server/db/database_handle.hpp"
#include "server/net/connection_registry.hpp"

namespace hypercom::server {

// Ce dont dispose une commande d'administration. Passe explicitement, comme
// partout ailleurs (regle G4) : il n'existe pas d'etat serveur global auquel
// une commande pourrait aller puiser.
//
// La configuration est const : aucune commande ne la modifie. Le rechargement
// a chaud n'est pas implemente, et le type le dit plutot que de laisser croire
// le contraire.
struct admin_context {
    server_config const &config;
    util::logger &logger;
    database_handle &database;
    connection_registry &registry;
    std::uint64_t started_at = 0;
};

} // namespace hypercom::server
