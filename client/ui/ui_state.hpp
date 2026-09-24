#pragma once

#include "client/net/server_connection.hpp"
#include "client/ui/i18n.hpp"
#include "common/protocol/content_records.hpp"
#include "common/protocol/dm_envelope_record.hpp"
#include "common/protocol/forum_record.hpp"
#include "common/protocol/social_records.hpp"
#include "common/protocol/top8_message.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace hypercom::client {

// Two tabs for the side column: direct messages, or social (friends,
// viewed profiles, top 8). Avoids stacking all three in the same
// space.
enum class side_panel_tab {
    direct_messages,
    social,
};

// A direct message already decrypted, as it's displayed.
//
// It only exists in this structure, in memory, for the duration of
// the session. Nothing ever writes the plaintext to disk: closing the
// client erases the conversation.
struct decrypted_message {
    std::string sender_hex;
    std::string text;
    std::uint64_t received_at = 0;
    bool readable = false;
};

// All of the client's displayable state.
//
// A method-less structure, passed explicitly to the drawing
// functions. ImGui runs in immediate mode: every frame re-reads this
// state and redraws it, so there's nothing to synchronize between a
// model and a view.
struct ui_state {
    language current_lang = language::french;
    bool connected = false;
    bool registered = false;
    std::string handle;
    std::string identity_hex;
    std::string server_key_hex;
    // Kept for reconnection: it redoes a full handshake, so it needs
    // the address AND the route (Tor proxy where applicable) again.
    server_endpoint endpoint;
    std::string status_message;
    bool status_is_error = false;
    // Raised only once, by draw_auth_modal, right after account
    // creation. gui_main consumes it to start the music and the
    // welcome sequence: an ordinary login therefore triggers nothing.
    bool intro_requested = false;

    std::vector<proto::forum_record> forums;
    std::uint64_t selected_forum_id = 0;
    std::vector<proto::post_record> posts;
    std::uint64_t selected_post_id = 0;
    proto::post_record open_post;
    std::vector<proto::comment_record> comments;
    bool thread_truncated = false;

    side_panel_tab active_side_tab = side_panel_tab::direct_messages;

    std::vector<proto::friend_record> friends;
    proto::profile_record viewed_profile;
    std::array<proto::wire_public_key, proto::TOP8_SLOT_COUNT>
        viewed_top8_slots{};
    std::vector<proto::friend_record> viewed_top8_details;

    std::array<proto::wire_public_key, proto::TOP8_SLOT_COUNT> own_top8_slots{};
    std::vector<proto::friend_record> own_top8_details;

    std::vector<decrypted_message> inbox;
    std::string dm_recipient_hex;

    // Input buffers. ImGui writes directly into them, hence the
    // fixed-size arrays rather than std::string.
    char forum_name_input[64] = {};
    char forum_description_input[256] = {};
    char post_title_input[256] = {};
    char post_body_input[4096] = {};
    char comment_input[2048] = {};
    char display_name_input[64] = {};
    char bio_input[1024] = {};
    char dm_recipient_input[80] = {};
    char dm_text_input[2048] = {};
    char registration_handle_input[64] = {};
    char friend_add_input[80] = {};
};

} // namespace hypercom::client
