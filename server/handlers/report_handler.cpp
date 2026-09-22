#include "server/handlers/report_handler.hpp"

#include "common/protocol/report_message.hpp"
#include "server/db/post_repository.hpp"
#include "server/db/report_repository.hpp"
#include "server/db/user_repository.hpp"
#include "server/handlers/response_builder.hpp"
#include "server/handlers/session_guard.hpp"

namespace hypercom::server {

bool handle_report_post_request(handler_context &context,
                                proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::report_post_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    post_repository posts{context.database};
    proto::post_record target;
    if (!posts.find_by_id(static_cast<std::int64_t>(request.post_id),
                          target)) {
        return send_status_error(context.connection,
                                 proto::error_code::not_found);
    }
    report_repository reports{context.database};
    if (!reports.record_post_report(static_cast<std::int64_t>(request.post_id),
                                    context.connection.session.user_id,
                                    request.reason)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_status_ok(context.connection, request.post_id);
}

bool handle_report_account_request(handler_context &context,
                                   proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::report_account_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    user_repository users{context.database};
    user_row target;
    if (!users.find_by_pubkey(request.target_pubkey, target)) {
        return send_status_error(context.connection,
                                 proto::error_code::not_found);
    }
    report_repository reports{context.database};
    if (!reports.record_account_report(request.target_pubkey,
                                       context.connection.session.user_id,
                                       request.reason)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_status_ok(context.connection, 0);
}

} // namespace hypercom::server
