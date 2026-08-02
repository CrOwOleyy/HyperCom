#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include "server/db/sql_statement.hpp"

namespace hypercom::server {

// Liaison des parametres. Les index commencent a 1, comme dans sqlite.
//
// Toutes les valeurs sont copiees par sqlite (SQLITE_TRANSIENT) : l'appelant
// n'a pas a maintenir ses tampons vivants jusqu'au step, ce qui elimine une
// classe entiere d'usage-apres-liberation dans les depots.

[[nodiscard]] bool bind_integer(sql_statement &statement, int index,
                                std::int64_t value);

[[nodiscard]] bool bind_text(sql_statement &statement, int index,
                             std::string_view value);

[[nodiscard]] bool bind_blob(sql_statement &statement, int index,
                             std::span<std::uint8_t const> value);

} // namespace hypercom::server
