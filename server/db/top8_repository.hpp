#pragma once

#include "common/protocol/top8_message.hpp"
#include "server/db/database_handle.hpp"

#include <cstdint>
#include <vector>

namespace hypercom::server {

struct top8_entry {
    std::uint8_t slot = 0;
    std::int64_t friend_id = 0;
};

class top8_repository {
public:
    explicit top8_repository(database_handle &database);

    // Full replacement rather than a per-slot update: the client sends
    // the complete state of all eight slots, which avoids any drift
    // between what it displays and what's stored.
    [[nodiscard]] bool replace_slots(std::int64_t user_id,
                                     std::vector<top8_entry> const &entries);

    [[nodiscard]] bool list_slots(std::int64_t user_id,
                                  proto::top8_response &out);

private:
    database_handle &database_;
};

} // namespace hypercom::server
