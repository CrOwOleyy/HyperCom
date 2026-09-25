#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Receiving a report. The server records it, it doesn't judge
// anything -- no automatic action follows, it's the admin who reads the
// queue via `reports` on the local socket.
//
// report_account_request doesn't check that the target is a friend or a
// contact of the reporter: that's intentional, someone may want to report an
// account before ever having spoken to them.

[[nodiscard]] bool handle_report_post_request(handler_context &context,
                                              proto::byte_reader &reader);

[[nodiscard]] bool handle_report_account_request(handler_context &context,
                                                 proto::byte_reader &reader);

} // namespace hypercom::server
