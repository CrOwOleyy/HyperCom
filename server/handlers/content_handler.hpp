#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Posts et commentaires imbriques -- le premier des trois blocs de la v1.

[[nodiscard]] bool handle_post_create_request(handler_context &context,
                                              proto::byte_reader &reader);

[[nodiscard]] bool handle_post_list_request(handler_context &context,
                                            proto::byte_reader &reader);

[[nodiscard]] bool handle_thread_fetch_request(handler_context &context,
                                               proto::byte_reader &reader);

[[nodiscard]] bool handle_comment_create_request(handler_context &context,
                                                 proto::byte_reader &reader);

} // namespace hypercom::server
