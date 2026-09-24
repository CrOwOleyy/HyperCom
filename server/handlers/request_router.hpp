#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/message_type.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Dispatch by message type.
//
// The type has already been validated by decode_frame_header: the router
// never reasons over an arbitrary byte. A type that is known but out of
// context -- a server response sent by a client, for instance -- is
// rejected here.
//
// Returns false to close the connection. That's reserved for protocol
// violations; a business error goes out as status_error and the session
// continues.
[[nodiscard]] bool route_message(handler_context &context,
                                 proto::message_type type,
                                 proto::byte_reader &reader);

} // namespace hypercom::server
