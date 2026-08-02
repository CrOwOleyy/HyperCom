#pragma once

#include <cstdint>
#include <vector>

#include "common/protocol/social_records.hpp"
#include "server/db/database_handle.hpp"

namespace hypercom::server {

class friend_repository {
public:
    explicit friend_repository(database_handle &database);

    // La relation est unidirectionnelle et sans validation par l'autre partie :
    // il n'y a pas d'autorite pour arbitrer une demande. Chacun declare qui il
    // suit, et le client de chacun decide quoi en faire.
    [[nodiscard]] bool replace_friendship(std::int64_t user_id,
                                          std::int64_t friend_id,
                                          proto::friendship_status status);

    [[nodiscard]] bool list_friends(std::int64_t user_id,
                                    std::uint16_t limit,
                                    std::vector<proto::friend_record> &out);

private:
    database_handle &database_;
};

} // namespace hypercom::server
