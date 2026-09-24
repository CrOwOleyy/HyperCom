#include "server/db/migration_runner.hpp"

#include "common/util/unix_clock.hpp"
#include "server/db/sql_binder.hpp"
#include "server/db/sql_statement.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace hypercom::server {
namespace {

constexpr char const *MIGRATIONS_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS schema_migrations ("
    "  name       TEXT PRIMARY KEY,"
    "  applied_at INTEGER NOT NULL)";

struct migration_file {
    std::string name;
    std::filesystem::path path;
};

[[nodiscard]] bool collect_migration_files(std::string const &directory,
                                           std::vector<migration_file> &out,
                                           std::string &error_out)
{
    std::error_code failure;
    if (!std::filesystem::is_directory(directory, failure)) {
        error_out = "repertoire de migrations introuvable : " + directory;
        return false;
    }
    for (auto const &entry :
         std::filesystem::directory_iterator{directory, failure}) {
        if (!entry.is_regular_file() || entry.path().extension() != ".sql") {
            continue;
        }
        out.push_back({entry.path().filename().string(), entry.path()});
    }
    // Lexicographic sorting on NNNN_... gives chronological order as long
    // as the numeric prefix has a fixed width. That's the project's
    // convention.
    std::sort(out.begin(), out.end(),
              [](migration_file const &left, migration_file const &right) {
                  return left.name < right.name;
              });
    return true;
}

[[nodiscard]] bool is_already_applied(database_handle &database,
                                      std::string const &name, bool &out)
{
    sql_statement statement{database,
                            "SELECT 1 FROM schema_migrations WHERE name = ?1"};
    if (!bind_text(statement, 1, name)) {
        return false;
    }
    step_result const result = statement.step_row();
    if (result == step_result::failed) {
        return false;
    }
    out = result == step_result::row;
    return true;
}

[[nodiscard]] bool record_applied_migration(database_handle &database,
                                            std::string const &name,
                                            std::string &error_out)
{
    sql_statement statement{
        database,
        "INSERT INTO schema_migrations (name, applied_at) VALUES (?1, ?2)"};
    if (!bind_text(statement, 1, name) ||
        !bind_integer(statement, 2,
                      static_cast<std::int64_t>(util::get_unix_timestamp()))) {
        error_out = "liaison impossible pour schema_migrations";
        return false;
    }
    if (statement.step_row() != step_result::done) {
        error_out = "enregistrement de la migration " + name + " impossible";
        return false;
    }
    return true;
}

[[nodiscard]] bool read_file_contents(std::filesystem::path const &path,
                                      std::string &out)
{
    std::ifstream input{path, std::ios::binary};
    if (!input) {
        return false;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    out = buffer.str();
    return true;
}

[[nodiscard]] bool apply_one_migration(database_handle &database,
                                       migration_file const &file,
                                       std::string &error_out)
{
    std::string script;
    if (!read_file_contents(file.path, script)) {
        error_out = "lecture impossible : " + file.name;
        return false;
    }
    if (!database.execute_script("BEGIN IMMEDIATE", error_out)) {
        return false;
    }
    std::string ignored;
    if (!database.execute_script(script, error_out) ||
        !record_applied_migration(database, file.name, error_out)) {
        // All or nothing: a half-applied migration would leave a database
        // whose state nobody knows anymore.
        static_cast<void>(database.execute_script("ROLLBACK", ignored));
        // sqlite's raw message says which constraint tripped, never what
        // to do next. Pointing to the documentation saves the operator
        // from having to guess -- they're the one running the server, not
        // us.
        error_out = file.name + " : " + error_out +
                    "\n  La base n'a pas ete modifiee. Voir docs/ADMIN.md, "
                    "section « Migrations », pour resoudre puis relancer.";
        return false;
    }
    return database.execute_script("COMMIT", error_out);
}

} // namespace

bool apply_pending_migrations(database_handle &database,
                              std::string const &directory,
                              util::logger &logger, std::string &error_out)
{
    if (!database.execute_script(MIGRATIONS_TABLE_SQL, error_out)) {
        return false;
    }
    std::vector<migration_file> files;
    if (!collect_migration_files(directory, files, error_out)) {
        return false;
    }
    for (migration_file const &file : files) {
        bool applied = false;
        if (!is_already_applied(database, file.name, applied)) {
            error_out = "lecture de schema_migrations impossible";
            return false;
        }
        if (applied) {
            continue;
        }
        std::string const notice = "application de la migration " + file.name;
        logger.write_entry(util::log_level::info, notice);
        if (!apply_one_migration(database, file, error_out)) {
            return false;
        }
    }
    return true;
}

} // namespace hypercom::server
