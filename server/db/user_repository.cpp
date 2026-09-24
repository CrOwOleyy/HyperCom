#include "server/db/user_repository.hpp"

#include "server/db/sql_binder.hpp"
#include "server/db/sql_column_reader.hpp"
#include "server/db/sql_statement.hpp"

namespace hypercom::server {
namespace {

constexpr char const *SELECT_COLUMNS =
    "SELECT id, pubkey, handle, banned FROM users ";

[[nodiscard]] bool read_user_row(sql_statement const &statement, user_row &out)
{
    out.id = read_integer(statement, 0);
    if (!read_fixed_bytes(statement, 1, out.pubkey)) {
        return false;
    }
    out.handle = read_text(statement, 2);
    out.banned = read_integer(statement, 3) != 0;
    return true;
}

} // namespace

user_repository::user_repository(database_handle &database)
    : database_{database}
{}

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
                                  std::string_view handle, std::int64_t &out_id)
{
    // No creation date, no login date: an account is a public key and a
    // handle, nothing else.
    sql_statement statement{
        database_, "INSERT INTO users (pubkey, handle) VALUES (?1, ?2)"};
    if (!bind_blob(statement, 1, pubkey) || !bind_text(statement, 2, handle)) {
        return false;
    }
    // A handle that's already taken fails the UNIQUE constraint: the
    // database is the arbiter, not a prior SELECT that would leave a race
    // window.
    if (statement.step_row() != step_result::done) {
        return false;
    }
    out_id = database_.get_last_insert_id();
    return true;
}

bool user_repository::set_banned(std::int64_t user_id, bool banned)
{
    sql_statement statement{database_,
                            "UPDATE users SET banned = ?1 WHERE id = ?2"};
    if (!bind_integer(statement, 1, banned ? 1 : 0) ||
        !bind_integer(statement, 2, user_id)) {
        return false;
    }
    return statement.step_row() == step_result::done;
}

} // namespace hypercom::server
