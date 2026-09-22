#include "client/cli/cli_content_delete_commands.hpp"

#include <charconv>
#include <iostream>

#include "client/net/message_exchange.hpp"
#include "common/protocol/content_delete_message.hpp"
#include "common/protocol/status_message.hpp"

namespace hypercom::client {
namespace {

[[nodiscard]] bool parse_identifier(std::string const &text,
                                    std::uint64_t &out)
{
    auto const result =
        std::from_chars(text.data(), text.data() + text.size(), out);
    return result.ec == std::errc{};
}

} // namespace

bool run_post_delete(cli_context &context,
                     std::vector<std::string> const &arguments,
                     std::string &error_out)
{
    if (arguments.empty()) {
        error_out = "usage : post-delete <post_id>";
        return false;
    }
    proto::post_delete_request request;
    if (!parse_identifier(arguments[0], request.post_id)) {
        error_out = "identifiant invalide";
        return false;
    }
    if (!send_typed_message(context.connection,
                            proto::message_type::post_delete_request,
                            request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::status_ok_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::status_ok, response,
                               error_out)) {
        return false;
    }
    std::cout << "post retire\n";
    return true;
}

bool run_comment_delete(cli_context &context,
                        std::vector<std::string> const &arguments,
                        std::string &error_out)
{
    if (arguments.empty()) {
        error_out = "usage : comment-delete <comment_id>";
        return false;
    }
    proto::comment_delete_request request;
    if (!parse_identifier(arguments[0], request.comment_id)) {
        error_out = "identifiant invalide";
        return false;
    }
    if (!send_typed_message(context.connection,
                            proto::message_type::comment_delete_request,
                            request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::status_ok_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::status_ok, response,
                               error_out)) {
        return false;
    }
    std::cout << "commentaire retire\n";
    return true;
}

} // namespace hypercom::client
