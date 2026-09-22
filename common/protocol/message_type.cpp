#include "common/protocol/message_type.hpp"

namespace hypercom::proto {

bool is_known_message_type(std::uint8_t raw_type)
{
    switch (static_cast<message_type>(raw_type)) {
        case message_type::hello_request:
        case message_type::hello_response:
        case message_type::auth_challenge:
        case message_type::auth_response:
        case message_type::auth_accepted:
        case message_type::ping_request:
        case message_type::ping_response:
        case message_type::motd_push:
        case message_type::status_ok:
        case message_type::status_error:
        case message_type::register_request:
        case message_type::prekey_publish_request:
        case message_type::prekey_fetch_request:
        case message_type::prekey_bundle_response:
        case message_type::forum_create_request:
        case message_type::forum_info_response:
        case message_type::forum_list_request:
        case message_type::forum_list_response:
        case message_type::post_create_request:
        case message_type::post_info_response:
        case message_type::post_list_request:
        case message_type::post_list_response:
        case message_type::thread_fetch_request:
        case message_type::thread_response:
        case message_type::comment_create_request:
        case message_type::comment_info_response:
        case message_type::post_delete_request:
        case message_type::comment_delete_request:
        case message_type::profile_get_request:
        case message_type::profile_response:
        case message_type::profile_set_request:
        case message_type::friend_add_request:
        case message_type::friend_list_request:
        case message_type::friend_list_response:
        case message_type::top8_set_request:
        case message_type::top8_get_request:
        case message_type::top8_response:
        case message_type::dm_send_request:
        case message_type::dm_fetch_request:
        case message_type::dm_list_response:
        case message_type::dm_ack_request:
        case message_type::blob_announce_request:
        case message_type::blob_locate_request:
        case message_type::blob_peers_response:
        case message_type::report_post_request:
        case message_type::report_account_request:
            return true;
    }
    return false;
}

} // namespace hypercom::proto
