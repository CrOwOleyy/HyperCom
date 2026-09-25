#pragma once

#include "common/protocol/content_records.hpp"
#include "server/db/database_handle.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace hypercom::server {

class post_repository {
public:
    explicit post_repository(database_handle &database);

    [[nodiscard]] bool create_post(std::int64_t forum_id,
                                   std::int64_t author_id,
                                   std::string_view title,
                                   std::string_view body, std::int64_t &out_id);

    [[nodiscard]] bool find_by_id(std::int64_t id, proto::post_record &out);

    // The body is truncated to MAX_POST_PREVIEW_LENGTH in a listing: the
    // query does the truncating, not the C++, to avoid hauling 16 KiB per
    // post out of sqlite just to discard it.
    [[nodiscard]] bool list_by_forum(std::int64_t forum_id,
                                     std::uint32_t offset, std::uint16_t limit,
                                     std::vector<proto::post_record> &out,
                                     std::uint32_t &total_count);

    // Ownership is checked in the WHERE clause, not in C++: a post that
    // doesn't belong to author_id matches no row. Returns false if nothing
    // was touched -- nonexistent, already deleted, or not theirs. The text
    // is wiped, the row survives so the thread's tree structure holds
    // together.
    [[nodiscard]] bool delete_own_post(std::int64_t post_id,
                                       std::int64_t author_id);

    // Reserved for the admin command `reports delete-post`.
    // Deliberately absent from the protocol side -- see
    // content_delete_handler.hpp. Unconditional, unlike delete_own_post:
    // it's up to the caller to make sure a legitimate report is the reason
    // behind it.
    [[nodiscard]] bool admin_delete_post(std::int64_t post_id);

private:
    database_handle &database_;
};

} // namespace hypercom::server
