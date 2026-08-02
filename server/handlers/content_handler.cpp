#include "server/handlers/content_handler.hpp"

#include <algorithm>

#include "common/protocol/comment_create_message.hpp"
#include "common/protocol/post_create_message.hpp"
#include "common/protocol/post_list_message.hpp"
#include "common/protocol/thread_fetch_message.hpp"
#include "server/db/comment_repository.hpp"
#include "server/db/forum_repository.hpp"
#include "server/db/post_repository.hpp"
#include "server/handlers/response_builder.hpp"
#include "server/handlers/session_guard.hpp"

namespace hypercom::server {
namespace {

[[nodiscard]] std::uint16_t clamp_list_limit(std::uint16_t requested)
{
    std::uint16_t const effective =
        requested == 0 ? proto::DEFAULT_LIST_ITEMS : requested;
    return std::min<std::uint16_t>(effective, proto::MAX_LIST_ITEMS);
}

} // namespace

bool handle_post_create_request(handler_context &context,
                                proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::post_create_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    if (request.title.empty() || request.body.empty()) {
        return send_status_error(context.connection,
                                 proto::error_code::invalid_field);
    }
    forum_repository forums{context.database};
    proto::forum_record forum;
    if (!forums.find_by_id(static_cast<std::int64_t>(request.forum_id),
                           forum)) {
        return send_status_error(context.connection,
                                 proto::error_code::not_found);
    }
    post_repository posts{context.database};
    std::int64_t created_id = 0;
    if (!posts.create_post(static_cast<std::int64_t>(request.forum_id),
                           context.connection.session.user_id, request.title,
                           request.body, created_id)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    proto::post_info_response response;
    if (!posts.find_by_id(created_id, response.post)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_message(context.connection,
                        proto::message_type::post_info_response, response);
}

bool handle_post_list_request(handler_context &context,
                              proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::post_list_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    post_repository posts{context.database};
    proto::post_list_response response;
    if (!posts.list_by_forum(static_cast<std::int64_t>(request.forum_id),
                             request.offset, clamp_list_limit(request.limit),
                             response.posts, response.total_count)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_message(context.connection,
                        proto::message_type::post_list_response, response);
}

bool handle_thread_fetch_request(handler_context &context,
                                 proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::thread_fetch_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    post_repository posts{context.database};
    proto::thread_response response;
    if (!posts.find_by_id(static_cast<std::int64_t>(request.post_id),
                          response.post)) {
        return send_status_error(context.connection,
                                 proto::error_code::not_found);
    }
    comment_repository comments{context.database};
    bool truncated = false;
    if (!comments.list_thread(static_cast<std::int64_t>(request.post_id),
                              proto::MAX_THREAD_COMMENTS, response.comments,
                              truncated)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    response.truncated = truncated ? 1U : 0U;
    return send_message(context.connection,
                        proto::message_type::thread_response, response);
}

bool handle_comment_create_request(handler_context &context,
                                   proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::comment_create_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    if (request.body.empty()) {
        return send_status_error(context.connection,
                                 proto::error_code::invalid_field);
    }
    post_repository posts{context.database};
    proto::post_record parent_post;
    if (!posts.find_by_id(static_cast<std::int64_t>(request.post_id),
                          parent_post)) {
        return send_status_error(context.connection,
                                 proto::error_code::not_found);
    }
    comment_repository comments{context.database};
    // Un parent doit appartenir au meme post : sans ce controle, un client
    // pourrait greffer sa reponse sous le fil de quelqu'un d'autre.
    if (request.parent_comment_id != 0
        && !comments.check_parent_belongs_to_post(
            static_cast<std::int64_t>(request.parent_comment_id),
            static_cast<std::int64_t>(request.post_id))) {
        return send_status_error(context.connection,
                                 proto::error_code::invalid_field);
    }
    std::int64_t created_id = 0;
    if (!comments.create_comment(
            static_cast<std::int64_t>(request.post_id),
            static_cast<std::int64_t>(request.parent_comment_id),
            context.connection.session.user_id, request.body, created_id)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    proto::comment_info_response response;
    if (!comments.find_by_id(created_id, response.comment)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_message(context.connection,
                        proto::message_type::comment_info_response, response);
}

} // namespace hypercom::server
