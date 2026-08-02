#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "server/config/server_config.hpp"

namespace hypercom::server {

// Lecture de hypercom.conf. Format minimal :
//
//   # commentaire
//   [section]
//   cle = valeur
//
// Pas de JSON, pas de YAML, pas de TOML : un format a une ligne par reglage
// n'a pas de parseur a auditer, et le fichier est modifie a la main par un
// humain, pas genere par une machine.
//
// Toute erreur est ACCUMULEE plutot que remontee a la premiere : un
// administrateur qui a fait trois fautes de frappe veut les trois d'un coup,
// pas trois redemarrages.
struct config_error {
    std::size_t line_number = 0;
    std::string message;
};

[[nodiscard]] bool parse_config_text(std::string_view text,
                                     server_config &out,
                                     std::vector<config_error> &errors);

[[nodiscard]] bool parse_config_file(std::string const &path,
                                     server_config &out,
                                     std::vector<config_error> &errors);

// Rend les erreurs sous une forme directement affichable sur stderr.
[[nodiscard]] std::string format_config_errors(
    std::string const &path, std::vector<config_error> const &errors);

} // namespace hypercom::server
