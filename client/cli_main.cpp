#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "client/cli/cli_content_commands.hpp"
#include "client/cli/cli_content_delete_commands.hpp"
#include "client/cli/cli_dm_commands.hpp"
#include "client/cli/cli_forum_commands.hpp"
#include "client/cli/cli_options.hpp"
#include "client/cli/cli_report_commands.hpp"
#include "client/cli/cli_server_commands.hpp"
#include "client/cli/cli_social_commands.hpp"
#include "client/cli/cli_top8_commands.hpp"
#include "client/keystore/identity_store.hpp"
#include "client/keystore/master_seed_store.hpp"
#include "client/keystore/server_identity.hpp"
#include "client/keystore/server_registry.hpp"
#include "common/crypto/secure_memory.hpp"
#include "common/crypto/sodium_runtime.hpp"
#include "common/util/hex_codec.hpp"

namespace {

using namespace hypercom;

[[nodiscard]] bool decode_server_key(std::string const &text,
                                     crypto::x25519_public_key &out,
                                     std::string &error_out)
{
    std::vector<std::uint8_t> decoded;
    if (!util::decode_hex(text, decoded) || decoded.size() != out.size()) {
        error_out = "--server-key doit faire 64 caracteres hexadecimaux";
        return false;
    }
    std::copy(decoded.begin(), decoded.end(), out.begin());
    return true;
}

// Sans --server, on reste en mode direct : hote, port et cle donnes a la main,
// identite lue dans --identity. C'est ce que fait encore le client graphique,
// et ce que font les scripts existants.
[[nodiscard]] bool resolve_server(client::cli_options const &options,
                                  std::string_view passphrase,
                                  client::server_entry &out,
                                  std::string &error_out)
{
    if (options.server_label.empty()) {
        out.label = options.host;
        out.endpoint = {options.host, options.port, options.socks5_host,
                        options.socks5_port};
        out.source = client::identity_source::imported;
        out.imported_identity_path = options.identity_path;
        return decode_server_key(options.server_key_hex, out.server_key,
                                 error_out);
    }
    client::server_registry registry{options.registry_path};
    std::vector<client::server_entry> entries;
    if (!registry.load(passphrase, entries, error_out)) {
        return false;
    }
    for (client::server_entry const &entry : entries) {
        if (entry.label == options.server_label) {
            out = entry;
            return true;
        }
    }
    error_out = "serveur inconnu dans le registre : " + options.server_label;
    return false;
}

// Une identite importee vient de son propre fichier ; une identite derivee se
// recalcule depuis la graine maitresse et la cle du serveur, sans rien stocker.
[[nodiscard]] bool resolve_identity(client::cli_options const &options,
                                    client::server_entry const &entry,
                                    std::string_view passphrase,
                                    crypto::identity_keypair &out,
                                    std::string &error_out)
{
    if (entry.source == client::identity_source::imported) {
        client::identity_store store{entry.imported_identity_path};
        if (store.has_stored_identity()) {
            return store.unlock_identity(passphrase, out, error_out);
        }
        std::cout << "aucune identite dans " << entry.imported_identity_path
                  << ", creation d'une nouvelle paire de cles.\n"
                     "ATTENTION : cette cle EST le compte. Perdue, elle ne se "
                     "recupere pas, et personne ne peut la reinitialiser.\n";
        return store.create_identity(passphrase, out, error_out);
    }
    client::master_seed_store seeds{options.master_seed_path};
    crypto::ed25519_seed seed{};
    bool ready = seeds.has_stored_seed()
                     ? seeds.unlock_seed(passphrase, seed, error_out)
                     : seeds.create_seed(passphrase, seed, error_out);
    if (ready) {
        ready = client::derive_server_identity(seed, entry.server_key, out);
        if (!ready) {
            error_out = "derivation de l'identite impossible";
        }
    }
    crypto::wipe_bytes(seed);
    return ready;
}

[[nodiscard]] bool dispatch_command(client::cli_context &context,
                                    client::cli_options const &options,
                                    std::string &error_out)
{
    std::string const &command = options.command;
    std::vector<std::string> const &arguments = options.arguments;
    if (command == "whoami") {
        std::string encoded;
        util::encode_hex(context.identity.get_public_key(), encoded);
        std::cout << "@" << context.session.get_handle() << '\n'
                  << encoded << '\n';
        return true;
    }
    if (command == "forum-create") {
        return client::run_forum_create(context, arguments, error_out);
    }
    if (command == "forum-list") {
        return client::run_forum_list(context, error_out);
    }
    if (command == "post") {
        return client::run_post_create(context, arguments, error_out);
    }
    if (command == "posts") {
        return client::run_post_list(context, arguments, error_out);
    }
    if (command == "thread") {
        return client::run_thread_fetch(context, arguments, error_out);
    }
    if (command == "comment") {
        return client::run_comment_create(context, arguments, error_out);
    }
    if (command == "post-delete") {
        return client::run_post_delete(context, arguments, error_out);
    }
    if (command == "comment-delete") {
        return client::run_comment_delete(context, arguments, error_out);
    }
    if (command == "profile-set") {
        return client::run_profile_set(context, arguments, error_out);
    }
    if (command == "profile-get") {
        return client::run_profile_get(context, arguments, error_out);
    }
    if (command == "friend-add") {
        return client::run_friend_add(context, arguments, error_out);
    }
    if (command == "friends") {
        return client::run_friend_list(context, error_out);
    }
    if (command == "prekey-publish") {
        return client::run_prekey_publish(context, error_out);
    }
    if (command == "top8-set") {
        return client::run_top8_set(context, arguments, error_out);
    }
    if (command == "top8-get") {
        return client::run_top8_get(context, arguments, error_out);
    }
    if (command == "dm-send") {
        return client::run_dm_send(context, arguments, error_out);
    }
    if (command == "dm-fetch") {
        return client::run_dm_fetch(context, error_out);
    }
    if (command == "report-post") {
        return client::run_report_post(context, arguments, error_out);
    }
    if (command == "report-account") {
        return client::run_report_account(context, arguments, error_out);
    }
    error_out = "commande inconnue : " + command;
    return false;
}

// Etablit le canal, authentifie, enregistre si besoin, puis execute.
[[nodiscard]] bool run_cli(client::cli_options const &options,
                           std::string &error_out)
{
    std::string passphrase;
    if (!client::read_passphrase(passphrase, error_out)) {
        return false;
    }
    client::server_entry entry;
    crypto::identity_keypair identity;
    if (!resolve_server(options, passphrase, entry, error_out)
        || !resolve_identity(options, entry, passphrase, identity,
                             error_out)) {
        return false;
    }
    client::server_connection connection{entry.server_key};
    if (!connection.open_session(entry.endpoint, error_out)) {
        return false;
    }
    client::client_session session{connection, identity};
    if (!session.authenticate(error_out)) {
        return false;
    }
    if (options.command == "register") {
        if (options.arguments.empty()) {
            error_out = "usage : register <pseudo>";
            return false;
        }
        if (!session.register_handle(options.arguments[0], error_out)) {
            return false;
        }
        std::cout << "compte cree : @" << session.get_handle() << '\n';
        return true;
    }
    if (session.needs_registration()) {
        error_out = "cette cle n'a pas encore de compte : lancer d'abord "
                    "'register <pseudo>'";
        return false;
    }
    client::cli_context context{connection, identity, session};
    return dispatch_command(context, options, error_out);
}

} // namespace

int main(int argc, char **argv)
{
    if (!crypto::initialize_sodium()) {
        std::cerr << "libsodium n'a pas pu s'initialiser\n";
        return 1;
    }
    client::cli_options options;
    std::string failure;
    if (!client::parse_cli_options(argc, argv, options, failure)) {
        std::cerr << failure << "\n\n";
        client::print_cli_usage();
        return 2;
    }
    // Les commandes de registre n'ouvrent aucune connexion : elles se traitent
    // avant tout ce qui touche au reseau.
    if (client::is_registry_command(options.command)) {
        bool const done =
            options.command == "server-add"
                ? client::run_server_add(options, options.arguments, failure)
            : options.command == "server-list"
                ? client::run_server_list(options, failure)
                : client::run_server_import(options, options.arguments,
                                            failure);
        if (!done) {
            std::cerr << failure << '\n';
            return 3;
        }
        return 0;
    }
    if (!run_cli(options, failure)) {
        std::cerr << failure << '\n';
        return 4;
    }
    return 0;
}
