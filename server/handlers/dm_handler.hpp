#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// End-to-end encrypted private messages -- the second of v1's three blocks.
//
// These three functions are everything the server knows how to do with a DM:
// store it, hand it back to its recipient, erase it once received. None of
// them can read the content, and none could even if it wanted to: there is
// no key anywhere on this machine that could open it.
//
// What it does see regardless, and what needs to be told to users rather
// than left unsaid: who writes to whom, and when.

[[nodiscard]] bool handle_dm_send_request(handler_context &context,
                                          proto::byte_reader &reader);

[[nodiscard]] bool handle_dm_fetch_request(handler_context &context,
                                           proto::byte_reader &reader);

[[nodiscard]] bool handle_dm_ack_request(handler_context &context,
                                         proto::byte_reader &reader);

} // namespace hypercom::server
