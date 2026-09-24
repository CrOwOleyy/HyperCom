#pragma once

#include "client/ui/i18n.hpp"

#include <cstddef>
#include <string>

namespace hypercom::client {

// What belongs to the application as a whole, not to any single server.
//
// Everything else lives in each slot's ui_state: forums, threads,
// friends, messages, input buffers. Sharing happens here, and only
// here.
struct app_state {
    language current_lang = language::french;
    std::size_t active_slot = 0;
    // Raised once after an account creation: the welcome flow is a
    // window sequence, not a per-server sequence.
    bool intro_requested = false;
    // Status of application-level actions (adding a server, switching).
    // Errors specific to one server stay in that slot's ui_state.
    std::string status_message;
    bool status_is_error = false;
    // Input for the invite line, in the sidebar.
    char invite_input[160] = {};
    // Session passphrase, kept in memory so the registry can be
    // rewritten when a server is added or a warning is acknowledged.
    // It never leaves this process and is never written to disk.
    std::string passphrase;
    std::string registry_path;
    std::string master_seed_path;
};

} // namespace hypercom::client
