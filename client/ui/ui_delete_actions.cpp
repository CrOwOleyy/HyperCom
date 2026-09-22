#include "client/ui/ui_delete_actions.hpp"

#include "client/net/message_exchange.hpp"
#include "client/ui/connection_guard.hpp"
#include "client/ui/ui_actions.hpp"
#include "common/protocol/content_delete_message.hpp"
#include "common/protocol/status_message.hpp"

namespace hypercom::client {

void delete_post(cli_context &context, ui_state &state, std::uint64_t post_id)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::post_delete_request request;
    request.post_id = post_id;
    std::string failure;
    proto::status_ok_response response;
    if (!send_typed_message(context.connection,
                            proto::message_type::post_delete_request, request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::status_ok, response,
                                  failure)) {
        state.status_message = failure;
        state.status_is_error = true;
        return;
    }
    // Le fil affiche encore le post ouvert : le refermer evite de laisser a
    // l'ecran un contenu qui n'existe plus.
    state.selected_post_id = 0;
    state.comments.clear();
    refresh_post_list(context, state);
    state.status_message = "post retire";
    state.status_is_error = false;
}

void delete_comment(cli_context &context, ui_state &state,
                    std::uint64_t comment_id)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::comment_delete_request request;
    request.comment_id = comment_id;
    std::string failure;
    proto::status_ok_response response;
    if (!send_typed_message(context.connection,
                            proto::message_type::comment_delete_request,
                            request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::status_ok, response,
                                  failure)) {
        state.status_message = failure;
        state.status_is_error = true;
        return;
    }
    open_thread(context, state, state.selected_post_id);
    state.status_message = "commentaire retire";
    state.status_is_error = false;
}

} // namespace hypercom::client
