#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// Discord-style account creation screen / modal, shown when the key
// isn't registered yet.
void draw_auth_modal(cli_context &context, ui_state &state, float scale);

} // namespace hypercom::client
