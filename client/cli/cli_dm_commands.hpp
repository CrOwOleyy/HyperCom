#pragma once

#include "client/cli/cli_context.hpp"

#include <string>
#include <vector>

namespace hypercom::client {

// Run once per identity: publishes the signed prekey without which no one
// can open an encrypted conversation with you.
[[nodiscard]] bool run_prekey_publish(cli_context &context,
                                      std::string &error_out);

[[nodiscard]] bool run_dm_send(cli_context &context,
                               std::vector<std::string> const &arguments,
                               std::string &error_out);

// Fetches the mailbox, decrypts locally, then acknowledges -- which
// deletes the envelopes from the server.
[[nodiscard]] bool run_dm_fetch(cli_context &context, std::string &error_out);

} // namespace hypercom::client
