#include "client/cli/cli_forum_commands.hpp"

#include <iostream>

#include "client/net/message_exchange.hpp"
#include "common/protocol/forum_create_message.hpp"
#include "common/protocol/forum_list_message.hpp"
#include "common/util/hex_codec.hpp"

namespace hypercom::client {

bool run_forum_create(cli_context &context,
                      std::vector<std::string> const &arguments,
                      std::string &error_out)
{
    if (arguments.size() < 2) {
        error_out = "usage : forum-create <nom> <description>";
        return false;
    }
    proto::forum_create_request request;
    request.name = arguments[0];
    request.description = arguments[1];
    if (!send_typed_message(context.connection,
                            proto::message_type::forum_create_request,
                            request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::forum_info_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::forum_info_response,
                               response, error_out)) {
        return false;
    }
    std::cout << "forum cree : #" << response.forum.id << " "
              << response.forum.name << '\n';
    return true;
}

bool run_forum_list(cli_context &context, std::string &error_out)
{
    proto::forum_list_request request;
    if (!send_typed_message(context.connection,
                            proto::message_type::forum_list_request,
                            request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::forum_list_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::forum_list_response,
                               response, error_out)) {
        return false;
    }
    std::cout << response.forums.size() << " forum(s) sur "
              << response.total_count << " :\n";
    for (proto::forum_record const &forum : response.forums) {
        std::cout << "  #" << forum.id << "  " << forum.name << "  ("
                  << forum.post_count << " posts, fonde par "
                  << forum.founder_handle << ")\n";
        if (!forum.description.empty()) {
            std::cout << "        " << forum.description << '\n';
        }
    }
    return true;
}

} // namespace hypercom::client
