#pragma once

#include <cstdint>

namespace hypercom::server {

// Coarse sliding window, one minute.
//
// The object holds NO state at all: it only carries the cap, and the
// counters are passed to it by reference. That's imposed by G4.
//
// Where those counters live is the real question, and it has already been
// gotten wrong once before: putting them in the session resets them to zero
// on every new connection, effectively removing the limit. They now live in
// rate_tracker, shared across connections -- read the tradeoff documented in
// rate_tracker.hpp before revisiting this.
class rate_limiter {
public:
    explicit rate_limiter(std::uint32_t max_events_per_minute);

    [[nodiscard]] bool register_event(std::uint64_t now, std::uint32_t &counter,
                                      std::uint64_t &window_start) const;

private:
    std::uint32_t max_events_per_minute_;
};

} // namespace hypercom::server
