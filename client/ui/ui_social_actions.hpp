#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::client {

// Network actions for the social pane. Same discipline as
// ui_actions.hpp: synchronous, one round trip per call, no error
// swallowed silently.

void refresh_friend_list(cli_context &context, ui_state &state);

// Reads state.friend_add_input, adds the key as a friend (accepted
// status -- same behavior as the CLI, no request flow to confirm),
// then refreshes the list.
void add_friend(cli_context &context, ui_state &state);

// Loads both the profile AND the top 8 of the target into
// state.viewed_profile / viewed_top8_*. The two go together: viewing
// someone means seeing both at once.
void view_profile(cli_context &context, ui_state &state,
                  proto::wire_public_key const &target_pubkey);

void refresh_own_top8(cli_context &context, ui_state &state);

void submit_own_top8(cli_context &context, ui_state &state);

} // namespace hypercom::client
