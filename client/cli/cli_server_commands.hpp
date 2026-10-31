#pragma once

#include "client/cli/cli_options.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace hypercom::client {

// Commands that touch ONLY the local registry: they open no connection
// and therefore reveal nothing to anyone. That's why they don't go
// through cli_context, unlike all the others.

[[nodiscard]] bool is_registry_command(std::string_view command);

// Shown once, when a master seed is created. The seed cannot be turned
// back into words afterwards, so this is the user's only chance to write
// the phrase down.
void print_recovery_phrase(std::string const &phrase);

// Recreates the master seed from its 12-word phrase, on a machine that
// has lost (or never had) hypercom_master.key. Refuses to overwrite an
// existing seed. The phrase is read from HYPERCOM_RECOVERY_PHRASE or
// asked on standard input -- never an argument, which would end up in the
// shell history.
[[nodiscard]] bool run_seed_restore(cli_options const &options,
                                    std::string &error_out);

[[nodiscard]] bool run_server_add(cli_options const &options,
                                  std::vector<std::string> const &arguments,
                                  std::string &error_out);

[[nodiscard]] bool run_server_list(cli_options const &options,
                                   std::string &error_out);

// Attaches an identity that predates multi-server support to a server in
// the registry. Without it, accounts created before this change would be
// unreachable: their key was drawn at random, and no seed can reproduce
// it.
[[nodiscard]] bool run_server_import(cli_options const &options,
                                     std::vector<std::string> const &arguments,
                                     std::string &error_out);

} // namespace hypercom::client
