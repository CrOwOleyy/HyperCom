#include "client/cli/cli_content_commands.hpp"

#include <charconv>
#include <iostream>

#include "client/net/message_exchange.hpp"
#include "common/protocol/comment_create_message.hpp"
#include "common/protocol/post_create_message.hpp"
#include "common/protocol/post_list_message.hpp"
#include "common/protocol/thread_fetch_message.hpp"

namespace hypercom::client {
namespace {

[[nodiscard]] bool parse_identifier(std::string const &text,
                                    std::uint64_t &out)
{
    auto const result =
        std::from_chars(text.data(), text.data() + text.size(), out);
    return result.ec == std::errc{};
}

void print_indented_comment(proto::comment_record const &comment)
{
    for (std::uint16_t level = 0; level < comment.depth; ++level) {
        std::cout << "  ";
    }
    std::cout << "|- #" << comment.id << " <" << comment.author_handle << "> "
              << comment.body << '\n';
}

} // namespace

bool run_post_create(cli_context &context,
                     std::vector<std::string> const &arguments,
                     std::string &error_out)
{
    if (arguments.size() < 3) {
        error_out = "usage : post <forum_id> <titre> <corps>";
        return false;
    }
    proto::post_create_request request;
    if (!parse_identifier(arguments[0], request.forum_id)) {
        error_out = "identifiant de forum invalide";
        return false;
    }
    request.title = arguments[1];
    request.body = arguments[2];
    if (!send_typed_message(context.connection,
                            proto::message_type::post_create_request,
                            request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::post_info_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::post_info_response,
                               response, error_out)) {
        return false;
    }
    std::cout << "post cree : #" << response.post.id << '\n';
    return true;
}

bool run_post_list(cli_context &context,
                   std::vector<std::string> const &arguments,
                   std::string &error_out)
{
    if (arguments.empty()) {
        error_out = "usage : posts <forum_id>";
        return false;
    }
    proto::post_list_request request;
    if (!parse_identifier(arguments[0], request.forum_id)) {
        error_out = "identifiant de forum invalide";
        return false;
    }
    if (!send_typed_message(context.connection,
                            proto::message_type::post_list_request, request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::post_list_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::post_list_response,
                               response, error_out)) {
        return false;
    }
    std::cout << response.posts.size() << " post(s) sur "
              << response.total_count << " :\n";
    for (proto::post_record const &post : response.posts) {
        std::cout << "  #" << post.id << "  " << post.title << "  <"
                  << post.author_handle << ">  " << post.comment_count
                  << " commentaire(s)\n";
    }
    return true;
}

bool run_thread_fetch(cli_context &context,
                      std::vector<std::string> const &arguments,
                      std::string &error_out)
{
    if (arguments.empty()) {
        error_out = "usage : thread <post_id>";
        return false;
    }
    proto::thread_fetch_request request;
    if (!parse_identifier(arguments[0], request.post_id)) {
        error_out = "identifiant de post invalide";
        return false;
    }
    if (!send_typed_message(context.connection,
                            proto::message_type::thread_fetch_request,
                            request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::thread_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::thread_response, response,
                               error_out)) {
        return false;
    }
    std::cout << "#" << response.post.id << "  " << response.post.title
              << "  <" << response.post.author_handle << ">\n"
              << response.post.body << "\n---\n";
    for (proto::comment_record const &comment : response.comments) {
        print_indented_comment(comment);
    }
    if (response.truncated != 0) {
        std::cout << "(fil tronque)\n";
    }
    return true;
}

bool run_comment_create(cli_context &context,
                        std::vector<std::string> const &arguments,
                        std::string &error_out)
{
    if (arguments.size() < 3) {
        error_out = "usage : comment <post_id> <parent_id|0> <corps>";
        return false;
    }
    proto::comment_create_request request;
    if (!parse_identifier(arguments[0], request.post_id)
        || !parse_identifier(arguments[1], request.parent_comment_id)) {
        error_out = "identifiant invalide";
        return false;
    }
    request.body = arguments[2];
    if (!send_typed_message(context.connection,
                            proto::message_type::comment_create_request,
                            request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::comment_info_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::comment_info_response,
                               response, error_out)) {
        return false;
    }
    std::cout << "commentaire cree : #" << response.comment.id << '\n';
    return true;
}

} // namespace hypercom::client
