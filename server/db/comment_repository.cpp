#include "server/db/comment_repository.hpp"

#include <string>

#include "common/util/unix_clock.hpp"
#include "server/db/sql_binder.hpp"
#include "server/db/sql_column_reader.hpp"
#include "server/db/sql_statement.hpp"

namespace hypercom::server {
namespace {

// path sert uniquement au tri : il place chaque reponse juste sous son parent,
// ce qui donne un parcours prefixe sans reconstruire d'arbre cote serveur.
constexpr char const *SELECT_THREAD =
    "WITH RECURSIVE thread(id, post_id, parent_comment_id, author_id, body,"
    "                      created_at, depth, path) AS ("
    "  SELECT c.id, c.post_id, c.parent_comment_id, c.author_id, c.body,"
    "         c.created_at, 0, printf('%010d', c.id)"
    "  FROM comments c"
    "  WHERE c.post_id = ?1 AND c.parent_comment_id IS NULL"
    "  UNION ALL"
    "  SELECT c.id, c.post_id, c.parent_comment_id, c.author_id, c.body,"
    "         c.created_at, t.depth + 1,"
    "         t.path || '/' || printf('%010d', c.id)"
    "  FROM comments c JOIN thread t ON c.parent_comment_id = t.id"
    "  WHERE t.depth < ?2"
    ") "
    "SELECT t.id, t.post_id, COALESCE(t.parent_comment_id, 0), u.pubkey,"
    "       u.handle, t.body, t.created_at, t.depth "
    "FROM thread t JOIN users u ON u.id = t.author_id "
    "ORDER BY t.path LIMIT ?3";

constexpr char const *SELECT_ONE =
    "SELECT c.id, c.post_id, COALESCE(c.parent_comment_id, 0), u.pubkey,"
    "       u.handle, c.body, c.created_at, 0 "
    "FROM comments c JOIN users u ON u.id = c.author_id WHERE c.id = ?1";

[[nodiscard]] bool read_comment_record(sql_statement const &statement,
                                       proto::comment_record &out)
{
    out.id = static_cast<std::uint64_t>(read_integer(statement, 0));
    out.post_id = static_cast<std::uint64_t>(read_integer(statement, 1));
    out.parent_comment_id =
        static_cast<std::uint64_t>(read_integer(statement, 2));
    if (!read_fixed_bytes(statement, 3, out.author_pubkey)) {
        return false;
    }
    out.author_handle = read_text(statement, 4);
    out.body = read_text(statement, 5);
    out.created_at = static_cast<std::uint64_t>(read_integer(statement, 6));
    out.depth = static_cast<std::uint16_t>(read_integer(statement, 7));
    return true;
}

} // namespace

comment_repository::comment_repository(database_handle &database)
    : database_{database}
{
}

bool comment_repository::create_comment(std::int64_t post_id,
                                        std::int64_t parent_comment_id,
                                        std::int64_t author_id,
                                        std::string_view body,
                                        std::int64_t &out_id)
{
    sql_statement statement{
        database_,
        "INSERT INTO comments (post_id, parent_comment_id, author_id, body,"
        "                      created_at) "
        "VALUES (?1, NULLIF(?2, 0), ?3, ?4, ?5)"};
    if (!bind_integer(statement, 1, post_id)
        || !bind_integer(statement, 2, parent_comment_id)
        || !bind_integer(statement, 3, author_id)
        || !bind_text(statement, 4, body)
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

bool comment_repository::find_by_id(std::int64_t id,
                                    proto::comment_record &out)
{
    sql_statement statement{database_, SELECT_ONE};
    if (!bind_integer(statement, 1, id)
        || statement.step_row() != step_result::row) {
        return false;
    }
    return read_comment_record(statement, out);
}

bool comment_repository::list_thread(std::int64_t post_id,
                                     std::uint16_t limit,
                                     std::vector<proto::comment_record> &out,
                                     bool &truncated)
{
    sql_statement statement{database_, SELECT_THREAD};
    // On demande un element de plus que la limite : si sqlite le rend, c'est
    // qu'il y en avait davantage, et le client doit le savoir.
    if (!bind_integer(statement, 1, post_id)
        || !bind_integer(statement, 2, MAX_COMMENT_DEPTH)
        || !bind_integer(statement, 3, static_cast<std::int64_t>(limit) + 1)) {
        return false;
    }
    out.clear();
    truncated = false;
    while (statement.step_row() == step_result::row) {
        if (out.size() >= limit) {
            truncated = true;
            break;
        }
        proto::comment_record record;
        if (!read_comment_record(statement, record)) {
            return false;
        }
        out.push_back(std::move(record));
    }
    return true;
}

bool comment_repository::check_parent_belongs_to_post(
    std::int64_t parent_comment_id, std::int64_t post_id)
{
    sql_statement statement{
        database_, "SELECT 1 FROM comments WHERE id = ?1 AND post_id = ?2"};
    if (!bind_integer(statement, 1, parent_comment_id)
        || !bind_integer(statement, 2, post_id)) {
        return false;
    }
    return statement.step_row() == step_result::row;
}

} // namespace hypercom::server
