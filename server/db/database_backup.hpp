#pragma once

#include "server/db/database_handle.hpp"

#include <string>

namespace hypercom::server {

// Hot backup, without stopping the server.
//
// Goes through SQLite's backup API, not a file copy. The difference isn't
// cosmetic: in WAL mode, part of the data lives in the journal, and a `cp`
// during a write produces an inconsistent file. The API takes a
// consistent snapshot instead.
[[nodiscard]] bool backup_database_to(database_handle &source,
                                      std::string const &destination_path,
                                      std::string &error_out);

} // namespace hypercom::server
