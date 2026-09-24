#pragma once

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"
#include "common/protocol/wire_key.hpp"

#include <cstdint>

namespace hypercom::client {

// Reporting (BRIEF.md 13). No reason entered from the GUI -- just
// report, quickly. The CLI (report-post/report-account) remains
// available for anyone who wants to attach an explanation.

void report_post(cli_context &context, ui_state &state, std::uint64_t post_id);

void report_account(cli_context &context, ui_state &state,
                    proto::wire_public_key const &target_pubkey);

} // namespace hypercom::client
