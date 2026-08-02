#pragma once

#include <cstdint>
#include <string>

#include "common/util/log_level.hpp"

namespace hypercom::server {

// Tout ce qui pilote le serveur sans recompiler.
//
// C'est une structure de donnees pure, passee explicitement a qui en a besoin.
// La regle G4 interdit d'en faire un singleton : il n'existe pas de
// "configuration courante" accessible de partout, il n'existe qu'un objet
// qu'on se transmet.
struct listener_config {
    bool enabled = false;
    std::string bind_address;
    std::uint16_t port = 0;
};

struct limits_config {
    std::uint32_t max_connections = 512;
    std::uint32_t max_connections_per_address = 8;
    std::uint32_t max_frame_size = 1024 * 1024;
    std::uint32_t handshake_timeout_seconds = 10;
    std::uint32_t idle_timeout_seconds = 300;
    std::uint32_t requests_per_minute_per_address = 240;
    std::uint32_t requests_per_minute_per_identity = 600;
};

// Politique de journalisation. log_peer_addresses est a false par defaut et
// demande une action explicite pour passer a true -- un reseau qui se dit non
// surveille ne journalise pas des IP par accident.
struct logging_config {
    util::log_level level = util::log_level::info;
    bool log_peer_addresses = false;
    std::uint32_t retention_days = 7;
    std::string file_path;
};

struct paths_config {
    std::string database_path = "hypercom.db";
    std::string migrations_directory = "db/migrations";
    std::string server_key_path = "keys/server_static.key";
    std::string admin_socket_path = "run/hypercom-admin.sock";
};

struct server_config {
    listener_config clearnet;
    listener_config onion;
    limits_config limits;
    logging_config logging;
    paths_config paths;
    bool registration_open = true;
};

} // namespace hypercom::server
