#pragma once

#include "server/db/database_handle.hpp"

#include <cstdint>

namespace hypercom::server {

// Counters shown by the admin `stats` command.
//
// Aggregates only: none of these numbers can be traced back to a person.
// This is consistent with everything else -- admin exists to operate the
// server, not to observe its users.
struct server_counts {
    std::int64_t users = 0;
    std::int64_t forums = 0;
    std::int64_t posts = 0;
    std::int64_t comments = 0;
    std::int64_t pending_envelopes = 0;
};

class stats_repository {
public:
    explicit stats_repository(database_handle &database);

    [[nodiscard]] bool collect_counts(server_counts &out);

private:
    database_handle &database_;
};

} // namespace hypercom::server
