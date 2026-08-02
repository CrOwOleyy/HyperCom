#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "server/db/sqlite_deleters.hpp"

namespace hypercom::server {

// La connexion SQLite, en mode WAL.
//
// Personne en dehors de server/db n'appelle sqlite3_* directement : les
// handlers metier passent tous par un depot. C'est ce qui garantit qu'aucune
// requete ne peut etre construite par concatenation de chaines quelque part
// dans la logique applicative.
class database_handle {
public:
    database_handle();

    // Ouvre la base et applique les PRAGMA de fonctionnement. Echoue avec un
    // message exploitable plutot que d'ouvrir une base a moitie configuree.
    [[nodiscard]] bool open_database(std::string const &path,
                                     std::string &error_out);

    // Reserve au DDL et aux PRAGMA, c'est-a-dire a du SQL entierement ecrit
    // par nous. Aucune donnee utilisateur ne passe jamais par ici.
    [[nodiscard]] bool execute_script(std::string_view sql,
                                      std::string &error_out);

    [[nodiscard]] sqlite3 *get_raw_handle() const;

    [[nodiscard]] std::int64_t get_last_insert_id() const;

private:
    database_pointer handle_;
};

} // namespace hypercom::server
