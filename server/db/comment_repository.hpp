#pragma once

#include "common/protocol/content_records.hpp"
#include "server/db/database_handle.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace hypercom::server {

// Maximum nesting depth. Without a cap, a long enough reply chain would
// blow up the recursive query and the client's rendering.
constexpr std::int64_t MAX_COMMENT_DEPTH = 24;

class comment_repository {
public:
    explicit comment_repository(database_handle &database);

    [[nodiscard]] bool create_comment(std::int64_t post_id,
                                      std::int64_t parent_comment_id,
                                      std::int64_t author_id,
                                      std::string_view body,
                                      std::int64_t &out_id);

    [[nodiscard]] bool find_by_id(std::int64_t id, proto::comment_record &out);

    // Returns the tree flattened, in preorder, each element carrying its
    // depth. The C++ side never recurses: WITH RECURSIVE does the
    // traversal, with a depth bound baked into the query itself.
    [[nodiscard]] bool list_thread(std::int64_t post_id, std::uint16_t limit,
                                   std::vector<proto::comment_record> &out,
                                   bool &truncated);

    // Checks that a parent exists AND actually belongs to the same post,
    // before accepting a reply. Without this check, a client could graft a
    // comment onto someone else's thread.
    [[nodiscard]] bool
    check_parent_belongs_to_post(std::int64_t parent_comment_id,
                                 std::int64_t post_id);

    // Same principle as delete_own_post: ownership lives in the WHERE
    // clause, and only the text disappears. The row stays, otherwise ON
    // DELETE CASCADE would take every reply with it -- meaning other
    // people's content.
    [[nodiscard]] bool delete_own_comment(std::int64_t comment_id,
                                          std::int64_t author_id);

private:
    database_handle &database_;
};

} // namespace hypercom::server
