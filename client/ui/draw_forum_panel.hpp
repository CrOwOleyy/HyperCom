#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// Left-hand column: forums, then the thread of the selected forum.
//
// Splitting this into small draw_* functions isn't cosmetic: ImGui
// runs in immediate mode, so a drawing function swells very quickly
// and blows past the sixty lines of rule F4 as soon as a screen gets
// even a little rich.

void draw_forum_column(cli_context &context, ui_state &state,
                       float column_width = 330.0f);

void draw_forum_list(cli_context &context, ui_state &state, float list_height);

void draw_post_list(cli_context &context, ui_state &state, float list_height);

void draw_post_composer(cli_context &context, ui_state &state,
                        float input_height);

} // namespace hypercom::client
