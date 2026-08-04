#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace hypercom::client {

struct cli_options {
    std::string host = "127.0.0.1";
    std::uint16_t port = 7717;
    // Vide par defaut, et c'est essentiel : epingler une cle par defaut ferait
    // silencieusement confiance au serveur de celui qui a compile le binaire.
    // La validation exige --server-key, ce qui force un choix explicite.
    std::string server_key_hex;
    std::string identity_path = "hypercom_identity.key";
    std::string command;
    std::vector<std::string> arguments;
    // Client graphique uniquement : rejoue la sequence d'accueil sur un compte
    // deja existant. Sert a regler l'animation sans creer un compte jetable a
    // chaque essai. Le client CLI ignore ce drapeau.
    bool replay_intro = false;
};

[[nodiscard]] bool parse_cli_options(int argc, char **argv, cli_options &out,
                                     std::string &error_out,
                                     bool require_command = true);

void print_cli_usage();

// Lue depuis HYPERCOM_PASSPHRASE si la variable existe, sinon demandee sur
// l'entree standard. Elle n'est jamais acceptee en argument de ligne de
// commande : un argument est visible dans ps et fini dans l'historique du
// shell.
[[nodiscard]] bool read_passphrase(std::string &out, std::string &error_out);

} // namespace hypercom::client
