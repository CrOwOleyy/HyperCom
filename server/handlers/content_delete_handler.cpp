#include "server/handlers/content_delete_handler.hpp"

#include "common/protocol/content_delete_message.hpp"
#include "server/db/comment_repository.hpp"
#include "server/db/post_repository.hpp"
#include "server/handlers/response_builder.hpp"
#include "server/handlers/session_guard.hpp"

namespace hypercom::server {
namespace {

// secure_delete cleans up the main file, but the WAL keeps the image of the
// pages from before the change: the removed text would stay readable there
// until the next automatic checkpoint. On a project that promises deletion
// really means deletion, "it'll eventually disappear" isn't good enough.
//
// The cost is acceptable because a deletion is rare, unlike a read or a
// publish.
void checkpoint_after_deletion(handler_context &context)
{
    std::string error;
    if (!context.database.execute_script("PRAGMA wal_checkpoint(TRUNCATE);",
                                         error)) {
        // The content is already erased from the database: a checkpoint that
        // fails only delays the WAL cleanup, it doesn't bring anything back.
        context.logger.write_entry(
            util::log_level::warning,
            "point de controle WAL apres suppression : " + error);
    }
}

} // namespace

bool handle_post_delete_request(handler_context &context,
                                proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::post_delete_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    post_repository posts{context.database};
    // One single code for "not found", "already deleted" and "not theirs":
    // all three are already inferable from the public list of posts, but
    // distinguishing them here would multiply the code paths for no gain.
    if (!posts.delete_own_post(static_cast<std::int64_t>(request.post_id),
                               context.connection.session.user_id)) {
        return send_status_error(context.connection,
                                 proto::error_code::permission_denied);
    }
    checkpoint_after_deletion(context);
    return send_status_ok(context.connection, request.post_id);
}

bool handle_comment_delete_request(handler_context &context,
                                   proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::comment_delete_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    comment_repository comments{context.database};
    if (!comments.delete_own_comment(
            static_cast<std::int64_t>(request.comment_id),
            context.connection.session.user_id)) {
        return send_status_error(context.connection,
                                 proto::error_code::permission_denied);
    }
    checkpoint_after_deletion(context);
    return send_status_ok(context.connection, request.comment_id);
}

} // namespace hypercom::server
