#include "server/db/forum_repository.hpp"

#include <string>

#include "common/util/unix_clock.hpp"
#include "server/db/sql_binder.hpp"
#include "server/db/sql_column_reader.hpp"
#include "server/db/sql_statement.hpp"

namespace hypercom::server {
namespace {

// Le nombre de posts est calcule par sous-requete plutot que maintenu dans une
// colonne : un compteur denormalise finit toujours par diverger, et a cette
// echelle le cout est negligeable.
constexpr char const *SELECT_FORUM =
    "SELECT f.id, f.name, f.description, u.pubkey, u.handle, f.theme_json,"
    "       f.created_at,"
    "       (SELECT COUNT(*) FROM posts p WHERE p.forum_id = f.id) "
    "FROM forums f JOIN users u ON u.id = f.founder_id ";

[[nodiscard]] bool read_forum_record(sql_statement const &statement,
                                     proto::forum_record &out)
{
    out.id = static_cast<std::uint64_t>(read_integer(statement, 0));
    out.name = read_text(statement, 1);
    out.description = read_text(statement, 2);
    if (!read_fixed_bytes(statement, 3, out.founder_pubkey)) {
        return false;
    }
    out.founder_handle = read_text(statement, 4);
    out.theme_json = read_text(statement, 5);
    out.created_at = static_cast<std::uint64_t>(read_integer(statement, 6));
    out.post_count = static_cast<std::uint32_t>(read_integer(statement, 7));
    return true;
}

} // namespace

forum_repository::forum_repository(database_handle &database)
    : database_{database}
{
}

bool forum_repository::create_forum(std::int64_t founder_id,
                                    std::string_view name,
                                    std::string_view description,
                                    std::string_view theme_json,
                                    std::int64_t &out_id)
{
    sql_statement statement{
        database_,
        "INSERT INTO forums (name, founder_id, description, theme_json,"
        "                    created_at) VALUES (?1, ?2, ?3, ?4, ?5)"};
    if (!bind_text(statement, 1, name)
        || !bind_integer(statement, 2, founder_id)
        || !bind_text(statement, 3, description)
        || !bind_text(statement, 4, theme_json)
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

bool forum_repository::find_by_id(std::int64_t id, proto::forum_record &out)
{
    sql_statement statement{database_,
                            std::string{SELECT_FORUM} + "WHERE f.id = ?1"};
    if (!bind_integer(statement, 1, id)
        || statement.step_row() != step_result::row) {
        return false;
    }
    return read_forum_record(statement, out);
}

bool forum_repository::find_by_name(std::string_view name,
                                    proto::forum_record &out)
{
    sql_statement statement{database_,
                            std::string{SELECT_FORUM} + "WHERE f.name = ?1"};
    if (!bind_text(statement, 1, name)
        || statement.step_row() != step_result::row) {
        return false;
    }
    return read_forum_record(statement, out);
}

bool forum_repository::list_forums(std::uint32_t offset, std::uint16_t limit,
                                   std::vector<proto::forum_record> &out,
                                   std::uint32_t &total_count)
{
    sql_statement counter{database_, "SELECT COUNT(*) FROM forums"};
    if (counter.step_row() != step_result::row) {
        return false;
    }
    total_count = static_cast<std::uint32_t>(read_integer(counter, 0));
    sql_statement statement{database_, std::string{SELECT_FORUM}
                                           + "ORDER BY f.created_at DESC "
                                             "LIMIT ?1 OFFSET ?2"};
    if (!bind_integer(statement, 1, limit)
        || !bind_integer(statement, 2, offset)) {
        return false;
    }
    out.clear();
    while (statement.step_row() == step_result::row) {
        proto::forum_record record;
        if (!read_forum_record(statement, record)) {
            return false;
        }
        out.push_back(std::move(record));
    }
    return true;
}

} // namespace hypercom::server
