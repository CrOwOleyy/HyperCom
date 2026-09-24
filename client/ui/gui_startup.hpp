#pragma once

#include "client/cli/cli_options.hpp"
#include "common/crypto/identity_keypair.hpp"
#include "common/crypto/key_types.hpp"

#include <string>

namespace hypercom::client {

// Everything that happens before a window exists: reading the
// server's key, opening the keystore, deriving the identity.

[[nodiscard]] bool prepare_session(cli_options const &options,
                                   crypto::identity_keypair &identity,
                                   crypto::x25519_public_key &server_key,
                                   std::string &error_out);

// On Windows, the client is linked as a GUI subsystem app: it has no
// console, so a message on stderr vanishes without a trace. That's
// exactly the "black window flashes then closes" symptom. So here we
// go through a dialog box, and through stderr everywhere else.
void report_startup_failure(std::string const &message);

} // namespace hypercom::client
