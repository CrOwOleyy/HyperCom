#include "server/db/stats_repository.hpp"

#include "server/db/sql_column_reader.hpp"
#include "server/db/sql_statement.hpp"

namespace hypercom::server {
namespace {

[[nodiscard]] bool count_rows(database_handle &database, std::string_view sql,
                              std::int64_t &out)
{
    sql_statement statement{database, sql};
    if (statement.step_row() != step_result::row) {
        return false;
    }
    out = read_integer(statement, 0);
    return true;
}

} // namespace

stats_repository::stats_repository(database_handle &database)
    : database_{database}
{
}

bool stats_repository::collect_counts(server_counts &out)
{
    return count_rows(database_, "SELECT COUNT(*) FROM users", out.users)
        && count_rows(database_, "SELECT COUNT(*) FROM forums", out.forums)
        && count_rows(database_, "SELECT COUNT(*) FROM posts", out.posts)
        && count_rows(database_, "SELECT COUNT(*) FROM comments", out.comments)
        && count_rows(database_, "SELECT COUNT(*) FROM dm_envelopes",
                      out.pending_envelopes);
}

} // namespace hypercom::server
