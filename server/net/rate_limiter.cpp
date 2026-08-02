#include "server/net/rate_limiter.hpp"

namespace hypercom::server {
namespace {

constexpr std::uint64_t WINDOW_DURATION_SECONDS = 60;

} // namespace

rate_limiter::rate_limiter(std::uint32_t max_events_per_minute)
    : max_events_per_minute_{max_events_per_minute}
{
}

bool rate_limiter::register_event(std::uint64_t now, std::uint32_t &counter,
                                  std::uint64_t &window_start) const
{
    if (max_events_per_minute_ == 0) {
        return true;
    }
    if (now < window_start || now - window_start >= WINDOW_DURATION_SECONDS) {
        window_start = now;
        counter = 0;
    }
    if (counter >= max_events_per_minute_) {
        return false;
    }
    ++counter;
    return true;
}

} // namespace hypercom::server
