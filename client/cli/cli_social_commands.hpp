#pragma once

#include <string>
#include <vector>

#include "client/cli/cli_context.hpp"

namespace hypercom::client {

[[nodiscard]] bool run_profile_set(cli_context &context,
                                   std::vector<std::string> const &arguments,
                                   std::string &error_out);

[[nodiscard]] bool run_profile_get(cli_context &context,
                                   std::vector<std::string> const &arguments,
                                   std::string &error_out);

[[nodiscard]] bool run_friend_add(cli_context &context,
                                  std::vector<std::string> const &arguments,
                                  std::string &error_out);

[[nodiscard]] bool run_friend_list(cli_context &context,
                                   std::string &error_out);

} // namespace hypercom::client
