#include "server/db/friend_repository.hpp"

#include "common/util/unix_clock.hpp"
#include "server/db/sql_binder.hpp"
#include "server/db/sql_column_reader.hpp"
#include "server/db/sql_statement.hpp"

namespace hypercom::server {

friend_repository::friend_repository(database_handle &database)
    : database_{database}
{
}

bool friend_repository::replace_friendship(std::int64_t user_id,
                                           std::int64_t friend_id,
                                           proto::friendship_status status)
{
    sql_statement statement{
        database_,
        "INSERT INTO friends (user_id, friend_id, status, created_at) "
        "VALUES (?1, ?2, ?3, ?4) "
        "ON CONFLICT(user_id, friend_id) DO UPDATE SET status = excluded.status"};
    if (!bind_integer(statement, 1, user_id)
        || !bind_integer(statement, 2, friend_id)
        || !bind_integer(statement, 3, static_cast<std::int64_t>(status))
        || !bind_integer(statement, 4,
                         static_cast<std::int64_t>(
                             util::get_unix_timestamp()))) {
        return false;
    }
    return statement.step_row() == step_result::done;
}

bool friend_repository::list_friends(std::int64_t user_id,
                                     std::uint16_t limit,
                                     std::vector<proto::friend_record> &out)
{
    sql_statement statement{
        database_,
        "SELECT u.pubkey, u.handle, COALESCE(p.display_name, ''), f.status,"
        "       f.created_at "
        "FROM friends f "
        "JOIN users u ON u.id = f.friend_id "
        "LEFT JOIN profiles p ON p.user_id = u.id "
        "WHERE f.user_id = ?1 ORDER BY f.created_at DESC LIMIT ?2"};
    if (!bind_integer(statement, 1, user_id)
        || !bind_integer(statement, 2, limit)) {
        return false;
    }
    out.clear();
    while (statement.step_row() == step_result::row) {
        proto::friend_record record;
        if (!read_fixed_bytes(statement, 0, record.pubkey)) {
            return false;
        }
        record.handle = read_text(statement, 1);
        record.display_name = read_text(statement, 2);
        record.status = static_cast<proto::friendship_status>(
            read_integer(statement, 3));
        record.created_at =
            static_cast<std::uint64_t>(read_integer(statement, 4));
        out.push_back(std::move(record));
    }
    return true;
}

} // namespace hypercom::server
