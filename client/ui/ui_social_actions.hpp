#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::client {

// Actions reseau du volet social. Meme discipline que ui_actions.hpp :
// synchrones, un aller-retour par appel, aucune erreur avalee en silence.

void refresh_friend_list(cli_context &context, ui_state &state);

// Lit state.friend_add_input, ajoute la cle en ami (statut accepte -- meme
// comportement que le CLI, pas de flux de demande a confirmer), puis
// rafraichit la liste.
void add_friend(cli_context &context, ui_state &state);

// Charge le profil ET le top 8 de la cible dans state.viewed_profile /
// viewed_top8_*. Les deux vont ensemble : consulter quelqu'un, c'est voir
// les deux a la fois.
void view_profile(cli_context &context, ui_state &state,
                  proto::wire_public_key const &target_pubkey);

void refresh_own_top8(cli_context &context, ui_state &state);

void submit_own_top8(cli_context &context, ui_state &state);

} // namespace hypercom::client
