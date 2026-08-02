#include "server/db/post_repository.hpp"

#include <string>

#include "common/protocol/protocol_limits.hpp"
#include "common/util/unix_clock.hpp"
#include "server/db/sql_binder.hpp"
#include "server/db/sql_column_reader.hpp"
#include "server/db/sql_statement.hpp"

namespace hypercom::server {
namespace {

constexpr char const *SELECT_PREFIX =
    "SELECT p.id, p.forum_id, u.pubkey, u.handle, p.title, ";
constexpr char const *SELECT_SUFFIX =
    " , p.created_at,"
    "   (SELECT COUNT(*) FROM comments c WHERE c.post_id = p.id) "
    "FROM posts p JOIN users u ON u.id = p.author_id ";

[[nodiscard]] bool read_post_record(sql_statement const &statement,
                                    proto::post_record &out)
{
    out.id = static_cast<std::uint64_t>(read_integer(statement, 0));
    out.forum_id = static_cast<std::uint64_t>(read_integer(statement, 1));
    if (!read_fixed_bytes(statement, 2, out.author_pubkey)) {
        return false;
    }
    out.author_handle = read_text(statement, 3);
    out.title = read_text(statement, 4);
    out.body = read_text(statement, 5);
    out.created_at = static_cast<std::uint64_t>(read_integer(statement, 6));
    out.comment_count = static_cast<std::uint32_t>(read_integer(statement, 7));
    return true;
}

} // namespace

post_repository::post_repository(database_handle &database)
    : database_{database}
{
}

bool post_repository::create_post(std::int64_t forum_id,
                                  std::int64_t author_id,
                                  std::string_view title,
                                  std::string_view body, std::int64_t &out_id)
{
    sql_statement statement{
        database_,
        "INSERT INTO posts (forum_id, author_id, title, body, created_at) "
        "VALUES (?1, ?2, ?3, ?4, ?5)"};
    if (!bind_integer(statement, 1, forum_id)
        || !bind_integer(statement, 2, author_id)
        || !bind_text(statement, 3, title) || !bind_text(statement, 4, body)
        || !bind_integer(statement, 5,
                         static_cast<std::int64_t>(
                             util::get_unix_timestamp()))) {
        return false;
    }
    if (statement.step_row() != step_result::done) {
        return false;
    }
    out_id = database_.get_last_insert_id();
    return true;
}

bool post_repository::find_by_id(std::int64_t id, proto::post_record &out)
{
    sql_statement statement{database_, std::string{SELECT_PREFIX} + "p.body"
                                           + SELECT_SUFFIX + "WHERE p.id = ?1"};
    if (!bind_integer(statement, 1, id)
        || statement.step_row() != step_result::row) {
        return false;
    }
    return read_post_record(statement, out);
}

bool post_repository::list_by_forum(std::int64_t forum_id,
                                    std::uint32_t offset, std::uint16_t limit,
                                    std::vector<proto::post_record> &out,
                                    std::uint32_t &total_count)
{
    sql_statement counter{database_,
                          "SELECT COUNT(*) FROM posts WHERE forum_id = ?1"};
    if (!bind_integer(counter, 1, forum_id)
        || counter.step_row() != step_result::row) {
        return false;
    }
    total_count = static_cast<std::uint32_t>(read_integer(counter, 0));
    sql_statement statement{
        database_,
        std::string{SELECT_PREFIX} + "substr(p.body, 1, "
            + std::to_string(proto::MAX_POST_PREVIEW_LENGTH) + ")"
            + SELECT_SUFFIX
            + "WHERE p.forum_id = ?1 ORDER BY p.created_at DESC "
              "LIMIT ?2 OFFSET ?3"};
    if (!bind_integer(statement, 1, forum_id)
        || !bind_integer(statement, 2, limit)
        || !bind_integer(statement, 3, offset)) {
        return false;
    }
    out.clear();
    while (statement.step_row() == step_result::row) {
        proto::post_record record;
        if (!read_post_record(statement, record)) {
            return false;
        }
        out.push_back(std::move(record));
    }
    return true;
}

} // namespace hypercom::server
