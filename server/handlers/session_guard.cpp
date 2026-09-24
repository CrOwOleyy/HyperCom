#include "server/handlers/session_guard.hpp"

#include "server/handlers/response_builder.hpp"

namespace hypercom::server {

bool require_registered_session(handler_context &context)
{
    session_state const &session = context.connection.session;
    if (session.phase == session_phase::authenticated && session.user_id != 0) {
        return true;
    }
    // A send failure isn't propagated: the session is rejected either way,
    // and the loop will close the connection on the next failed flush.
    static_cast<void>(send_status_error(context.connection,
                                        proto::error_code::not_authenticated));
    return false;
}

} // namespace hypercom::server
