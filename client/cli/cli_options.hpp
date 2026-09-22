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
    // Proxy SOCKS5 a traverser. Vide = connexion directe. C'est le seul moyen
    // d'atteindre un .onion, que le DNS ne connait pas. Rempli automatiquement
    // quand l'hote se termine par .onion, donc Tor s'active sans rien demander
    // de plus que l'adresse elle-meme.
    std::string socks5_host;
    std::uint16_t socks5_port = 0;
    std::string identity_path = "hypercom_identity.key";
    // Serveur du registre a viser. Vide = mode direct, ou l'hote, le port et la
    // cle sont donnes explicitement -- ce que fait encore le client graphique.
    std::string server_label;
    // La graine dont derivent toutes les identites, et la liste des serveurs.
    // Chemins relatifs par defaut, comme identity_path : le binaire ne va
    // jamais chercher un repertoire de configuration tout seul.
    std::string master_seed_path = "hypercom_master.key";
    std::string registry_path = "hypercom_servers.dat";
    std::string command;
    std::vector<std::string> arguments;
    // Client graphique uniquement : rejoue la sequence d'accueil sur un compte
    // deja existant. Sert a regler l'animation sans creer un compte jetable a
    // chaque essai. Le client CLI ignore ce drapeau.
    bool replay_intro = false;
};

// require_server_key : le client en ligne de commande agit immediatement et a
// donc besoin d'un serveur des l'analyse. Le client graphique, lui, resout ses
// serveurs depuis le registre apres coup et passe false.
[[nodiscard]] bool parse_cli_options(int argc, char **argv, cli_options &out,
                                     std::string &error_out,
                                     bool require_command = true,
                                     bool require_server_key = true);

void print_cli_usage();

// Lue depuis HYPERCOM_PASSPHRASE si la variable existe, sinon demandee sur
// l'entree standard. Elle n'est jamais acceptee en argument de ligne de
// commande : un argument est visible dans ps et fini dans l'historique du
// shell.
[[nodiscard]] bool read_passphrase(std::string &out, std::string &error_out);

} // namespace hypercom::client
