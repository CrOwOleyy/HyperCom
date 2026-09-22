#pragma once

#include <cstdint>

#include "client/cli/cli_context.hpp"
#include "client/ui/ui_state.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::client {

// Signalement (BRIEF.md 13). Aucun motif saisi depuis le GUI -- juste
// signaler, vite. Le CLI (report-post/report-account) reste disponible pour
// qui veut joindre une explication.

void report_post(cli_context &context, ui_state &state, std::uint64_t post_id);

void report_account(cli_context &context, ui_state &state,
                    proto::wire_public_key const &target_pubkey);

} // namespace hypercom::client
