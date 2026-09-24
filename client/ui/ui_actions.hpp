#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// Network actions triggered by the interface.
//
// They are SYNCHRONOUS: one round trip per call, on the render
// thread. The client has only one connection and responses take a
// few milliseconds, so an async queue would only add shared state to
// protect. If some operation ever becomes slow, that's the one to
// offload, not the whole architecture.
//
// Each action fills in state.status_message, which is the only
// channel of information to the user: no error is ever swallowed
// silently.

void refresh_forum_list(cli_context &context, ui_state &state);

void refresh_post_list(cli_context &context, ui_state &state);

void open_thread(cli_context &context, ui_state &state, std::uint64_t post_id);

void submit_post(cli_context &context, ui_state &state);

void submit_comment(cli_context &context, ui_state &state,
                    std::uint64_t parent_comment_id);

} // namespace hypercom::client
