#include "server/db/report_repository.hpp"

#include <algorithm>

#include "common/util/unix_clock.hpp"
#include "server/db/sql_binder.hpp"
#include "server/db/sql_column_reader.hpp"
#include "server/db/sql_statement.hpp"

namespace hypercom::server {
namespace {

constexpr char const *SELECT_COLUMNS =
    "SELECT id, kind, post_id, target_pubkey, reporter_id, reason, "
    "created_at FROM reports ";

[[nodiscard]] bool read_report_row(sql_statement const &statement,
                                   report_row &out)
{
    out.id = read_integer(statement, 0);
    out.kind = read_text(statement, 1);
    out.post_id = read_integer(statement, 2);
    // target_pubkey est NULL pour un signalement de post : une colonne
    // absente donne un blob vide, pas une erreur -- la case reste a zero.
    std::vector<std::uint8_t> const raw_pubkey = read_blob(statement, 3);
    if (raw_pubkey.size() == out.target_pubkey.size()) {
        std::copy(raw_pubkey.begin(), raw_pubkey.end(),
                 out.target_pubkey.begin());
    }
    out.reporter_id = read_integer(statement, 4);
    out.reason = read_text(statement, 5);
    out.created_at = read_integer(statement, 6);
    return true;
}

} // namespace

report_repository::report_repository(database_handle &database)
    : database_{database}
{
}

bool report_repository::record_post_report(std::int64_t post_id,
                                           std::int64_t reporter_id,
                                           std::string_view reason)
{
    sql_statement statement{
        database_,
        "INSERT INTO reports (kind, post_id, reporter_id, reason, created_at)"
        " VALUES ('post', ?1, ?2, ?3, ?4)"};
    if (!bind_integer(statement, 1, post_id)
        || !bind_integer(statement, 2, reporter_id)
        || !bind_text(statement, 3, reason)
        || !bind_integer(statement, 4,
                         static_cast<std::int64_t>(util::get_unix_timestamp()))) {
        return false;
    }
    return statement.step_row() == step_result::done;
}

bool report_repository::record_account_report(
    proto::wire_public_key const &target_pubkey, std::int64_t reporter_id,
    std::string_view reason)
{
    sql_statement statement{
        database_,
        "INSERT INTO reports (kind, target_pubkey, reporter_id, reason, "
        "created_at) VALUES ('account', ?1, ?2, ?3, ?4)"};
    if (!bind_blob(statement, 1, target_pubkey)
        || !bind_integer(statement, 2, reporter_id)
        || !bind_text(statement, 3, reason)
        || !bind_integer(statement, 4,
                         static_cast<std::int64_t>(util::get_unix_timestamp()))) {
        return false;
    }
    return statement.step_row() == step_result::done;
}

bool report_repository::list_reports(std::uint16_t limit,
                                     std::vector<report_row> &out)
{
    sql_statement statement{
        database_, std::string{SELECT_COLUMNS}
                       + "ORDER BY created_at ASC LIMIT ?1"};
    if (!bind_integer(statement, 1, static_cast<std::int64_t>(limit))) {
        return false;
    }
    out.clear();
    while (statement.step_row() == step_result::row) {
        report_row row;
        if (!read_report_row(statement, row)) {
            return false;
        }
        out.push_back(std::move(row));
    }
    return true;
}

bool report_repository::clear_report(std::int64_t id)
{
    sql_statement statement{database_, "DELETE FROM reports WHERE id = ?1"};
    if (!bind_integer(statement, 1, id)) {
        return false;
    }
    return statement.step_row() == step_result::done;
}

} // namespace hypercom::server
