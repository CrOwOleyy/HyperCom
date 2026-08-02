#pragma once

#include <string>

#include "server/db/database_handle.hpp"

namespace hypercom::server {

// Sauvegarde a chaud, sans arreter le serveur.
//
// Passe par l'API de sauvegarde de SQLite, pas par une copie de fichier. La
// difference n'est pas cosmetique : en mode WAL, une partie des donnees vit
// dans le journal, et un `cp` pendant une ecriture produit un fichier
// incoherent. L'API, elle, prend un instantane coherent.
[[nodiscard]] bool backup_database_to(database_handle &source,
                                      std::string const &destination_path,
                                      std::string &error_out);

} // namespace hypercom::server
