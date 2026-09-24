#pragma once

#include "server/admin/admin_command.hpp"
#include "server/admin/admin_context.hpp"

#include <string>
#include <vector>

namespace hypercom::server {

// `sessions` / `sessions close <descriptor>`
//
// close_requests collects the descriptors to close. The command doesn't
// close anything itself: destroying a connection while iterating the
// registry would invalidate the iterator. It's the event loop that applies
// the closure, once the response is built.
//
// The list shows neither username nor address: just anonymous connections,
// their state and duration. See the comment on list_sessions() for why --
// this isn't an oversight, it's the command's deliberate boundary.
[[nodiscard]] std::string
run_sessions_command(admin_context &context, admin_command const &command,
                     std::vector<int> &close_requests);

} // namespace hypercom::server
