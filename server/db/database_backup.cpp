#include "server/db/database_backup.hpp"

#include <sqlite3.h>

namespace hypercom::server {

bool backup_database_to(database_handle &source,
                        std::string const &destination_path,
                        std::string &error_out)
{
    sqlite3 *destination = nullptr;
    if (sqlite3_open(destination_path.c_str(), &destination) != SQLITE_OK) {
        error_out = "ouverture impossible de " + destination_path;
        sqlite3_close(destination);
        return false;
    }
    sqlite3_backup *const operation = sqlite3_backup_init(
        destination, "main", source.get_raw_handle(), "main");
    if (operation == nullptr) {
        error_out = "sauvegarde refusee par sqlite : " +
                    std::string{sqlite3_errmsg(destination)};
        sqlite3_close(destination);
        return false;
    }
    // -1: copy everything in one go. At this scale the database fits in a
    // few megabytes, breaking it into chunks wouldn't help.
    int const step_status = sqlite3_backup_step(operation, -1);
    sqlite3_backup_finish(operation);
    bool const succeeded = step_status == SQLITE_DONE;
    if (!succeeded) {
        error_out =
            "copie incomplete : " + std::string{sqlite3_errmsg(destination)};
    }
    sqlite3_close(destination);
    return succeeded;
}

} // namespace hypercom::server
