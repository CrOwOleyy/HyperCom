#include "client/cli/cli_options.hpp"

#include <charconv>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string_view>

namespace hypercom::client {
namespace {

// Default values for Tor's local proxy, identical on all platforms (tor
// daemon and Tor Browser alike).
constexpr char const *DEFAULT_TOR_SOCKS_HOST = "127.0.0.1";
constexpr std::uint16_t DEFAULT_TOR_SOCKS_PORT = 9050;
constexpr char const *ONION_SUFFIX = ".onion";

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

[[nodiscard]] std::string trim_spaces(std::string_view text)
{
    std::size_t start = 0;
    while (start < text.size() && (text[start] == ' ' || text[start] == '\t')) {
        ++start;
    }
    std::size_t end = text.size();
    while (end > start && (text[end - 1] == ' ' || text[end - 1] == '\t' ||
                           text[end - 1] == '\r')) {
        --end;
    }
    return std::string{text.substr(start, end - start)};
}

[[nodiscard]] bool parse_proxy_endpoint(std::string const &text,
                                        cli_options &out,
                                        std::string &error_out)
{
    std::size_t const separator = text.rfind(':');
    if (separator == std::string::npos ||
        !parse_port(text.substr(separator + 1), out.socks5_port)) {
        error_out = "proxy SOCKS5 invalide, attendu hote:port : " + text;
        return false;
    }
    out.socks5_host = text.substr(0, separator);
    if (out.socks5_host.empty()) {
        error_out = "proxy SOCKS5 : hote manquant : " + text;
        return false;
    }
    return true;
}

// A line without '=' is ignored rather than rejected: a connect file can
// carry comments or blank lines without making the client fail. Only a
// recognized but invalid value (port) is a fatal error.
[[nodiscard]] bool apply_connect_line(std::string_view line, cli_options &out,
                                      std::string &error_out)
{
    std::size_t const separator = line.find('=');
    if (separator == std::string_view::npos) {
        return true;
    }
    std::string const key = trim_spaces(line.substr(0, separator));
    std::string const value = trim_spaces(line.substr(separator + 1));
    if (key == "host") {
        out.host = value;
    } else if (key == "port") {
        if (!parse_port(value, out.port)) {
            error_out = "fichier de connexion : port invalide : " + value;
            return false;
        }
    } else if (key == "server_key") {
        out.server_key_hex = value;
    } else if (key == "socks5") {
        if (!parse_proxy_endpoint(value, out, error_out)) {
            return false;
        }
    }
    return true;
}

// Applied immediately at the point where --connect-file appears in the
// arguments: a --host/--port/--server-key placed AFTER it on the same
// command line overwrites the file's value, and a --connect-file placed
// after those overwrites them in turn. Left-to-right order, no hidden
// state.
[[nodiscard]] bool load_connect_file(std::string const &path, cli_options &out,
                                     std::string &error_out)
{
    std::ifstream file{path};
    if (!file.is_open()) {
        error_out = "fichier de connexion introuvable : " + path;
        return false;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (!apply_connect_line(line, out, error_out)) {
            return false;
        }
    }
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
           "  --connect-file <chemin> fichier host+port+server-key partage\n"
           "                          par l'administrateur (ecrase par tout\n"
           "                          flag place apres sur la ligne)\n"
           "  --identity <chemin>    fichier de cle privee chiffree\n"
           "  --server <nom>         serveur du registre a viser\n"
           "  --master-seed <chemin> graine dont derivent les identites\n"
           "  --servers <chemin>     registre chiffre des serveurs\n"
           "  --tor                  passe par Tor (127.0.0.1:9050). "
           "Implicite\n"
           "                          si --host se termine par .onion\n"
           "  --socks5 <hote:port>   proxy SOCKS5 autre que celui par defaut\n"
           "  --direct               force la connexion directe, sans proxy\n"
           "  --replay-intro         (client graphique) rejoue l'accueil\n\n"
           "Commandes de registre (aucune connexion) :\n"
           "  server-add <hypercom://hote:port#cle> [nom]\n"
           "  server-list\n"
           "  server-import <nom> <chemin_identite>\n\n"
           "Commandes :\n"
           "  register <pseudo>\n"
           "  whoami\n"
           "  forum-create <nom> <description>\n"
           "  forum-list\n"
           "  post <forum_id> <titre> <corps>\n"
           "  posts <forum_id>\n"
           "  thread <post_id>\n"
           "  comment <post_id> <parent_id|0> <corps>\n"
           "  post-delete <post_id>        retire SON propre post\n"
           "  comment-delete <comment_id>  retire SON propre commentaire\n"
           "  profile-set <nom_affiche> <bio>\n"
           "  profile-get <pubkey_hex>\n"
           "  friend-add <pubkey_hex>\n"
           "  friends\n"
           "  prekey-publish\n"
           "  dm-send <pubkey_hex> <texte>\n"
           "  dm-fetch\n"
           "  report-post <post_id> [motif]\n"
           "  report-account <pubkey_hex> [motif]\n\n"
           "La passphrase se lit dans HYPERCOM_PASSPHRASE, ou est demandee.\n";
}

bool parse_cli_options(int argc, char **argv, cli_options &out,
                       std::string &error_out, bool require_command,
                       bool require_server_key)
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
            if (!take_value(argc, argv, index, out.server_key_hex, error_out)) {
                return false;
            }
        } else if (argument == "--identity") {
            if (!take_value(argc, argv, index, out.identity_path, error_out)) {
                return false;
            }
        } else if (argument == "--server") {
            if (!take_value(argc, argv, index, out.server_label, error_out)) {
                return false;
            }
        } else if (argument == "--master-seed") {
            if (!take_value(argc, argv, index, out.master_seed_path,
                            error_out)) {
                return false;
            }
        } else if (argument == "--servers") {
            if (!take_value(argc, argv, index, out.registry_path, error_out)) {
                return false;
            }
        } else if (argument == "--connect-file") {
            std::string path;
            if (!take_value(argc, argv, index, path, error_out)) {
                return false;
            }
            if (!load_connect_file(path, out, error_out)) {
                return false;
            }
        } else if (argument == "--tor") {
            out.socks5_host = DEFAULT_TOR_SOCKS_HOST;
            out.socks5_port = DEFAULT_TOR_SOCKS_PORT;
        } else if (argument == "--socks5") {
            std::string endpoint;
            if (!take_value(argc, argv, index, endpoint, error_out) ||
                !parse_proxy_endpoint(endpoint, out, error_out)) {
                return false;
            }
        } else if (argument == "--direct") {
            // Explicit disable: useful when the connect file forces Tor
            // but you want to reach the server in the clear.
            out.socks5_host.clear();
            out.socks5_port = 0;
        } else if (argument == "--replay-intro") {
            out.replay_intro = true;
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
    // A .onion address doesn't exist in DNS: without a proxy, resolution
    // would fail with a baffling "host not found". So we switch to the
    // default Tor proxy. This is what makes Tor active with no
    // configuration beyond the address itself -- and --direct lets you
    // opt out.
    if (out.socks5_host.empty() && out.host.ends_with(ONION_SUFFIX)) {
        out.socks5_host = DEFAULT_TOR_SOCKS_HOST;
        out.socks5_port = DEFAULT_TOR_SOCKS_PORT;
    }
    // Two cases don't need an explicit key: a registry command doesn't
    // connect to anything, and --server resolves the host and key from
    // the registry.
    if (require_server_key && out.server_label.empty() &&
        !out.command.starts_with("server-") &&
        out.server_key_hex.size() != 64) {
        error_out = "--server-key est requis : 64 caracteres hexadecimaux, "
                    "affiches par le serveur a son demarrage. Sinon, --server "
                    "<nom> pour viser un serveur deja enregistre";
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
    // No fallback value here. A hardcoded passphrase would end up in
    // every distributed binary, so in everyone's hands: the Argon2id
    // keystore would no longer protect anything. The graphical client has
    // no console and will fail here -- that's intentional, it goes
    // through HYPERCOM_PASSPHRASE and report_startup_failure says so.
    std::cout << "passphrase : " << std::flush;
    if (!std::getline(std::cin, out) || out.empty()) {
        error_out = "passphrase requise";
        return false;
    }
    return true;
}

} // namespace hypercom::client
