#pragma once

#include "client/cli/cli_context.hpp"

#include <string>
#include <vector>

namespace hypercom::client {

// Split off from cli_content_commands.hpp, which already exposes four
// functions against a cap of five.

[[nodiscard]] bool run_post_delete(cli_context &context,
                                   std::vector<std::string> const &arguments,
                                   std::string &error_out);

[[nodiscard]] bool run_comment_delete(cli_context &context,
                                      std::vector<std::string> const &arguments,
                                      std::string &error_out);

} // namespace hypercom::client
