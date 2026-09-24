#pragma once

#include "server/db/database_handle.hpp"
#include "server/db/user_row.hpp"

#include <cstdint>
#include <string_view>

namespace hypercom::server {

// One repository per entity rather than one catch-all database class.
// That's what rule O3 requires in practice, and it works out well: you
// always know where to look for a query.
class user_repository {
public:
    explicit user_repository(database_handle &database);

    [[nodiscard]] bool find_by_pubkey(proto::wire_public_key const &pubkey,
                                      user_row &out);

    [[nodiscard]] bool find_by_id(std::int64_t id, user_row &out);

    [[nodiscard]] bool create_user(proto::wire_public_key const &pubkey,
                                   std::string_view handle,
                                   std::int64_t &out_id);

    // Reserved for action on a report (BRIEF.md 13). Revokes
    // authentication, doesn't touch any content already published.
    [[nodiscard]] bool set_banned(std::int64_t user_id, bool banned);

    // There is deliberately NO update_last_seen: recording the last
    // connection amounts to keeping a presence log, and it used to be
    // readable by everyone via profile_get. See migration
    // 0003_remove_last_seen.sql.

private:
    database_handle &database_;
};

} // namespace hypercom::server
