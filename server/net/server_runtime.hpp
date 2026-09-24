#pragma once

#include "common/crypto/key_types.hpp"
#include "common/util/logger.hpp"
#include "server/admin/admin_service.hpp"
#include "server/config/server_config.hpp"
#include "server/db/database_handle.hpp"
#include "server/net/connection_registry.hpp"
#include "server/net/event_loop.hpp"
#include "server/net/rate_tracker.hpp"
#include "server/net/tcp_listener.hpp"

#include <string>

namespace hypercom::server {

// Server assembly: listeners, epoll loop, connection registry.
//
// Clean shutdown goes through signalfd rather than a global atomic flag.
// The reason is simple: rule G4 forbids any mutable global, and a classic
// signal handler would require one. signalfd turns the signal into a
// descriptor, so into a plain loop event -- no more global, and no more
// reentrant asynchronous code to audit.
class server_runtime {
public:
    server_runtime(server_config const &config, util::logger &logger,
                   database_handle &database,
                   crypto::x25519_public_key const &static_public,
                   crypto::x25519_secret_key const &static_secret);

    [[nodiscard]] bool start_listeners(std::string &error_out);

    [[nodiscard]] bool run_until_stopped(std::string &error_out);

private:
    // Routes a descriptor to its owner: clearnet listener, onion listener,
    // admin socket, or client connection.
    void dispatch_event(int descriptor, std::uint32_t events);

    // is_clearnet identifies the listener behind the call: it's what
    // guarantees an onion connection is never logged, even if
    // log_peer_addresses is on -- see BRIEF.md 13.
    void accept_pending_connections(tcp_listener const &listener,
                                    bool is_clearnet);

    void service_connection(int descriptor, std::uint32_t events);

    void close_connection(int descriptor);

    void sweep_expired_connections();

    server_config const &config_;
    util::logger &logger_;
    database_handle &database_;
    crypto::x25519_public_key static_public_;
    crypto::x25519_secret_key static_secret_;
    tcp_listener clearnet_listener_;
    tcp_listener onion_listener_;
    event_loop loop_;
    connection_registry registry_;
    rate_limiter address_limiter_;
    rate_limiter identity_limiter_;
    rate_tracker rate_tracker_;
    admin_service admin_;
    std::uint64_t started_at_;
    unique_descriptor signal_descriptor_;
};

} // namespace hypercom::server
