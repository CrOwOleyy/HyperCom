#include "server/db/prekey_repository.hpp"

#include "common/util/unix_clock.hpp"
#include "server/db/sql_binder.hpp"
#include "server/db/sql_column_reader.hpp"
#include "server/db/sql_statement.hpp"

namespace hypercom::server {

prekey_repository::prekey_repository(database_handle &database)
    : database_{database}
{
}

bool prekey_repository::replace_prekey(std::int64_t user_id,
                                       proto::wire_public_key const &prekey,
                                       proto::wire_signature const &signature)
{
    sql_statement statement{
        database_,
        "INSERT INTO prekeys (user_id, prekey, signature, created_at) "
        "VALUES (?1, ?2, ?3, ?4) "
        "ON CONFLICT(user_id) DO UPDATE SET "
        "  prekey = excluded.prekey,"
        "  signature = excluded.signature,"
        "  created_at = excluded.created_at"};
    if (!bind_integer(statement, 1, user_id)
        || !bind_blob(statement, 2, prekey)
        || !bind_blob(statement, 3, signature)
        || !bind_integer(statement, 4,
                         static_cast<std::int64_t>(
                             util::get_unix_timestamp()))) {
        return false;
    }
    return statement.step_row() == step_result::done;
}

bool prekey_repository::find_bundle_by_pubkey(
    proto::wire_public_key const &owner_pubkey,
    proto::prekey_bundle_response &out)
{
    sql_statement statement{
        database_,
        "SELECT p.prekey, p.signature, p.created_at "
        "FROM prekeys p JOIN users u ON u.id = p.user_id "
        "WHERE u.pubkey = ?1"};
    if (!bind_blob(statement, 1, owner_pubkey)) {
        return false;
    }
    if (statement.step_row() != step_result::row) {
        return false;
    }
    out.owner_pubkey = owner_pubkey;
    if (!read_fixed_bytes(statement, 0, out.prekey)
        || !read_fixed_bytes(statement, 1, out.signature)) {
        return false;
    }
    out.created_at = static_cast<std::uint64_t>(read_integer(statement, 2));
    return true;
}

} // namespace hypercom::server
