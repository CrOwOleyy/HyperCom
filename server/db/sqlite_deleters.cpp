#include "server/db/sqlite_deleters.hpp"

#include <sqlite3.h>

namespace hypercom::server {

void database_deleter::operator()(sqlite3 *handle) const
{
    // sqlite3_close_v2 tolerates statements that are still alive: the
    // database closes once the last one disappears. sqlite3_close would
    // fail instead, and leave the file open.
    sqlite3_close_v2(handle);
}

void statement_deleter::operator()(sqlite3_stmt *handle) const
{
    sqlite3_finalize(handle);
}

} // namespace hypercom::server
