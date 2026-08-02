#include "server/db/user_repository.hpp"

#include "common/util/unix_clock.hpp"
#include "server/db/sql_binder.hpp"
#include "server/db/sql_column_reader.hpp"
#include "server/db/sql_statement.hpp"

namespace hypercom::server {
namespace {

constexpr char const *SELECT_COLUMNS =
    "SELECT id, pubkey, handle, created_at, last_seen FROM users ";

[[nodiscard]] bool read_user_row(sql_statement const &statement, user_row &out)
{
    out.id = read_integer(statement, 0);
    if (!read_fixed_bytes(statement, 1, out.pubkey)) {
        return false;
    }
    out.handle = read_text(statement, 2);
    out.created_at = static_cast<std::uint64_t>(read_integer(statement, 3));
    out.last_seen = static_cast<std::uint64_t>(read_integer(statement, 4));
    return true;
}

} // namespace

user_repository::user_repository(database_handle &database)
    : database_{database}
{
}

bool user_repository::find_by_pubkey(proto::wire_public_key const &pubkey,
                                     user_row &out)
{
    sql_statement statement{database_,
                            std::string{SELECT_COLUMNS} + "WHERE pubkey = ?1"};
    if (!bind_blob(statement, 1, pubkey)) {
        return false;
    }
    if (statement.step_row() != step_result::row) {
        return false;
    }
    return read_user_row(statement, out);
}

bool user_repository::find_by_id(std::int64_t id, user_row &out)
{
    sql_statement statement{database_,
                            std::string{SELECT_COLUMNS} + "WHERE id = ?1"};
    if (!bind_integer(statement, 1, id)) {
        return false;
    }
    if (statement.step_row() != step_result::row) {
        return false;
    }
    return read_user_row(statement, out);
}

bool user_repository::create_user(proto::wire_public_key const &pubkey,
                                  std::string_view handle,
                                  std::int64_t &out_id)
{
    sql_statement statement{
        database_,
        "INSERT INTO users (pubkey, handle, created_at, last_seen) "
        "VALUES (?1, ?2, ?3, ?3)"};
    if (!bind_blob(statement, 1, pubkey) || !bind_text(statement, 2, handle)
        || !bind_integer(statement, 3,
                         static_cast<std::int64_t>(
                             util::get_unix_timestamp()))) {
        return false;
    }
    // Un pseudo deja pris fait echouer la contrainte UNIQUE : c'est la base
    // qui arbitre, pas un SELECT prealable qui laisserait une fenetre de course.
    if (statement.step_row() != step_result::done) {
        return false;
    }
    out_id = database_.get_last_insert_id();
    return true;
}

bool user_repository::update_last_seen(std::int64_t id,
                                       std::uint64_t timestamp)
{
    sql_statement statement{database_,
                            "UPDATE users SET last_seen = ?2 WHERE id = ?1"};
    if (!bind_integer(statement, 1, id)
        || !bind_integer(statement, 2, static_cast<std::int64_t>(timestamp))) {
        return false;
    }
    return statement.step_row() == step_result::done;
}

} // namespace hypercom::server
