#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace hypercom::client {

struct cli_options {
    std::string host = "127.0.0.1";
    std::uint16_t port = 7717;
    // Empty by default, and that's essential: pinning a default key would
    // silently trust whoever compiled the binary's server. Validation
    // requires --server-key, which forces an explicit choice.
    std::string server_key_hex;
    // SOCKS5 proxy to go through. Empty = direct connection. It's the
    // only way to reach a .onion, which DNS doesn't know about. Filled in
    // automatically when the host ends in .onion, so Tor activates
    // without asking for anything more than the address itself.
    std::string socks5_host;
    std::uint16_t socks5_port = 0;
    std::string identity_path = "hypercom_identity.key";
    // Registry server to target. Empty = direct mode, where the host,
    // port, and key are given explicitly -- which is still what the
    // graphical client does.
    std::string server_label;
    // The seed all identities derive from, and the list of servers.
    // Relative paths by default, like identity_path: the binary never
    // goes looking for a configuration directory on its own.
    std::string master_seed_path = "hypercom_master.key";
    std::string registry_path = "hypercom_servers.dat";
    std::string command;
    std::vector<std::string> arguments;
    // Graphical client only: replays the welcome sequence on an
    // already-existing account. Used to tune the animation without
    // creating a throwaway account on every attempt. The CLI client
    // ignores this flag.
    bool replay_intro = false;
};

// require_server_key: the command line client acts immediately and so
// needs a server as soon as parsing happens. The graphical client, on the
// other hand, resolves its servers from the registry afterward and
// passes false.
[[nodiscard]] bool parse_cli_options(int argc, char **argv, cli_options &out,
                                     std::string &error_out,
                                     bool require_command = true,
                                     bool require_server_key = true);

void print_cli_usage();

// Read from HYPERCOM_PASSPHRASE if the variable exists, otherwise
// prompted on standard input. It's never accepted as a command line
// argument: an argument is visible in ps and ends up in the shell
// history.
[[nodiscard]] bool read_passphrase(std::string &out, std::string &error_out);

} // namespace hypercom::client
