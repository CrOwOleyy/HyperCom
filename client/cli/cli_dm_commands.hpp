#pragma once

#include <string>
#include <vector>

#include "client/cli/cli_context.hpp"

namespace hypercom::client {

// A lancer une fois par identite : publie la prekey signee sans laquelle
// personne ne peut ouvrir de conversation chiffree avec vous.
[[nodiscard]] bool run_prekey_publish(cli_context &context,
                                      std::string &error_out);

[[nodiscard]] bool run_dm_send(cli_context &context,
                               std::vector<std::string> const &arguments,
                               std::string &error_out);

// Releve la boite, dechiffre localement, puis acquitte -- ce qui supprime les
// enveloppes du serveur.
[[nodiscard]] bool run_dm_fetch(cli_context &context, std::string &error_out);

} // namespace hypercom::client
