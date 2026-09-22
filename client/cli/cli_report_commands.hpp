#pragma once

#include <string>
#include <vector>

#include "client/cli/cli_context.hpp"

namespace hypercom::client {

[[nodiscard]] bool run_report_post(cli_context &context,
                                   std::vector<std::string> const &arguments,
                                   std::string &error_out);

[[nodiscard]] bool run_report_account(
    cli_context &context, std::vector<std::string> const &arguments,
    std::string &error_out);

} // namespace hypercom::client
