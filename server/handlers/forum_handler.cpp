#include "server/handlers/forum_handler.hpp"

#include <algorithm>

#include "common/protocol/forum_create_message.hpp"
#include "common/protocol/forum_list_message.hpp"
#include "server/db/forum_repository.hpp"
#include "server/handlers/response_builder.hpp"
#include "server/handlers/session_guard.hpp"

namespace hypercom::server {

bool handle_forum_create_request(handler_context &context,
                                 proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::forum_create_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    if (!proto::validate_forum_name(request.name)) {
        return send_status_error(context.connection,
                                 proto::error_code::invalid_field);
    }
    forum_repository forums{context.database};
    std::int64_t created_id = 0;
    if (!forums.create_forum(context.connection.session.user_id, request.name,
                             request.description, request.theme_json,
                             created_id)) {
        return send_status_error(context.connection,
                                 proto::error_code::duplicate_entry);
    }
    proto::forum_info_response response;
    if (!forums.find_by_id(created_id, response.forum)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_message(context.connection,
                        proto::message_type::forum_info_response, response);
}

bool handle_forum_list_request(handler_context &context,
                               proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::forum_list_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    // La limite demandee est rabattue, jamais honoree telle quelle : c'est le
    // serveur qui decide combien il envoie.
    std::uint16_t const limit =
        std::min<std::uint16_t>(request.limit == 0 ? proto::DEFAULT_LIST_ITEMS
                                                   : request.limit,
                                proto::MAX_LIST_ITEMS);
    forum_repository forums{context.database};
    proto::forum_list_response response;
    if (!forums.list_forums(request.offset, limit, response.forums,
                            response.total_count)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_message(context.connection,
                        proto::message_type::forum_list_response, response);
}

} // namespace hypercom::server
