#include "server/db/profile_repository.hpp"

#include "server/db/sql_binder.hpp"
#include "server/db/sql_column_reader.hpp"
#include "server/db/sql_statement.hpp"

namespace hypercom::server {

profile_repository::profile_repository(database_handle &database)
    : database_{database}
{
}

bool profile_repository::replace_profile(
    std::int64_t user_id, proto::profile_set_request const &request)
{
    sql_statement statement{
        database_,
        "INSERT INTO profiles (user_id, display_name, bio, theme_json,"
        "                      banner_ref) VALUES (?1, ?2, ?3, ?4, ?5) "
        "ON CONFLICT(user_id) DO UPDATE SET "
        "  display_name = excluded.display_name,"
        "  bio = excluded.bio,"
        "  theme_json = excluded.theme_json,"
        "  banner_ref = excluded.banner_ref"};
    if (!bind_integer(statement, 1, user_id)
        || !bind_text(statement, 2, request.display_name)
        || !bind_text(statement, 3, request.bio)
        || !bind_text(statement, 4, request.theme_json)
        || !bind_text(statement, 5, request.banner_reference)) {
        return false;
    }
    return statement.step_row() == step_result::done;
}

bool profile_repository::find_by_pubkey(proto::wire_public_key const &pubkey,
                                        proto::profile_record &out)
{
    // LEFT JOIN : un compte existe des l'enregistrement, meme si son
    // proprietaire n'a jamais rempli de profil. COALESCE rend alors des
    // chaines vides plutot que de faire echouer la requete.
    sql_statement statement{
        database_,
        "SELECT u.pubkey, u.handle, COALESCE(p.display_name, ''),"
        "       COALESCE(p.bio, ''), COALESCE(p.theme_json, ''),"
        "       COALESCE(p.banner_ref, ''), u.created_at, u.last_seen "
        "FROM users u LEFT JOIN profiles p ON p.user_id = u.id "
        "WHERE u.pubkey = ?1"};
    if (!bind_blob(statement, 1, pubkey)
        || statement.step_row() != step_result::row) {
        return false;
    }
    if (!read_fixed_bytes(statement, 0, out.pubkey)) {
        return false;
    }
    out.handle = read_text(statement, 1);
    out.display_name = read_text(statement, 2);
    out.bio = read_text(statement, 3);
    out.theme_json = read_text(statement, 4);
    out.banner_reference = read_text(statement, 5);
    out.created_at = static_cast<std::uint64_t>(read_integer(statement, 6));
    out.last_seen = static_cast<std::uint64_t>(read_integer(statement, 7));
    return true;
}

} // namespace hypercom::server
