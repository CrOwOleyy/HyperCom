#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// A lancer une fois par identite : sans prekey publiee, personne ne peut
// ouvrir de conversation chiffree avec vous.
void publish_own_prekey(cli_context &context, ui_state &state);

// Releve la boite, dechiffre EN LOCAL, puis acquitte -- ce qui supprime les
// enveloppes du serveur. Un message illisible n'est jamais acquitte : il reste
// disponible pour un diagnostic plutot que d'etre perdu silencieusement.
void refresh_inbox(cli_context &context, ui_state &state);

void submit_direct_message(cli_context &context, ui_state &state);

} // namespace hypercom::client
