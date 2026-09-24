#pragma once

#include "client/cli/cli_context.hpp"

#include <string>
#include <vector>

namespace hypercom::client {

// Split off from cli_social_commands.hpp: that one is already at five
// public functions, the limit set by rule O3.

[[nodiscard]] bool run_top8_set(cli_context &context,
                                std::vector<std::string> const &arguments,
                                std::string &error_out);

[[nodiscard]] bool run_top8_get(cli_context &context,
                                std::vector<std::string> const &arguments,
                                std::string &error_out);

} // namespace hypercom::client
