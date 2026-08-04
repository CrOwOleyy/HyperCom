#include "client/ui/ui_social_actions.hpp"

#include "client/net/message_exchange.hpp"
#include "client/ui/connection_guard.hpp"
#include "common/protocol/friend_message.hpp"
#include "common/protocol/profile_get_message.hpp"
#include "common/protocol/status_message.hpp"
#include "common/protocol/top8_message.hpp"
#include "common/util/hex_codec.hpp"

namespace hypercom::client {
namespace {

void report_failure(ui_state &state, std::string const &message)
{
    state.status_message = message;
    state.status_is_error = true;
}

void report_success(ui_state &state, std::string message)
{
    state.status_message = std::move(message);
    state.status_is_error = false;
}

[[nodiscard]] bool parse_public_key(std::string const &text,
                                    proto::wire_public_key &out)
{
    std::vector<std::uint8_t> decoded;
    if (!util::decode_hex(text, decoded) || decoded.size() != out.size()) {
        return false;
    }
    std::copy(decoded.begin(), decoded.end(), out.begin());
    return true;
}

} // namespace

void refresh_friend_list(cli_context &context, ui_state &state)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    std::string failure;
    proto::friend_list_response response;
    if (!context.connection.send_frame(
            proto::message_type::friend_list_request, {})
        || !receive_typed_message(context.connection,
                                  proto::message_type::friend_list_response,
                                  response, failure)) {
        report_failure(state, failure);
        return;
    }
    state.friends = std::move(response.friends);
}

void add_friend(cli_context &context, ui_state &state)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::friend_add_request request;
    if (!parse_public_key(state.friend_add_input, request.target_pubkey)) {
        report_failure(state, "cle publique invalide");
        return;
    }
    request.status = proto::friendship_status::accepted;
    std::string failure;
    proto::status_ok_response response;
    if (!send_typed_message(context.connection,
                            proto::message_type::friend_add_request, request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::status_ok, response,
                                  failure)) {
        report_failure(state, failure);
        return;
    }
    state.friend_add_input[0] = '\0';
    refresh_friend_list(context, state);
    report_success(state, "ami ajoute");
}

void view_profile(cli_context &context, ui_state &state,
                  proto::wire_public_key const &target_pubkey)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::profile_get_request profile_request;
    profile_request.target_pubkey = target_pubkey;
    std::string failure;
    proto::profile_response profile_response;
    if (!send_typed_message(context.connection,
                            proto::message_type::profile_get_request,
                            profile_request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::profile_response,
                                  profile_response, failure)) {
        report_failure(state, failure);
        return;
    }
    state.viewed_profile = std::move(profile_response.profile);
    proto::top8_get_request top8_request;
    top8_request.target_pubkey = target_pubkey;
    proto::top8_response top8_response_data;
    if (!send_typed_message(context.connection,
                            proto::message_type::top8_get_request,
                            top8_request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::top8_response,
                                  top8_response_data, failure)) {
        report_failure(state, failure);
        return;
    }
    state.viewed_top8_slots = top8_response_data.slots;
    state.viewed_top8_details = std::move(top8_response_data.details);
    report_success(state, "profil charge");
}

void refresh_own_top8(cli_context &context, ui_state &state)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::top8_get_request request;
    request.target_pubkey = context.identity.get_public_key();
    std::string failure;
    proto::top8_response response;
    if (!send_typed_message(context.connection,
                            proto::message_type::top8_get_request, request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::top8_response,
                                  response, failure)) {
        report_failure(state, failure);
        return;
    }
    state.own_top8_slots = response.slots;
    state.own_top8_details = std::move(response.details);
}

void submit_own_top8(cli_context &context, ui_state &state)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::top8_set_request request;
    request.slots = state.own_top8_slots;
    std::string failure;
    proto::status_ok_response response;
    if (!send_typed_message(context.connection,
                            proto::message_type::top8_set_request, request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::status_ok, response,
                                  failure)) {
        report_failure(state, failure);
        return;
    }
    refresh_own_top8(context, state);
    report_success(state, "top 8 enregistre");
}

} // namespace hypercom::client
