#pragma once

#include "server/db/sql_statement.hpp"

#include <cstdint>
#include <span>
#include <string_view>

namespace hypercom::server {

// Parameter binding. Indices start at 1, as in sqlite.
//
// All values are copied by sqlite (SQLITE_TRANSIENT): the caller doesn't
// have to keep its buffers alive until the step, which eliminates a whole
// class of use-after-free bugs in the repositories.

[[nodiscard]] bool bind_integer(sql_statement &statement, int index,
                                std::int64_t value);

[[nodiscard]] bool bind_text(sql_statement &statement, int index,
                             std::string_view value);

[[nodiscard]] bool bind_blob(sql_statement &statement, int index,
                             std::span<std::uint8_t const> value);

} // namespace hypercom::server
