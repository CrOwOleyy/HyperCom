#pragma once

#include "common/protocol/social_records.hpp"
#include "server/db/database_handle.hpp"

#include <cstdint>
#include <vector>

namespace hypercom::server {

class friend_repository {
public:
    explicit friend_repository(database_handle &database);

    // The relationship is one-directional and requires no validation from
    // the other party: there is no authority to arbitrate a request. Each
    // person declares who they follow, and each person's client decides
    // what to do with that.
    [[nodiscard]] bool replace_friendship(std::int64_t user_id,
                                          std::int64_t friend_id,
                                          proto::friendship_status status);

    [[nodiscard]] bool list_friends(std::int64_t user_id, std::uint16_t limit,
                                    std::vector<proto::friend_record> &out);

private:
    database_handle &database_;
};

} // namespace hypercom::server
