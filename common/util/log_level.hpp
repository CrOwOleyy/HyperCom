#pragma once

#include <string_view>

namespace hypercom::util {

enum class log_level : unsigned char {
    debug = 0,
    info = 1,
    warning = 2,
    error = 3,
    silent = 4,
};

// Rend le niveau nomme dans hypercom.conf. Renvoie false sur un nom inconnu :
// le serveur refuse alors de demarrer plutot que de retomber sur une valeur
// par defaut silencieuse.
[[nodiscard]] bool parse_log_level(std::string_view name, log_level &out);

[[nodiscard]] std::string_view describe_log_level(log_level level);

} // namespace hypercom::util
