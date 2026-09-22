#pragma once

#include <cstdint>

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// Retrait de son propre contenu. Fichier separe de ui_actions.hpp, deja a cinq
// fonctions exposees pour un plafond de cinq.
//
// Le serveur refuse de toute facon ce qui n'appartient pas a la session : ces
// fonctions ne sont pas ce qui protege le contenu d'autrui, elles evitent
// seulement de proposer un bouton voue a l'echec.

void delete_post(cli_context &context, ui_state &state,
                 std::uint64_t post_id);

void delete_comment(cli_context &context, ui_state &state,
                    std::uint64_t comment_id);

} // namespace hypercom::client
