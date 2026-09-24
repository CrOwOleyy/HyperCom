#pragma once

#include "common/protocol/wire_key.hpp"

#include <cstdint>
#include <string>

namespace hypercom::server {

enum class session_phase {
    awaiting_handshake,
    awaiting_hello,
    awaiting_auth,
    authenticated,
};

// Application state of a connection.
//
// peer_address is kept in memory for per-address counting, and never
// reaches a log without going through logger::redact_peer_address. It is
// never written to disk anywhere.
//
// The rate counters are NOT here: a limit that resets to zero on every
// connection doesn't limit anything. They live in rate_tracker, shared
// across connections.
struct session_state {
    session_phase phase = session_phase::awaiting_handshake;
    proto::wire_public_key announced_pubkey{};
    proto::wire_nonce challenge_nonce{};
    std::int64_t user_id = 0;
    std::string handle;
    std::string peer_address;
    std::uint64_t connected_at = 0;
    std::uint64_t last_activity_at = 0;
};

} // namespace hypercom::server
