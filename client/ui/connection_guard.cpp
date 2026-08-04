#include "client/ui/connection_guard.hpp"

namespace hypercom::client {

bool ensure_connected(cli_context &context, ui_state &state)
{
    if (context.connection.is_open()) {
        state.connected = true;
        return true;
    }
    std::string error;
    bool const reconnected =
        context.connection.open_session(state.server_host, state.server_port,
                                        error)
        && context.session.authenticate(error);
    state.connected = reconnected;
    if (!reconnected) {
        state.status_message = "connexion perdue, reconnexion impossible : "
                               + error;
        state.status_is_error = true;
        return false;
    }
    state.status_message = "reconnecte au serveur";
    state.status_is_error = false;
    return true;
}

} // namespace hypercom::client
