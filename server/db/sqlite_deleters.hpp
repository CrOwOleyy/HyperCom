#pragma once

#include <memory>

struct sqlite3;
struct sqlite3_stmt;

namespace hypercom::server {

// RAII pour les deux poignees sqlite. En passant par unique_ptr, les classes
// qui les detiennent n'ont besoin ni de destructeur, ni de constructeur de
// copie supprime : elles deviennent deplacables et non copiables par
// construction. Trois fonctions membres economisees par classe, et surtout
// aucune fuite possible sur un chemin d'erreur.
struct database_deleter {
    void operator()(sqlite3 *handle) const;
};

struct statement_deleter {
    void operator()(sqlite3_stmt *handle) const;
};

using database_pointer = std::unique_ptr<sqlite3, database_deleter>;
using statement_pointer = std::unique_ptr<sqlite3_stmt, statement_deleter>;

} // namespace hypercom::server
