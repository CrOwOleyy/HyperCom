#include "server/handlers/account_handler.hpp"

#include "common/protocol/prekey_fetch_message.hpp"
#include "common/protocol/prekey_publish_message.hpp"
#include "server/db/prekey_repository.hpp"
#include "server/handlers/response_builder.hpp"
#include "server/handlers/session_guard.hpp"

namespace hypercom::server {

bool handle_prekey_publish_request(handler_context &context,
                                   proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::prekey_publish_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    prekey_repository prekeys{context.database};
    if (!prekeys.replace_prekey(context.connection.session.user_id,
                                request.prekey, request.signature)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_status_ok(context.connection, 0);
}

bool handle_prekey_fetch_request(handler_context &context,
                                 proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::prekey_fetch_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    prekey_repository prekeys{context.database};
    proto::prekey_bundle_response bundle;
    if (!prekeys.find_bundle_by_pubkey(request.target_pubkey, bundle)) {
        return send_status_error(context.connection,
                                 proto::error_code::not_found);
    }
    return send_message(context.connection,
                        proto::message_type::prekey_bundle_response, bundle);
}

} // namespace hypercom::server
