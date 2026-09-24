#pragma once

#include "server/admin/admin_context.hpp"

#include <string>

namespace hypercom::server {

// `stats`: server state and database volume metrics.
//
// Nothing identifying here, only aggregates. An administrator must be able
// to monitor the service's health without learning anything about who uses
// it.
[[nodiscard]] std::string run_stats_command(admin_context &context);

} // namespace hypercom::server
