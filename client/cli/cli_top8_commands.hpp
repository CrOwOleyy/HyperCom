#pragma once

#include <string>
#include <vector>

#include "client/cli/cli_context.hpp"

namespace hypercom::client {

// Separe de cli_social_commands.hpp : celui-ci est deja a cinq fonctions
// publiques, la limite de la regle O3.

[[nodiscard]] bool run_top8_set(cli_context &context,
                                std::vector<std::string> const &arguments,
                                std::string &error_out);

[[nodiscard]] bool run_top8_get(cli_context &context,
                                std::vector<std::string> const &arguments,
                                std::string &error_out);

} // namespace hypercom::client
