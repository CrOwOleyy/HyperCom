#include "server/handlers/profile_handler.hpp"

#include "common/protocol/profile_get_message.hpp"
#include "common/protocol/profile_set_message.hpp"
#include "server/db/profile_repository.hpp"
#include "server/handlers/response_builder.hpp"
#include "server/handlers/session_guard.hpp"

namespace hypercom::server {

bool handle_profile_get_request(handler_context &context,
                                proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::profile_get_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    profile_repository profiles{context.database};
    proto::profile_response response;
    if (!profiles.find_by_pubkey(request.target_pubkey, response.profile)) {
        return send_status_error(context.connection,
                                 proto::error_code::not_found);
    }
    return send_message(context.connection,
                        proto::message_type::profile_response, response);
}

bool handle_profile_set_request(handler_context &context,
                                proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::profile_set_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    // On ne modifie que le profil de la session : la cible n'est pas un
    // parametre, elle ne peut donc pas etre detournee.
    profile_repository profiles{context.database};
    if (!profiles.replace_profile(context.connection.session.user_id,
                                  request)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_status_ok(context.connection, 0);
}

} // namespace hypercom::server
