#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "common/protocol/forum_record.hpp"
#include "server/db/database_handle.hpp"

namespace hypercom::server {

class forum_repository {
public:
    explicit forum_repository(database_handle &database);

    // La creation est libre : le createur devient fondateur de son espace, et
    // le serveur n'a aucun avis sur le sujet du forum.
    [[nodiscard]] bool create_forum(std::int64_t founder_id,
                                    std::string_view name,
                                    std::string_view description,
                                    std::string_view theme_json,
                                    std::int64_t &out_id);

    [[nodiscard]] bool find_by_id(std::int64_t id, proto::forum_record &out);

    [[nodiscard]] bool find_by_name(std::string_view name,
                                    proto::forum_record &out);

    [[nodiscard]] bool list_forums(std::uint32_t offset, std::uint16_t limit,
                                   std::vector<proto::forum_record> &out,
                                   std::uint32_t &total_count);

private:
    database_handle &database_;
};

} // namespace hypercom::server
