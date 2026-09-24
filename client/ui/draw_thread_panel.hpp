#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// Center column: the open thread and its nested comments.
void draw_thread_column(cli_context &context, ui_state &state,
                        float column_width = 0.0f);

} // namespace hypercom::client
