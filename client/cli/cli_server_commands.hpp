#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "client/cli/cli_options.hpp"

namespace hypercom::client {

// Commandes qui ne touchent QUE le registre local : elles n'ouvrent aucune
// connexion et ne revelent donc rien a personne. C'est pour ca qu'elles ne
// passent pas par cli_context, contrairement a toutes les autres.

[[nodiscard]] bool is_registry_command(std::string_view command);

[[nodiscard]] bool run_server_add(cli_options const &options,
                                  std::vector<std::string> const &arguments,
                                  std::string &error_out);

[[nodiscard]] bool run_server_list(cli_options const &options,
                                   std::string &error_out);

// Rattache une identite anterieure au multi-serveurs a un serveur du registre.
// Sans elle, les comptes crees avant ce changement seraient inaccessibles :
// leur cle a ete tiree au hasard, aucune graine ne peut la reproduire.
[[nodiscard]] bool run_server_import(cli_options const &options,
                                     std::vector<std::string> const &arguments,
                                     std::string &error_out);

} // namespace hypercom::client
