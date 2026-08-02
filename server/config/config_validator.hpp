#pragma once

#include <string>
#include <vector>

#include "server/config/server_config.hpp"

namespace hypercom::server {

// Coherence d'ensemble, apres que chaque valeur a ete lue individuellement.
//
// Le serveur REFUSE de demarrer sur une configuration invalide plutot que de
// partir avec des valeurs par defaut silencieuses. Un serveur qui
// se rabat tout seul sur un port ou une politique de logs par defaut est un
// serveur qui, un jour, journalisera des IP sans que personne l'ait decide.
[[nodiscard]] bool validate_config(server_config const &config,
                                   std::vector<std::string> &problems);

} // namespace hypercom::server
