#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// Colonne de gauche : forums, puis fil du forum selectionne.
//
// Le decoupage en petites fonctions draw_* n'est pas cosmetique : ImGui est en
// mode immediat, donc une fonction de dessin gonfle tres vite et franchit les
// soixante lignes de la regle F4 des le premier ecran un peu riche.

void draw_forum_column(cli_context &context, ui_state &state, float column_width = 330.0f);

void draw_forum_list(cli_context &context, ui_state &state, float list_height);

void draw_post_list(cli_context &context, ui_state &state, float list_height);

void draw_post_composer(cli_context &context, ui_state &state, float input_height);

} // namespace hypercom::client
