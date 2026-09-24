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
