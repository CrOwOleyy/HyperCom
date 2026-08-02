#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace hypercom::client {

struct cli_options {
    std::string host = "127.0.0.1";
    std::uint16_t port = 7717;
    std::string server_key_hex =
        "REDACTED-SERVER-KEY";
    std::string identity_path = "hypercom_identity.key";
    std::string command;
    std::vector<std::string> arguments;
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
