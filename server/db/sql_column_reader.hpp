#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "server/db/sql_statement.hpp"

namespace hypercom::server {

// Lecture des colonnes d'une ligne. Les index commencent a 0, comme dans
// sqlite -- l'asymetrie avec sql_binder vient de sqlite, pas de nous.

[[nodiscard]] std::int64_t read_integer(sql_statement const &statement,
                                        int index);

[[nodiscard]] std::string read_text(sql_statement const &statement, int index);

[[nodiscard]] std::vector<std::uint8_t> read_blob(
    sql_statement const &statement, int index);

// Pour les cles publiques et signatures : refuse si la colonne n'a pas
// exactement la taille attendue. Une cle de 31 octets en base est une base
// corrompue, pas une valeur a completer de zeros.
[[nodiscard]] bool read_fixed_bytes(sql_statement const &statement, int index,
                                    std::span<std::uint8_t> destination);

} // namespace hypercom::server
