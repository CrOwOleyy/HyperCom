#pragma once

#include <string_view>

#include "server/db/database_handle.hpp"

namespace hypercom::server {

enum class step_result {
    row,
    done,
    failed,
};

// Requete preparee, exclusivement.
//
// Il n'existe aucun moyen de construire une requete a partir
// d'une chaine assemblee : le seul constructeur prend un litteral SQL et les
// valeurs arrivent ensuite par sql_binder. Une injection demanderait de
// modifier ce fichier, pas d'oublier un echappement.
class sql_statement {
public:
    sql_statement(database_handle &database, std::string_view sql);

    [[nodiscard]] step_result step_row();

    // Remet la requete a zero pour un nouveau jeu de parametres, ce qui evite
    // de re-compiler le SQL a chaque iteration.
    [[nodiscard]] bool reset_for_reuse();

    [[nodiscard]] sqlite3_stmt *get_raw_handle() const;

private:
    statement_pointer statement_;
};

} // namespace hypercom::server
