#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "common/protocol/content_records.hpp"
#include "server/db/database_handle.hpp"

namespace hypercom::server {

class post_repository {
public:
    explicit post_repository(database_handle &database);

    [[nodiscard]] bool create_post(std::int64_t forum_id,
                                   std::int64_t author_id,
                                   std::string_view title,
                                   std::string_view body,
                                   std::int64_t &out_id);

    [[nodiscard]] bool find_by_id(std::int64_t id, proto::post_record &out);

    // Le corps est tronque a MAX_POST_PREVIEW_LENGTH dans une liste : c'est la
    // requete qui tronque, pas le C++, pour ne pas transporter 16 KiB par post
    // depuis sqlite avant de les jeter.
    [[nodiscard]] bool list_by_forum(std::int64_t forum_id,
                                     std::uint32_t offset, std::uint16_t limit,
                                     std::vector<proto::post_record> &out,
                                     std::uint32_t &total_count);

private:
    database_handle &database_;
};

} // namespace hypercom::server
