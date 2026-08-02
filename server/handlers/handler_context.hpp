#pragma once

#include "common/util/logger.hpp"
#include "server/config/server_config.hpp"
#include "server/db/database_handle.hpp"
#include "server/net/client_connection.hpp"

namespace hypercom::server {

// Tout ce dont un handler a besoin, passe explicitement.
//
// C'est la forme que prend la regle G4 en pratique : il n'existe pas de
// « serveur courant » ni de « connexion courante » accessible de n'importe ou.
// Un handler ne peut toucher qu'a ce qu'on lui a remis, ce qui rend son
// perimetre lisible dans sa signature.
struct handler_context {
    server_config const &config;
    util::logger &logger;
    database_handle &database;
    client_connection &connection;
};

} // namespace hypercom::server
