#include "server/db/top8_repository.hpp"

#include <string>

#include "server/db/sql_binder.hpp"
#include "server/db/sql_column_reader.hpp"
#include "server/db/sql_statement.hpp"

namespace hypercom::server {

top8_repository::top8_repository(database_handle &database)
    : database_{database}
{
}

bool top8_repository::replace_slots(std::int64_t user_id,
                                    std::vector<top8_entry> const &entries)
{
    std::string error;
    if (!database_.execute_script("BEGIN IMMEDIATE", error)) {
        return false;
    }
    sql_statement remover{database_, "DELETE FROM top8 WHERE user_id = ?1"};
    bool succeeded = bind_integer(remover, 1, user_id)
                     && remover.step_row() == step_result::done;
    sql_statement inserter{
        database_,
        "INSERT INTO top8 (user_id, slot, friend_id) VALUES (?1, ?2, ?3)"};
    for (top8_entry const &entry : entries) {
        if (!succeeded) {
            break;
        }
        succeeded = inserter.reset_for_reuse()
                    && bind_integer(inserter, 1, user_id)
                    && bind_integer(inserter, 2,
                                    static_cast<std::int64_t>(entry.slot))
                    && bind_integer(inserter, 3, entry.friend_id)
                    && inserter.step_row() == step_result::done;
    }
    // Tout ou rien : un top 8 a moitie ecrit serait pire que pas de top 8.
    return database_.execute_script(succeeded ? "COMMIT" : "ROLLBACK", error)
           && succeeded;
}

bool top8_repository::list_slots(std::int64_t user_id,
                                 proto::top8_response &out)
{
    sql_statement statement{
        database_,
        "SELECT t.slot, u.pubkey, u.handle, COALESCE(p.display_name, ''),"
        "       COALESCE(f.status, 1), COALESCE(f.created_at, 0) "
        "FROM top8 t "
        "JOIN users u ON u.id = t.friend_id "
        "LEFT JOIN profiles p ON p.user_id = u.id "
        "LEFT JOIN friends f ON f.user_id = t.user_id AND f.friend_id = u.id "
        "WHERE t.user_id = ?1 ORDER BY t.slot"};
    if (!bind_integer(statement, 1, user_id)) {
        return false;
    }
    out.slots.fill(proto::wire_public_key{});
    out.details.clear();
    while (statement.step_row() == step_result::row) {
        auto const slot = static_cast<std::size_t>(read_integer(statement, 0));
        if (slot >= out.slots.size()) {
            continue;
        }
        if (!read_fixed_bytes(statement, 1, out.slots[slot])) {
            return false;
        }
        proto::friend_record record;
        record.pubkey = out.slots[slot];
        record.handle = read_text(statement, 2);
        record.display_name = read_text(statement, 3);
        record.status = static_cast<proto::friendship_status>(
            read_integer(statement, 4));
        record.created_at =
            static_cast<std::uint64_t>(read_integer(statement, 5));
        out.details.push_back(std::move(record));
    }
    return true;
}

} // namespace hypercom::server
