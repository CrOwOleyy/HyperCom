#include "server/db/sql_column_reader.hpp"

#include <algorithm>

#include <sqlite3.h>

namespace hypercom::server {

std::int64_t read_integer(sql_statement const &statement, int index)
{
    if (statement.get_raw_handle() == nullptr) {
        return 0;
    }
    return sqlite3_column_int64(statement.get_raw_handle(), index);
}

std::string read_text(sql_statement const &statement, int index)
{
    if (statement.get_raw_handle() == nullptr) {
        return {};
    }
    auto const *const data = reinterpret_cast<char const *>(
        sqlite3_column_text(statement.get_raw_handle(), index));
    if (data == nullptr) {
        return {};
    }
    int const size = sqlite3_column_bytes(statement.get_raw_handle(), index);
    return std::string{data, static_cast<std::size_t>(size)};
}

std::vector<std::uint8_t> read_blob(sql_statement const &statement, int index)
{
    if (statement.get_raw_handle() == nullptr) {
        return {};
    }
    auto const *const data = static_cast<std::uint8_t const *>(
        sqlite3_column_blob(statement.get_raw_handle(), index));
    int const size = sqlite3_column_bytes(statement.get_raw_handle(), index);
    if (data == nullptr || size <= 0) {
        return {};
    }
    return std::vector<std::uint8_t>{data, data + size};
}

bool read_fixed_bytes(sql_statement const &statement, int index,
                      std::span<std::uint8_t> destination)
{
    if (statement.get_raw_handle() == nullptr) {
        return false;
    }
    auto const *const data = static_cast<std::uint8_t const *>(
        sqlite3_column_blob(statement.get_raw_handle(), index));
    int const size = sqlite3_column_bytes(statement.get_raw_handle(), index);
    if (data == nullptr || size < 0
        || static_cast<std::size_t>(size) != destination.size()) {
        return false;
    }
    std::copy_n(data, destination.size(), destination.begin());
    return true;
}

} // namespace hypercom::server
