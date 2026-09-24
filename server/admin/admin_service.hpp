#pragma once

#include "server/admin/admin_context.hpp"
#include "server/admin/admin_listener.hpp"
#include "server/net/event_loop.hpp"
#include "server/net/unique_descriptor.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace hypercom::server {

// An admin connection: one command, one response, then close.
//
// output is buffered rather than written in one block. A session list can
// exceed what a single write accepts, and blocking the event loop to wait
// for room would suspend the whole server while an administrator reads
// their output.
struct admin_connection {
    unique_descriptor socket;
    std::string input;
    std::string output;
    bool response_ready = false;
};

struct admin_service {
    admin_listener listener;
    std::unordered_map<int, admin_connection> connections;
};

// Empty path = admin disabled. This isn't an error: a server can run
// without it, it just loses the CLI.
[[nodiscard]] bool open_admin_service(admin_service &service,
                                      std::string const &path, event_loop &loop,
                                      std::string &error_out);

void accept_admin_connections(admin_service &service, event_loop &loop);

[[nodiscard]] bool owns_admin_descriptor(admin_service const &service,
                                         int descriptor);

// close_requests collects the CLIENT session descriptors the command asks
// to close. The caller applies them afterward: closing while iterating the
// registry would invalidate the iterator.
void service_admin_connection(admin_service &service, event_loop &loop,
                              int descriptor, admin_context &context,
                              std::vector<int> &close_requests);

void close_admin_connection(admin_service &service, event_loop &loop,
                            int descriptor);

} // namespace hypercom::server
