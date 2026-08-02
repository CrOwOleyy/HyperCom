#include "server/handlers/dm_handler.hpp"

#include <algorithm>

#include "common/protocol/dm_ack_message.hpp"
#include "common/protocol/dm_fetch_message.hpp"
#include "common/protocol/dm_send_message.hpp"
#include "server/db/dm_repository.hpp"
#include "server/db/user_repository.hpp"
#include "server/handlers/response_builder.hpp"
#include "server/handlers/session_guard.hpp"

namespace hypercom::server {

bool handle_dm_send_request(handler_context &context,
                            proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::dm_send_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    if (request.ciphertext.empty()) {
        return send_status_error(context.connection,
                                 proto::error_code::invalid_field);
    }
    user_repository users{context.database};
    user_row recipient;
    if (!users.find_by_pubkey(request.recipient_pubkey, recipient)) {
        return send_status_error(context.connection,
                                 proto::error_code::not_found);
    }
    // L'expediteur est celui de la session authentifiee, jamais un champ du
    // message : on ne peut pas usurper une provenance en la declarant.
    dm_repository envelopes{context.database};
    std::int64_t created_id = 0;
    if (!envelopes.insert_envelope(recipient.id,
                                   context.connection.session.announced_pubkey,
                                   request.ciphertext, created_id)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_status_ok(context.connection,
                          static_cast<std::uint64_t>(created_id));
}

bool handle_dm_fetch_request(handler_context &context,
                             proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::dm_fetch_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    std::uint16_t const limit = std::min<std::uint16_t>(
        request.limit == 0 ? proto::MAX_DM_BATCH_ITEMS : request.limit,
        proto::MAX_DM_BATCH_ITEMS);
    dm_repository envelopes{context.database};
    proto::dm_list_response response;
    if (!envelopes.list_for_recipient(context.connection.session.user_id,
                                      request.since_id, limit, response)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_message(context.connection,
                        proto::message_type::dm_list_response, response);
}

bool handle_dm_ack_request(handler_context &context,
                           proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::dm_ack_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    // Acquitter, c'est supprimer definitivement. Le depot filtre sur
    // recipient_id : personne ne peut effacer la boite d'un autre en devinant
    // des identifiants.
    dm_repository envelopes{context.database};
    if (!envelopes.delete_acknowledged(context.connection.session.user_id,
                                       request.envelope_ids)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_status_ok(context.connection,
                          static_cast<std::uint64_t>(
                              request.envelope_ids.size()));
}

} // namespace hypercom::server
