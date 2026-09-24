#pragma once

#include "server/db/database_handle.hpp"

#include <string_view>

namespace hypercom::server {

enum class step_result {
    row,
    done,
    failed,
};

// A prepared statement, exclusively.
//
// There is no way to build a query from an assembled string: the only
// constructor takes a SQL literal, and values arrive afterward through
// sql_binder. An injection would require editing this file, not forgetting
// to escape something.
class sql_statement {
public:
    sql_statement(database_handle &database, std::string_view sql);

    [[nodiscard]] step_result step_row();

    // Resets the statement for a new set of parameters, which avoids
    // recompiling the SQL on every iteration.
    [[nodiscard]] bool reset_for_reuse();

    [[nodiscard]] sqlite3_stmt *get_raw_handle() const;

private:
    statement_pointer statement_;
};

} // namespace hypercom::server
