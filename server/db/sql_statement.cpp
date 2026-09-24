#include "server/db/sql_statement.hpp"

#include <sqlite3.h>

namespace hypercom::server {

sql_statement::sql_statement(database_handle &database, std::string_view sql)
    : statement_{nullptr}
{
    if (database.get_raw_handle() == nullptr) {
        return;
    }
    sqlite3_stmt *raw = nullptr;
    // SQLITE_PREPARE_PERSISTENT: these statements get reused for the
    // server's entire lifetime, might as well tell the query planner.
    if (sqlite3_prepare_v3(
            database.get_raw_handle(), sql.data(), static_cast<int>(sql.size()),
            SQLITE_PREPARE_PERSISTENT, &raw, nullptr) != SQLITE_OK) {
        return;
    }
    statement_.reset(raw);
}

step_result sql_statement::step_row()
{
    if (statement_ == nullptr) {
        return step_result::failed;
    }
    switch (sqlite3_step(statement_.get())) {
        case SQLITE_ROW:
            return step_result::row;
        case SQLITE_DONE:
            return step_result::done;
        default:
            return step_result::failed;
    }
}

bool sql_statement::reset_for_reuse()
{
    if (statement_ == nullptr) {
        return false;
    }
    sqlite3_clear_bindings(statement_.get());
    return sqlite3_reset(statement_.get()) == SQLITE_OK;
}

sqlite3_stmt *sql_statement::get_raw_handle() const
{
    return statement_.get();
}

} // namespace hypercom::server
