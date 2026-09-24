#pragma once

#include "common/util/log_level.hpp"

#include <cstdint>
#include <iosfwd>
#include <string_view>

namespace hypercom::util {

// Explicit logging: no global instance, no singleton (G4). The logger
// is passed as a parameter to anything that needs to log.
//
// The logger also carries the peer-address policy, and that's a design
// choice more than a detail: by default it's false, so no IP gets
// logged. Callers don't have to think about it -- they go through
// redact_peer_address(), which returns a neutral string until
// hypercom.conf has explicitly opened up the policy.
class logger {
public:
    logger(log_level minimum, bool allow_peer_addresses, std::ostream &sink);

    void write_entry(log_level level, std::string_view message);

    [[nodiscard]] bool is_level_enabled(log_level level) const;

    // Returns the address as-is if and only if the policy allows it, and
    // "[redacted]" otherwise. This is the only path allowed into a log.
    [[nodiscard]] std::string_view
    redact_peer_address(std::string_view address) const;

private:
    void write_prefix(log_level level);

    log_level minimum_;
    bool allow_peer_addresses_;
    std::ostream &sink_;
};

} // namespace hypercom::util
