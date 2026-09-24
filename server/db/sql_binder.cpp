#include "server/db/sql_binder.hpp"

#include <sqlite3.h>

namespace hypercom::server {

bool bind_integer(sql_statement &statement, int index, std::int64_t value)
{
    if (statement.get_raw_handle() == nullptr) {
        return false;
    }
    return sqlite3_bind_int64(statement.get_raw_handle(), index, value) ==
           SQLITE_OK;
}

bool bind_text(sql_statement &statement, int index, std::string_view value)
{
    if (statement.get_raw_handle() == nullptr) {
        return false;
    }
    return sqlite3_bind_text(statement.get_raw_handle(), index, value.data(),
                             static_cast<int>(value.size()),
                             SQLITE_TRANSIENT) == SQLITE_OK;
}

bool bind_blob(sql_statement &statement, int index,
               std::span<std::uint8_t const> value)
{
    if (statement.get_raw_handle() == nullptr) {
        return false;
    }
    // sqlite rejects a null pointer with a zero length on some code paths:
    // we give it a valid address and a length of zero instead.
    static constexpr std::uint8_t EMPTY_MARKER = 0;
    void const *const data = value.empty()
                                 ? static_cast<void const *>(&EMPTY_MARKER)
                                 : static_cast<void const *>(value.data());
    return sqlite3_bind_blob(statement.get_raw_handle(), index, data,
                             static_cast<int>(value.size()),
                             SQLITE_TRANSIENT) == SQLITE_OK;
}

} // namespace hypercom::server
