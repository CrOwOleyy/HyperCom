#pragma once

#include <memory>

struct sqlite3;
struct sqlite3_stmt;

namespace hypercom::server {

// RAII for the two sqlite handles. By going through unique_ptr, the
// classes that own them need neither a destructor nor a deleted copy
// constructor: they become movable and non-copyable by construction.
// Three member functions saved per class, and above all no possible leak
// on an error path.
struct database_deleter {
    void operator()(sqlite3 *handle) const;
};

struct statement_deleter {
    void operator()(sqlite3_stmt *handle) const;
};

using database_pointer = std::unique_ptr<sqlite3, database_deleter>;
using statement_pointer = std::unique_ptr<sqlite3_stmt, statement_deleter>;

} // namespace hypercom::server
