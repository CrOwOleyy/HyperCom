#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// Garde d'entree pour toute action reseau declenchee par un bouton.
//
// Si la connexion est encore ouverte, ne fait rien d'autre que confirmer
// state.connected. Si elle est tombee, tente une reconnexion complete :
// handshake Noise neuf puis defi-reponse neuf, exactement comme au premier
// lancement. Un echec renseigne state.status_message au lieu de laisser
// l'action echouer en silence.
[[nodiscard]] bool ensure_connected(cli_context &context, ui_state &state);

} // namespace hypercom::client
