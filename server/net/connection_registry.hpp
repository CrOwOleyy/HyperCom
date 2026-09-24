#pragma once

#include "server/config/server_config.hpp"
#include "server/net/client_connection.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace hypercom::server {

// Registry of live connections.
//
// address_counts is only used for the per-address cap. It lives in memory,
// only contains ongoing connections, and disappears with the process: this
// is not a log, and nothing here is written to disk.
struct connection_registry {
    std::uint32_t max_connections = 0;
    std::uint32_t max_connections_per_address = 0;
    std::unordered_map<int, std::unique_ptr<client_connection>> connections;
    std::unordered_map<std::string, std::uint32_t> address_counts;
};

[[nodiscard]] bool insert_connection(connection_registry &registry,
                                     std::unique_ptr<client_connection> entry);

void remove_connection(connection_registry &registry, int descriptor);

[[nodiscard]] client_connection *find_connection(connection_registry &registry,
                                                 int descriptor);

// Connections to close: a lingering handshake, or prolonged silence.
// Without this sweep, a connection that's opened and then abandoned would
// tie up a descriptor forever -- that's the cheapest possible attack.
[[nodiscard]] std::vector<int>
collect_expired_descriptors(connection_registry const &registry,
                            std::uint64_t now, limits_config const &limits);

} // namespace hypercom::server
