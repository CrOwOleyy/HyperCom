#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Removing one's own content.
//
// File kept separate from content_handler: that one is already at four
// exposed functions, and rule O3 caps it at five.
//
// There is deliberately no protocol-side equivalent: no network message
// allows deleting someone else's content, ownership is checked in SQL, not
// through a permission a client could exercise. That's the door the project
// exists to keep closed to the network.
//
// The only exception lives elsewhere: the `reports delete-post` admin
// command, on the local Unix socket -- never
// reachable from the network, reserved for responding to a legal report.
// post_repository::admin_delete_post() is only ever called from there, never
// from a handler that reads a client message.

[[nodiscard]] bool handle_post_delete_request(handler_context &context,
                                              proto::byte_reader &reader);

[[nodiscard]] bool handle_comment_delete_request(handler_context &context,
                                                 proto::byte_reader &reader);

} // namespace hypercom::server
