#pragma once

#include "common/util/logger.hpp"
#include "server/config/server_config.hpp"
#include "server/db/database_handle.hpp"
#include "server/net/client_connection.hpp"

namespace hypercom::server {

// Everything a handler needs, passed explicitly.
//
// This is what rule G4 looks like in practice: there is no "current server"
// or "current connection" reachable from just anywhere. A handler can only
// touch what it was handed, which makes its scope readable right in its
// signature.
struct handler_context {
    server_config const &config;
    util::logger &logger;
    database_handle &database;
    client_connection &connection;
};

} // namespace hypercom::server
