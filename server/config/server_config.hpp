#pragma once

#include "common/util/log_level.hpp"

#include <cstdint>
#include <string>

namespace hypercom::server {

// Everything that drives the server without recompiling.
//
// This is a plain data structure, passed explicitly to whoever needs it.
// Rule G4 forbids making it a singleton: there is no "current
// configuration" reachable from everywhere, only an object that gets
// passed around.
struct listener_config {
    bool enabled = false;
    std::string bind_address;
    std::uint16_t port = 0;
    // Address to advertise to clients, when it differs from the listening
    // one. bind_address = 0.0.0.0 means "all interfaces" and isn't
    // reachable by anyone: the server can't guess its public address, the
    // administrator declares it here. Empty = use bind_address.
    std::string advertised_host;
};

struct limits_config {
    std::uint32_t max_connections = 512;
    std::uint32_t max_connections_per_address = 8;
    std::uint32_t max_frame_size = 1024 * 1024;
    std::uint32_t handshake_timeout_seconds = 10;
    // 0 = disabled: no application-level timeout on an authenticated
    // session, per the project's choice (BRIEF.md 9). Only TCP keepalive
    // reclaims a connection whose peer has genuinely vanished.
    std::uint32_t idle_timeout_seconds = 0;
    std::uint32_t requests_per_minute_per_address = 240;
    std::uint32_t requests_per_minute_per_identity = 600;
};

// Logging policy. log_peer_addresses defaults to false and requires an
// explicit action to flip to true -- a network that claims to be
// unmonitored doesn't log IPs by accident.
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
    // Empty = no file written. Non-secret content (host+port+public key):
    // what the admin would hand a new user anyway, in a form that can be
    // copied as-is instead of retyped by hand.
    std::string connect_file_path = "run/hypercom-connect.txt";
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
