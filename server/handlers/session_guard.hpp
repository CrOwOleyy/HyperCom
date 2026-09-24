#pragma once

#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Entry gate for every business handler.
//
// Two distinct states are rejected here: the unauthenticated session, and
// the authenticated session whose key doesn't have an account yet
// (user_id == 0). The second case exists because signing the challenge
// proves possession of a key well before a handle has been chosen.
//
// Emits the protocol error itself, so that no handler has to remember to do
// it.
[[nodiscard]] bool require_registered_session(handler_context &context);

} // namespace hypercom::server
