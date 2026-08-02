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
//
// problems et warnings sont bien deux choses distinctes : les premiers
// empechent le demarrage, les seconds sont affiches puis le serveur demarre.
// Les confondre rendrait inutilisable un reglage pourtant legitime -- activer
// la journalisation des IP doit etre bruyant, pas interdit.
[[nodiscard]] bool validate_config(server_config const &config,
                                   std::vector<std::string> &problems,
                                   std::vector<std::string> &warnings);

} // namespace hypercom::server
