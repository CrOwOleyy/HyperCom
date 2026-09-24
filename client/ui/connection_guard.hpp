#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// Entry guard for any network action triggered by a button.
//
// If the connection is still open, this does nothing more than confirm
// state.connected. If it dropped, it attempts a full reconnection: a
// fresh Noise handshake followed by a fresh challenge-response,
// exactly as on first launch. A failure fills in
// state.status_message instead of letting the action fail silently.
[[nodiscard]] bool ensure_connected(cli_context &context, ui_state &state);

} // namespace hypercom::client
