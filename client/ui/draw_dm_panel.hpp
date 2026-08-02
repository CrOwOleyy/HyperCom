#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// Colonne de droite : identite, profil, messages prives.
void draw_side_column(cli_context &context, ui_state &state, float column_width = 0.0f);

void draw_identity_panel(cli_context &context, ui_state &state, float bio_height);

void draw_dm_panel(cli_context &context, ui_state &state, float inbox_height, float input_height);

} // namespace hypercom::client
