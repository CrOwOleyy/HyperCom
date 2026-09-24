#include "server/db/motd_repository.hpp"

#include "common/util/unix_clock.hpp"
#include "server/db/sql_binder.hpp"
#include "server/db/sql_column_reader.hpp"
#include "server/db/sql_statement.hpp"

#include <string>

namespace hypercom::server {

motd_repository::motd_repository(database_handle &database)
    : database_{database}
{}

bool motd_repository::find_active_motd(proto::motd_push &out)
{
    sql_statement statement{
        database_, "SELECT id, body FROM motd WHERE active = 1 LIMIT 1"};
    if (statement.step_row() != step_result::row) {
        return false;
    }
    // The id serves as the revision number: this lets the client know
    // whether it has already shown this announcement, without comparing
    // strings.
    out.revision = static_cast<std::uint64_t>(read_integer(statement, 0));
    out.body = read_text(statement, 1);
    return true;
}

bool motd_repository::publish_motd(std::string_view body)
{
    std::string error;
    if (!database_.execute_script("BEGIN IMMEDIATE", error)) {
        return false;
    }
    // The schema's partial unique index only allows a single active row:
    // the old one must therefore be deactivated before the new one is
    // inserted, within the same transaction.
    bool succeeded = database_.execute_script(
        "UPDATE motd SET active = 0 WHERE active = 1", error);
    if (succeeded) {
        sql_statement inserter{
            database_,
            "INSERT INTO motd (body, active, created_at) VALUES (?1, 1, ?2)"};
        succeeded = bind_text(inserter, 1, body) &&
                    bind_integer(inserter, 2,
                                 static_cast<std::int64_t>(
                                     util::get_unix_timestamp())) &&
                    inserter.step_row() == step_result::done;
    }
    return database_.execute_script(succeeded ? "COMMIT" : "ROLLBACK", error) &&
           succeeded;
}

bool motd_repository::clear_active_motd()
{
    std::string error;
    return database_.execute_script(
        "UPDATE motd SET active = 0 WHERE active = 1", error);
}

} // namespace hypercom::server
