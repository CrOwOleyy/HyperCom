#pragma once

#include "common/protocol/wire_key.hpp"

#include <cstdint>
#include <string>

namespace hypercom::server {

// The other repositories fill common/protocol records directly: those are
// already plain data structures, and adding a mirrored type layer would
// only double the copying code without preventing anything. users is the
// one case with no protocol equivalent -- a user row is never served
// as-is over the wire, only its profile is.
struct user_row {
    std::int64_t id = 0;
    proto::wire_public_key pubkey{};
    std::string handle;
    // Reserved for action on a report: a banned account can no longer
    // authenticate, which leaves its past content untouched --
    // banning and deleting remain two distinct, deliberate actions.
    bool banned = false;
};

} // namespace hypercom::server
