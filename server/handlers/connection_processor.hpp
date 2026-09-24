#pragma once

#include "server/handlers/handler_context.hpp"
#include "server/net/rate_tracker.hpp"

namespace hypercom::server {

// Processes everything that has arrived on a connection: the Noise handshake
// while it isn't finished yet, then decrypted application frames.
//
// Returns false when the connection must be closed. No attempt is made to
// recover from an invalid stream: a desynchronized stream can't be salvaged,
// and guessing where to resume is the kind of code that ends up exploited.
[[nodiscard]] bool process_connection_input(handler_context &context,
                                            rate_policy &policy);

} // namespace hypercom::server
