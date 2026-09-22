#include "server/db/database_handle.hpp"

#include <sqlite3.h>

namespace hypercom::server {
namespace {

// WAL : lectures concurrentes sans bloquer l'ecrivain.
// foreign_keys : sqlite les desactive par defaut, ce qui rendrait les
//   contraintes du schema purement decoratives.
// busy_timeout : sans lui, la moindre contention rend SQLITE_BUSY
//   immediatement au lieu d'attendre.
// synchronous NORMAL : le bon compromis en WAL, une coupure de courant peut
//   couter la derniere transaction, jamais l'integrite du fichier.
// secure_delete : sans lui, sqlite laisse le contenu efface intact dans les
//   pages liberees. Un post retire par son auteur resterait donc lisible en
//   clair dans le fichier de base, et dans chaque sauvegarde -- ce qui ne
//   serait pas une suppression, seulement une disparition de l'affichage.
//   Le cout est un peu d'ecriture supplementaire, a une echelle ou ca ne se
//   mesure pas.
constexpr char const *STARTUP_PRAGMAS =
    "PRAGMA journal_mode=WAL;"
    "PRAGMA foreign_keys=ON;"
    "PRAGMA busy_timeout=5000;"
    "PRAGMA synchronous=NORMAL;"
    "PRAGMA secure_delete=ON;"
    "PRAGMA temp_store=MEMORY;";

} // namespace

database_handle::database_handle() : handle_{nullptr} {}

bool database_handle::open_database(std::string const &path,
                                    std::string &error_out)
{
    sqlite3 *raw = nullptr;
    int const status = sqlite3_open_v2(
        path.c_str(), &raw, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    handle_.reset(raw);
    if (status != SQLITE_OK) {
        error_out = "ouverture de " + path + " : "
                    + (raw != nullptr ? sqlite3_errmsg(raw)
                                      : sqlite3_errstr(status));
        handle_.reset();
        return false;
    }
    return execute_script(STARTUP_PRAGMAS, error_out);
}

bool database_handle::execute_script(std::string_view sql,
                                     std::string &error_out)
{
    if (handle_ == nullptr) {
        error_out = "base non ouverte";
        return false;
    }
    char *raw_message = nullptr;
    std::string const script{sql};
    int const status = sqlite3_exec(handle_.get(), script.c_str(), nullptr,
                                    nullptr, &raw_message);
    if (status != SQLITE_OK) {
        error_out = raw_message != nullptr ? raw_message
                                           : sqlite3_errstr(status);
        sqlite3_free(raw_message);
        return false;
    }
    return true;
}

sqlite3 *database_handle::get_raw_handle() const
{
    return handle_.get();
}

std::int64_t database_handle::get_last_insert_id() const
{
    if (handle_ == nullptr) {
        return 0;
    }
    return static_cast<std::int64_t>(sqlite3_last_insert_rowid(handle_.get()));
}

int database_handle::get_changed_row_count() const
{
    if (handle_ == nullptr) {
        return 0;
    }
    return sqlite3_changes(handle_.get());
}

} // namespace hypercom::server
