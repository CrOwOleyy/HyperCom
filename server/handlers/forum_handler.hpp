#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Creating and listing forums.
//
// The server only rejects a forum on the shape of its name or its
// uniqueness. It has no list of forbidden topics, no editorial validation,
// and there's no plan to add one: that's the whole point of the project.

[[nodiscard]] bool handle_forum_create_request(handler_context &context,
                                               proto::byte_reader &reader);

[[nodiscard]] bool handle_forum_list_request(handler_context &context,
                                             proto::byte_reader &reader);

} // namespace hypercom::server
