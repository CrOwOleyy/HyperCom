#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Friends and "top 8".
//
// The relationship is declarative and one-sided: nobody has to accept, and
// the server doesn't arbitrate anything. Adding someone amounts to noting
// it in your own address book -- which the client then displays however it
// wants.

[[nodiscard]] bool handle_friend_add_request(handler_context &context,
                                             proto::byte_reader &reader);

[[nodiscard]] bool handle_friend_list_request(handler_context &context,
                                              proto::byte_reader &reader);

[[nodiscard]] bool handle_top8_set_request(handler_context &context,
                                           proto::byte_reader &reader);

[[nodiscard]] bool handle_top8_get_request(handler_context &context,
                                           proto::byte_reader &reader);

} // namespace hypercom::server
