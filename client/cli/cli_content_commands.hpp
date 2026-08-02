#pragma once

#include <string>
#include <vector>

#include "client/cli/cli_context.hpp"

namespace hypercom::client {

[[nodiscard]] bool run_post_create(cli_context &context,
                                   std::vector<std::string> const &arguments,
                                   std::string &error_out);

[[nodiscard]] bool run_post_list(cli_context &context,
                                 std::vector<std::string> const &arguments,
                                 std::string &error_out);

[[nodiscard]] bool run_thread_fetch(cli_context &context,
                                    std::vector<std::string> const &arguments,
                                    std::string &error_out);

[[nodiscard]] bool run_comment_create(
    cli_context &context, std::vector<std::string> const &arguments,
    std::string &error_out);

} // namespace hypercom::client
