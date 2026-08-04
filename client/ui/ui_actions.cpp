#include "client/ui/ui_actions.hpp"

#include "client/net/message_exchange.hpp"
#include "client/ui/connection_guard.hpp"
#include "common/protocol/comment_create_message.hpp"
#include "common/protocol/forum_list_message.hpp"
#include "common/protocol/post_create_message.hpp"
#include "common/protocol/post_list_message.hpp"
#include "common/protocol/thread_fetch_message.hpp"

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

} // namespace

void refresh_forum_list(cli_context &context, ui_state &state)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::forum_list_request request;
    std::string failure;
    proto::forum_list_response response;
    if (!send_typed_message(context.connection,
                            proto::message_type::forum_list_request, request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::forum_list_response,
                                  response, failure)) {
        report_failure(state, failure);
        return;
    }
    state.forums = std::move(response.forums);
}

void refresh_post_list(cli_context &context, ui_state &state)
{
    if (state.selected_forum_id == 0) {
        return;
    }
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::post_list_request request;
    request.forum_id = state.selected_forum_id;
    std::string failure;
    proto::post_list_response response;
    if (!send_typed_message(context.connection,
                            proto::message_type::post_list_request, request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::post_list_response,
                                  response, failure)) {
        report_failure(state, failure);
        return;
    }
    state.posts = std::move(response.posts);
    report_success(state, "fil a jour");
}

void open_thread(cli_context &context, ui_state &state,
                 std::uint64_t post_id)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::thread_fetch_request request;
    request.post_id = post_id;
    std::string failure;
    proto::thread_response response;
    if (!send_typed_message(context.connection,
                            proto::message_type::thread_fetch_request, request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::thread_response,
                                  response, failure)) {
        report_failure(state, failure);
        return;
    }
    state.selected_post_id = post_id;
    state.open_post = std::move(response.post);
    state.comments = std::move(response.comments);
    state.thread_truncated = response.truncated != 0;
    report_success(state, "fil ouvert");
}

void submit_post(cli_context &context, ui_state &state)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::post_create_request request;
    request.forum_id = state.selected_forum_id;
    request.title = state.post_title_input;
    request.body = state.post_body_input;
    if (request.title.empty() || request.body.empty()) {
        report_failure(state, "titre et corps sont requis");
        return;
    }
    std::string failure;
    proto::post_info_response response;
    if (!send_typed_message(context.connection,
                            proto::message_type::post_create_request, request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::post_info_response,
                                  response, failure)) {
        report_failure(state, failure);
        return;
    }
    state.post_title_input[0] = '\0';
    state.post_body_input[0] = '\0';
    refresh_post_list(context, state);
    report_success(state, "post publie");
}

void submit_comment(cli_context &context, ui_state &state,
                    std::uint64_t parent_comment_id)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::comment_create_request request;
    request.post_id = state.selected_post_id;
    request.parent_comment_id = parent_comment_id;
    request.body = state.comment_input;
    if (request.body.empty()) {
        report_failure(state, "commentaire vide");
        return;
    }
    std::string failure;
    proto::comment_info_response response;
    if (!send_typed_message(context.connection,
                            proto::message_type::comment_create_request,
                            request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::comment_info_response,
                                  response, failure)) {
        report_failure(state, failure);
        return;
    }
    state.comment_input[0] = '\0';
    open_thread(context, state, state.selected_post_id);
    report_success(state, "commentaire publie");
}

} // namespace hypercom::client
