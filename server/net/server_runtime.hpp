#pragma once

#include <string>

#include "common/crypto/key_types.hpp"
#include "common/util/logger.hpp"
#include "server/config/server_config.hpp"
#include "server/db/database_handle.hpp"
#include "server/net/connection_registry.hpp"
#include "server/net/event_loop.hpp"
#include "server/net/rate_limiter.hpp"
#include "server/net/tcp_listener.hpp"

namespace hypercom::server {

// Assemblage du serveur : listeners, boucle epoll, registre des connexions.
//
// L'arret propre passe par signalfd plutot que par un drapeau atomique global.
// La raison est simple : la regle G4 interdit toute globale mutable, et
// un gestionnaire de signal classique en exigerait une. signalfd transforme le
// signal en descripteur, donc en simple evenement de la boucle -- plus de
// globale, et plus de code asynchrone reentrant a auditer.
class server_runtime {
public:
    server_runtime(server_config const &config, util::logger &logger,
                   database_handle &database,
                   crypto::x25519_public_key const &static_public,
                   crypto::x25519_secret_key const &static_secret);

    [[nodiscard]] bool start_listeners(std::string &error_out);

    [[nodiscard]] bool run_until_stopped(std::string &error_out);

private:
    void accept_pending_connections(tcp_listener const &listener);

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
    rate_limiter limiter_;
    unique_descriptor signal_descriptor_;
};

} // namespace hypercom::server
