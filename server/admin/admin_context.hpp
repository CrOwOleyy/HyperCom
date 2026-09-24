#pragma once

#include "common/util/logger.hpp"
#include "server/config/server_config.hpp"
#include "server/db/database_handle.hpp"
#include "server/net/connection_registry.hpp"

#include <cstdint>

namespace hypercom::server {

// What an admin command has available. Passed explicitly, as everywhere else
// (rule G4): there is no global server state a command could reach into.
//
// The configuration is const: no command modifies it. Hot reload isn't
// implemented, and the type says so rather than suggesting otherwise.
struct admin_context {
    server_config const &config;
    util::logger &logger;
    database_handle &database;
    connection_registry &registry;
    std::uint64_t started_at = 0;
};

} // namespace hypercom::server
