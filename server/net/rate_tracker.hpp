#pragma once

#include "server/net/rate_limiter.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace hypercom::server {

struct rate_window {
    std::uint32_t counter = 0;
    std::uint64_t window_start = 0;
};

// Rate counters shared across connections.
//
// They used to live in session_state, so one per connection. The result was
// a limit with no effect: opening a second connection reset the counter to
// zero, and an address allowed eight connections got eight times the
// announced quota.
//
// DELIBERATE TRADEOFF, to know about before touching this file: limiting by
// address requires keeping a table indexed by IP address in memory. The
// project avoids IPs everywhere else, and this is the one place where
// there's no alternative -- without this index, the per-address limit
// simply cannot exist. Three properties set it apart from a log:
//   - it only holds a counter and a window start, no trace of what was
//     done;
//   - it's purged as soon as its window expires, so it never reaches back
//     further than the last minute;
//   - it's never written to disk, and disappears with the process.
// The connection registry already maintains an address -> count table for
// the per-address cap: this doesn't add a new category of data.
struct rate_tracker {
    std::unordered_map<std::string, rate_window> by_address;
    std::unordered_map<std::int64_t, rate_window> by_identity;
};

// The three arguments of a rate check, grouped together so they don't have
// to be dragged one by one through the handlers.
struct rate_policy {
    rate_limiter const &per_address;
    rate_limiter const &per_identity;
    rate_tracker &tracker;
};

[[nodiscard]] bool allow_address_request(rate_policy &policy,
                                         std::string const &address,
                                         std::uint64_t now);

// user_id equal to 0 designates a session that isn't registered yet: only
// the per-address limit applies then.
[[nodiscard]] bool allow_identity_request(rate_policy &policy,
                                          std::int64_t user_id,
                                          std::uint64_t now);

// Without this purge, both tables would grow indefinitely -- which would be
// both a memory leak and, for the address table, a history.
void forget_expired_windows(rate_tracker &tracker, std::uint64_t now);

} // namespace hypercom::server
