#pragma once

#include "client/net/client_session.hpp"
#include "client/net/server_connection.hpp"
#include "common/crypto/identity_keypair.hpp"

namespace hypercom::client {

// What a command line client command has available. Passed explicitly,
// as everywhere else (rule G4).
struct cli_context {
    server_connection &connection;
    crypto::identity_keypair const &identity;
    client_session &session;
};

} // namespace hypercom::client
