#pragma once

#include <string>
#include <vector>

#include "client/cli/cli_context.hpp"

namespace hypercom::client {

// Separe de cli_content_commands.hpp, deja a quatre fonctions exposees pour un
// plafond de cinq.

[[nodiscard]] bool run_post_delete(cli_context &context,
                                   std::vector<std::string> const &arguments,
                                   std::string &error_out);

[[nodiscard]] bool run_comment_delete(cli_context &context,
                                      std::vector<std::string> const &arguments,
                                      std::string &error_out);

} // namespace hypercom::client
