#pragma once

#include "client/net/client_session.hpp"
#include "client/net/server_connection.hpp"
#include "common/crypto/identity_keypair.hpp"

namespace hypercom::client {

// Ce dont dispose une commande du client en ligne de commande. Passe
// explicitement, comme partout ailleurs (regle G4).
struct cli_context {
    server_connection &connection;
    crypto::identity_keypair const &identity;
    client_session &session;
};

} // namespace hypercom::client
