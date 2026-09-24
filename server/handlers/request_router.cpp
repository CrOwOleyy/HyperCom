#include "server/handlers/request_router.hpp"

#include "server/handlers/account_handler.hpp"
#include "server/handlers/content_delete_handler.hpp"
#include "server/handlers/content_handler.hpp"
#include "server/handlers/dm_handler.hpp"
#include "server/handlers/forum_handler.hpp"
#include "server/handlers/profile_handler.hpp"
#include "server/handlers/report_handler.hpp"
#include "server/handlers/response_builder.hpp"
#include "server/handlers/session_handler.hpp"
#include "server/handlers/social_handler.hpp"

namespace hypercom::server {
namespace {

using proto::message_type;

// One router per family: rule F4 caps a function at sixty lines, and a
// switch over the whole set of types would blow well past that.
// handled indicates whether the family recognized the type.

[[nodiscard]] bool route_session_family(handler_context &context,
                                        message_type type,
                                        proto::byte_reader &reader,
                                        bool &handled)
{
    handled = true;
    switch (type) {
        case message_type::hello_request:
            return handle_hello_request(context, reader);
        case message_type::auth_response:
            return handle_auth_response(context, reader);
        case message_type::register_request:
            return handle_register_request(context, reader);
        case message_type::ping_request:
            return handle_ping_request(context, reader);
        default:
            handled = false;
            return true;
    }
}

[[nodiscard]] bool route_content_family(handler_context &context,
                                        message_type type,
                                        proto::byte_reader &reader,
                                        bool &handled)
{
    handled = true;
    switch (type) {
        case message_type::forum_create_request:
            return handle_forum_create_request(context, reader);
        case message_type::forum_list_request:
            return handle_forum_list_request(context, reader);
        case message_type::post_create_request:
            return handle_post_create_request(context, reader);
        case message_type::post_list_request:
            return handle_post_list_request(context, reader);
        case message_type::thread_fetch_request:
            return handle_thread_fetch_request(context, reader);
        case message_type::comment_create_request:
            return handle_comment_create_request(context, reader);
        case message_type::post_delete_request:
            return handle_post_delete_request(context, reader);
        case message_type::comment_delete_request:
            return handle_comment_delete_request(context, reader);
        default:
            handled = false;
            return true;
    }
}

[[nodiscard]] bool route_social_family(handler_context &context,
                                       message_type type,
                                       proto::byte_reader &reader,
                                       bool &handled)
{
    handled = true;
    switch (type) {
        case message_type::prekey_publish_request:
            return handle_prekey_publish_request(context, reader);
        case message_type::prekey_fetch_request:
            return handle_prekey_fetch_request(context, reader);
        case message_type::profile_get_request:
            return handle_profile_get_request(context, reader);
        case message_type::profile_set_request:
            return handle_profile_set_request(context, reader);
        case message_type::friend_add_request:
            return handle_friend_add_request(context, reader);
        case message_type::friend_list_request:
            return handle_friend_list_request(context, reader);
        case message_type::top8_set_request:
            return handle_top8_set_request(context, reader);
        case message_type::top8_get_request:
            return handle_top8_get_request(context, reader);
        default:
            handled = false;
            return true;
    }
}

[[nodiscard]] bool route_dm_family(handler_context &context, message_type type,
                                   proto::byte_reader &reader, bool &handled)
{
    handled = true;
    switch (type) {
        case message_type::dm_send_request:
            return handle_dm_send_request(context, reader);
        case message_type::dm_fetch_request:
            return handle_dm_fetch_request(context, reader);
        case message_type::dm_ack_request:
            return handle_dm_ack_request(context, reader);
        default:
            handled = false;
            return true;
    }
}

[[nodiscard]] bool route_report_family(handler_context &context,
                                       message_type type,
                                       proto::byte_reader &reader,
                                       bool &handled)
{
    handled = true;
    switch (type) {
        case message_type::report_post_request:
            return handle_report_post_request(context, reader);
        case message_type::report_account_request:
            return handle_report_account_request(context, reader);
        default:
            handled = false;
            return true;
    }
}

} // namespace

bool route_message(handler_context &context, message_type type,
                   proto::byte_reader &reader)
{
    bool handled = false;
    if (!route_session_family(context, type, reader, handled)) {
        return false;
    }
    if (handled) {
        return true;
    }
    if (!route_content_family(context, type, reader, handled)) {
        return false;
    }
    if (handled) {
        return true;
    }
    if (!route_social_family(context, type, reader, handled)) {
        return false;
    }
    if (handled) {
        return true;
    }
    if (!route_dm_family(context, type, reader, handled)) {
        return false;
    }
    if (handled) {
        return true;
    }
    if (!route_report_family(context, type, reader, handled)) {
        return false;
    }
    if (handled) {
        return true;
    }
    // A type the protocol knows but that a client has no reason to send: a
    // server response, or a blob message reserved for v2.
    return send_status_error(context.connection,
                             proto::error_code::not_implemented);
}

} // namespace hypercom::server
