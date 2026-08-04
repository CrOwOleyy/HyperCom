#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// Volet social de la colonne laterale : amis, profil consulte, top 8.
// Seule entree publique -- le decoupage interne (liste d'amis, profil
// consulte, editeur de top 8) reste prive, sur le modele de draw_dm_panel().
void draw_social_panel(cli_context &context, ui_state &state, float height);

} // namespace hypercom::client
