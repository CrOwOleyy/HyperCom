#pragma once

#include "server/admin/admin_command.hpp"
#include "server/admin/admin_context.hpp"

#include <string>

namespace hypercom::server {

// `reports [list]` / `reports clear <id>` / `reports delete-post <id>`
//
// clear and delete-post remain two distinct, composable actions: deleting a
// reported post doesn't automatically resolve the report, and resolving a
// report never touches the post -- the admin may decide there was nothing
// to do and resolve it anyway.
[[nodiscard]] std::string run_reports_command(admin_context &context,
                                              admin_command const &command);

} // namespace hypercom::server
