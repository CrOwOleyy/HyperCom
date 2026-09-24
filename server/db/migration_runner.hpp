#pragma once

#include "common/util/logger.hpp"
#include "server/db/database_handle.hpp"

#include <string>

namespace hypercom::server {

// Applies the missing migrations from db/migrations/NNNN_description.sql on
// startup. The operator can evolve the schema without recompiling anything.
//
// Each migration runs inside a transaction: it goes through entirely or not
// at all. A half-applied migration would leave a database whose state
// nobody knows.
[[nodiscard]] bool apply_pending_migrations(database_handle &database,
                                            std::string const &directory,
                                            util::logger &logger,
                                            std::string &error_out);

} // namespace hypercom::server
