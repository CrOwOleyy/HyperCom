#include "server/handlers/session_guard.hpp"

#include "server/handlers/response_builder.hpp"

namespace hypercom::server {

bool require_registered_session(handler_context &context)
{
    session_state const &session = context.connection.session;
    if (session.phase == session_phase::authenticated
        && session.user_id != 0) {
        return true;
    }
    // L'echec d'envoi n'est pas remonte : la session est de toute facon
    // refusee, et la boucle fermera la connexion au prochain flush rate.
    static_cast<void>(send_status_error(context.connection,
                                        proto::error_code::not_authenticated));
    return false;
}

} // namespace hypercom::server
