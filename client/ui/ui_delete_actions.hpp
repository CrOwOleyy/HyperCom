#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"

#include <cstdint>

namespace hypercom::client {

// Removal of one's own content. A separate file from ui_actions.hpp,
// which already has five exposed functions against a cap of five.
//
// The server refuses anything that doesn't belong to the session
// anyway: these functions aren't what protects other people's
// content, they only avoid offering a button that's bound to fail.

void delete_post(cli_context &context, ui_state &state, std::uint64_t post_id);

void delete_comment(cli_context &context, ui_state &state,
                    std::uint64_t comment_id);

} // namespace hypercom::client
