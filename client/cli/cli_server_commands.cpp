#include "client/cli/cli_server_commands.hpp"

#include "client/cli/invite_link.hpp"
#include "client/keystore/server_registry.hpp"
#include "common/util/hex_codec.hpp"

#include <algorithm>
#include <iostream>

namespace hypercom::client {
namespace {

// A missing registry is not an error: it's the normal state before
// adding your first server.
[[nodiscard]] bool load_existing(server_registry &registry,
                                 std::string_view passphrase,
                                 std::vector<server_entry> &out,
                                 std::string &error_out)
{
    if (!registry.has_stored_registry()) {
        out.clear();
        return true;
    }
    return registry.load(passphrase, out, error_out);
}

[[nodiscard]] server_entry *find_by_label(std::vector<server_entry> &entries,
                                          std::string_view label)
{
    auto const found = std::find_if(
        entries.begin(), entries.end(),
        [label](server_entry const &entry) { return entry.label == label; });
    return found == entries.end() ? nullptr : &*found;
}

[[nodiscard]] bool decode_key(std::string const &hex,
                              crypto::x25519_public_key &out)
{
    std::vector<std::uint8_t> decoded;
    if (!util::decode_hex(hex, decoded) || decoded.size() != out.size()) {
        return false;
    }
    std::copy(decoded.begin(), decoded.end(), out.begin());
    return true;
}

} // namespace

bool is_registry_command(std::string_view command)
{
    return command == "server-add" || command == "server-list" ||
           command == "server-import";
}

bool run_server_add(cli_options const &options,
                    std::vector<std::string> const &arguments,
                    std::string &error_out)
{
    if (arguments.empty()) {
        error_out = "usage : server-add <hypercom://hote:port#cle> [nom]";
        return false;
    }
    invite_link link;
    if (!parse_invite_link(arguments[0], link, error_out)) {
        return false;
    }
    server_entry entry;
    entry.label = arguments.size() > 1 ? arguments[1] : link.host;
    entry.endpoint = {link.host, link.port, options.socks5_host,
                      options.socks5_port};
    if (!decode_key(link.server_key_hex, entry.server_key)) {
        error_out = "cle du serveur illisible";
        return false;
    }
    std::string passphrase;
    if (!read_passphrase(passphrase, error_out)) {
        return false;
    }
    server_registry registry{options.registry_path};
    std::vector<server_entry> entries;
    if (!load_existing(registry, passphrase, entries, error_out)) {
        return false;
    }
    if (find_by_label(entries, entry.label) != nullptr) {
        error_out = "un serveur porte deja le nom " + entry.label;
        return false;
    }
    entries.push_back(std::move(entry));
    if (!registry.save(passphrase, entries, error_out)) {
        return false;
    }
    std::cout << "serveur enregistre : " << entries.back().label << '\n';
    return true;
}

bool run_server_list(cli_options const &options, std::string &error_out)
{
    std::string passphrase;
    if (!read_passphrase(passphrase, error_out)) {
        return false;
    }
    server_registry registry{options.registry_path};
    std::vector<server_entry> entries;
    if (!load_existing(registry, passphrase, entries, error_out)) {
        return false;
    }
    std::cout << entries.size() << " serveur(s) :\n";
    for (server_entry const &entry : entries) {
        std::string encoded;
        util::encode_hex(entry.server_key, encoded);
        std::cout << "  " << entry.label << "  " << entry.endpoint.host << ':'
                  << entry.endpoint.port << "  " << encoded.substr(0, 16)
                  << "  identite "
                  << (entry.source == identity_source::imported ? "importee"
                                                                : "derivee")
                  << '\n';
    }
    return true;
}

bool run_server_import(cli_options const &options,
                       std::vector<std::string> const &arguments,
                       std::string &error_out)
{
    if (arguments.size() < 2) {
        error_out = "usage : server-import <nom> <chemin_identite>";
        return false;
    }
    std::string passphrase;
    if (!read_passphrase(passphrase, error_out)) {
        return false;
    }
    server_registry registry{options.registry_path};
    std::vector<server_entry> entries;
    if (!load_existing(registry, passphrase, entries, error_out)) {
        return false;
    }
    server_entry *const target = find_by_label(entries, arguments[0]);
    if (target == nullptr) {
        error_out = "serveur inconnu : " + arguments[0];
        return false;
    }
    target->source = identity_source::imported;
    target->imported_identity_path = arguments[1];
    if (!registry.save(passphrase, entries, error_out)) {
        return false;
    }
    std::cout << "identite rattachee a " << target->label << '\n';
    return true;
}

} // namespace hypercom::client
