#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

namespace hypercom::client {

// Ecran / modal de creation de compte style Discord lorsque la cle n'est pas encore enregistree.
void draw_auth_modal(cli_context &context, ui_state &state, float scale);

} // namespace hypercom::client
