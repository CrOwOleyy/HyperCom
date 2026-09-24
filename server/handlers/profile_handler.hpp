#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Customizable profiles -- the MySpace side of v1.
//
// The profile is public by nature: any authenticated session can read
// anyone's. There is no visibility setting, and that's consistent with the
// rest: whatever needs to stay private goes through encrypted DMs, not
// through a flag the server would merely promise to respect.

[[nodiscard]] bool handle_profile_get_request(handler_context &context,
                                              proto::byte_reader &reader);

[[nodiscard]] bool handle_profile_set_request(handler_context &context,
                                              proto::byte_reader &reader);

} // namespace hypercom::server
