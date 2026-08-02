#pragma once

#include "client/ui/intro_sequence.hpp"

namespace hypercom::client {

// L'ecran des six premieres secondes : une carte de verre centree, deux
// lignes, rien d'autre. Aucun bouton -- on ne demande rien a l'utilisateur
// pendant ce temps-la, il vient de creer son compte.
void draw_welcome_overlay(intro_state const &state, float scale);

} // namespace hypercom::client
