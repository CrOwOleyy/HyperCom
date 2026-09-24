#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// Social pane of the side column: friends, viewed profile, top 8.
// The only public entry point -- the internal breakdown (friend list,
// viewed profile, top 8 editor) stays private, following the pattern
// of draw_dm_panel().
void draw_social_panel(cli_context &context, ui_state &state, float height);

} // namespace hypercom::client
