#include "client/cli/cli_options.hpp"

#include <charconv>
#include <cstdlib>
#include <iostream>

namespace hypercom::client {
namespace {

[[nodiscard]] bool parse_port(std::string const &text, std::uint16_t &out)
{
    std::uint32_t value = 0;
    auto const result =
        std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || value == 0 || value > 65535) {
        return false;
    }
    out = static_cast<std::uint16_t>(value);
    return true;
}

[[nodiscard]] bool take_value(int argc, char **argv, int &index,
                              std::string &out, std::string &error_out)
{
    if (index + 1 >= argc) {
        error_out = std::string{"valeur manquante apres "} + argv[index];
        return false;
    }
    ++index;
    out = argv[index];
    return true;
}

} // namespace

void print_cli_usage()
{
    std::cout
        << "hypercom_cli -- client en ligne de commande\n\n"
           "  --host <adresse>       defaut 127.0.0.1\n"
           "  --port <port>          defaut 7717\n"
           "  --server-key <hex>     cle statique du serveur, 64 caracteres\n"
           "  --identity <chemin>    fichier de cle privee chiffree\n\n"
           "Commandes :\n"
           "  register <pseudo>\n"
           "  whoami\n"
           "  forum-create <nom> <description>\n"
           "  forum-list\n"
           "  post <forum_id> <titre> <corps>\n"
           "  posts <forum_id>\n"
           "  thread <post_id>\n"
           "  comment <post_id> <parent_id|0> <corps>\n"
           "  profile-set <nom_affiche> <bio>\n"
           "  profile-get <pubkey_hex>\n"
           "  friend-add <pubkey_hex>\n"
           "  friends\n"
           "  prekey-publish\n"
           "  dm-send <pubkey_hex> <texte>\n"
           "  dm-fetch\n\n"
           "La passphrase se lit dans HYPERCOM_PASSPHRASE, ou est demandee.\n";
}

bool parse_cli_options(int argc, char **argv, cli_options &out,
                       std::string &error_out, bool require_command)
{
    for (int index = 1; index < argc; ++index) {
        std::string const argument{argv[index]};
        if (argument == "--host") {
            if (!take_value(argc, argv, index, out.host, error_out)) {
                return false;
            }
        } else if (argument == "--port") {
            std::string port_text;
            if (!take_value(argc, argv, index, port_text, error_out)) {
                return false;
            }
            if (!parse_port(port_text, out.port)) {
                error_out = "port invalide : " + port_text;
                return false;
            }
        } else if (argument == "--server-key") {
            if (!take_value(argc, argv, index, out.server_key_hex,
                            error_out)) {
                return false;
            }
        } else if (argument == "--identity") {
            if (!take_value(argc, argv, index, out.identity_path,
                            error_out)) {
                return false;
            }
        } else if (out.command.empty()) {
            out.command = argument;
        } else {
            out.arguments.push_back(argument);
        }
    }
    if (require_command && out.command.empty()) {
        error_out = "aucune commande fournie";
        return false;
    }
    if (out.server_key_hex.size() != 64) {
        error_out = "--server-key est requis : 64 caracteres hexadecimaux, "
                    "affiches par le serveur a son demarrage";
        return false;
    }
    return true;
}

bool read_passphrase(std::string &out, std::string &error_out)
{
    char const *const from_environment = std::getenv("HYPERCOM_PASSPHRASE");
    if (from_environment != nullptr) {
        out = from_environment;
        return true;
    }
    std::cout << "passphrase : " << std::flush;
    if (!std::getline(std::cin, out) || out.empty()) {
        error_out = "passphrase requise";
        return false;
    }
    return true;
}

} // namespace hypercom::client
