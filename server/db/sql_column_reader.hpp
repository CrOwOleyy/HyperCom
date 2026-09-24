#pragma once

#include "server/db/sql_statement.hpp"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace hypercom::server {

// Reading a row's columns. Indices start at 0, as in sqlite -- the
// asymmetry with sql_binder comes from sqlite, not from us.

[[nodiscard]] std::int64_t read_integer(sql_statement const &statement,
                                        int index);

[[nodiscard]] std::string read_text(sql_statement const &statement, int index);

[[nodiscard]] std::vector<std::uint8_t>
read_blob(sql_statement const &statement, int index);

// For public keys and signatures: rejects if the column isn't exactly the
// expected size. A 31-byte key in the database means a corrupted
// database, not a value to zero-pad.
[[nodiscard]] bool read_fixed_bytes(sql_statement const &statement, int index,
                                    std::span<std::uint8_t> destination);

} // namespace hypercom::server
