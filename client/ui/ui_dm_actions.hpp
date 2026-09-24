#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// To run once per identity: without a published prekey, no one can
// open an encrypted conversation with you.
void publish_own_prekey(cli_context &context, ui_state &state);

// Checks the inbox, decrypts LOCALLY, then acknowledges -- which
// removes the envelopes from the server. An unreadable message is
// never acknowledged: it stays available for diagnosis instead of
// being silently lost.
void refresh_inbox(cli_context &context, ui_state &state);

void submit_direct_message(cli_context &context, ui_state &state);

} // namespace hypercom::client
