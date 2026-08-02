#include "server/db/sqlite_deleters.hpp"

#include <sqlite3.h>

namespace hypercom::server {

void database_deleter::operator()(sqlite3 *handle) const
{
    // sqlite3_close_v2 tolere les requetes encore vivantes : la base se ferme
    // quand la derniere disparait. sqlite3_close echouerait a la place, et
    // laisserait le fichier ouvert.
    sqlite3_close_v2(handle);
}

void statement_deleter::operator()(sqlite3_stmt *handle) const
{
    sqlite3_finalize(handle);
}

} // namespace hypercom::server
