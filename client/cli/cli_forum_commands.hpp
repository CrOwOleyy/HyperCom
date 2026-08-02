#pragma once

#include <string>
#include <vector>

#include "client/cli/cli_context.hpp"

namespace hypercom::client {

[[nodiscard]] bool run_forum_create(cli_context &context,
                                    std::vector<std::string> const &arguments,
                                    std::string &error_out);

[[nodiscard]] bool run_forum_list(cli_context &context,
                                  std::string &error_out);

} // namespace hypercom::client
