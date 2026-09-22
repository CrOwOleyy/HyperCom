#include "client/ui/ui_report_actions.hpp"

#include "client/net/message_exchange.hpp"
#include "client/ui/connection_guard.hpp"
#include "common/protocol/report_message.hpp"
#include "common/protocol/status_message.hpp"

namespace hypercom::client {
namespace {

void report(ui_state &state, bool ok, std::string const &failure)
{
    if (ok) {
        state.status_message = "signalement envoye";
        state.status_is_error = false;
        return;
    }
    state.status_message = failure;
    state.status_is_error = true;
}

} // namespace

void report_post(cli_context &context, ui_state &state, std::uint64_t post_id)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::report_post_request request;
    request.post_id = post_id;
    std::string failure;
    proto::status_ok_response response;
    bool const ok =
        send_typed_message(context.connection,
                           proto::message_type::report_post_request, request)
        && receive_typed_message(context.connection,
                                 proto::message_type::status_ok, response,
                                 failure);
    report(state, ok, failure);
}

void report_account(cli_context &context, ui_state &state,
                    proto::wire_public_key const &target_pubkey)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::report_account_request request;
    request.target_pubkey = target_pubkey;
    std::string failure;
    proto::status_ok_response response;
    bool const ok = send_typed_message(
                        context.connection,
                        proto::message_type::report_account_request, request)
                    && receive_typed_message(context.connection,
                                            proto::message_type::status_ok,
                                            response, failure);
    report(state, ok, failure);
}

} // namespace hypercom::client
