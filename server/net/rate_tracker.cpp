#include "server/net/rate_tracker.hpp"

namespace hypercom::server {
namespace {

// Two windows: an entry whose window has been closed for longer than this no
// longer has any influence on the decision, it can be dropped.
constexpr std::uint64_t EXPIRY_SECONDS = 120;

} // namespace

bool allow_address_request(rate_policy &policy, std::string const &address,
                           std::uint64_t now)
{
    rate_window &window = policy.tracker.by_address[address];
    return policy.per_address.register_event(now, window.counter,
                                             window.window_start);
}

bool allow_identity_request(rate_policy &policy, std::int64_t user_id,
                            std::uint64_t now)
{
    if (user_id == 0) {
        return true;
    }
    rate_window &window = policy.tracker.by_identity[user_id];
    return policy.per_identity.register_event(now, window.counter,
                                              window.window_start);
}

void forget_expired_windows(rate_tracker &tracker, std::uint64_t now)
{
    for (auto entry = tracker.by_address.begin();
         entry != tracker.by_address.end();) {
        bool const expired = now >= entry->second.window_start &&
                             now - entry->second.window_start >= EXPIRY_SECONDS;
        entry = expired ? tracker.by_address.erase(entry) : std::next(entry);
    }
    for (auto entry = tracker.by_identity.begin();
         entry != tracker.by_identity.end();) {
        bool const expired = now >= entry->second.window_start &&
                             now - entry->second.window_start >= EXPIRY_SECONDS;
        entry = expired ? tracker.by_identity.erase(entry) : std::next(entry);
    }
}

} // namespace hypercom::server
