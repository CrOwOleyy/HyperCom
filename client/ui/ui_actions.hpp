#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// Actions reseau declenchees par l'interface.
//
// Elles sont SYNCHRONES : un aller-retour par appel, sur le fil de rendu. Le
// client n'a qu'une seule connexion et les reponses tiennent en quelques
// millisecondes, une file asynchrone n'apporterait ici qu'un etat partage a
// proteger. Si un jour une operation devient lente, c'est elle qu'il faudra
// deporter, pas l'architecture entiere.
//
// Chaque action renseigne state.status_message, qui est le seul canal
// d'information vers l'utilisateur : aucune erreur n'est avalee en silence.

void refresh_forum_list(cli_context &context, ui_state &state);

void refresh_post_list(cli_context &context, ui_state &state);

void open_thread(cli_context &context, ui_state &state,
                 std::uint64_t post_id);

void submit_post(cli_context &context, ui_state &state);

void submit_comment(cli_context &context, ui_state &state,
                    std::uint64_t parent_comment_id);

} // namespace hypercom::client
