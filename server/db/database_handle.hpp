#pragma once

#include "server/db/sqlite_deleters.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace hypercom::server {

// The SQLite connection, in WAL mode.
//
// Nobody outside server/db calls sqlite3_* directly: business-logic
// handlers all go through a repository. That's what guarantees no query
// can be built by string concatenation anywhere in application logic.
class database_handle {
public:
    database_handle();

    // Opens the database and applies the operating PRAGMAs. Fails with a
    // usable message rather than opening a half-configured database.
    [[nodiscard]] bool open_database(std::string const &path,
                                     std::string &error_out);

    // Reserved for DDL and PRAGMA statements, i.e. SQL entirely written by
    // us. No user data ever passes through here.
    [[nodiscard]] bool execute_script(std::string_view sql,
                                      std::string &error_out);

    [[nodiscard]] sqlite3 *get_raw_handle() const;

    [[nodiscard]] std::int64_t get_last_insert_id() const;

    // Rows touched by the last UPDATE/DELETE. An UPDATE whose WHERE clause
    // carries the ownership check doesn't fail when it finds nothing: it
    // succeeds while touching zero rows. This counter is what tells the
    // two apart.
    [[nodiscard]] int get_changed_row_count() const;

private:
    database_pointer handle_;
};

} // namespace hypercom::server
