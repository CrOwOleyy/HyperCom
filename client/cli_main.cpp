#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "client/cli/cli_content_commands.hpp"
#include "client/cli/cli_dm_commands.hpp"
#include "client/cli/cli_forum_commands.hpp"
#include "client/cli/cli_options.hpp"
#include "client/cli/cli_social_commands.hpp"
#include "client/keystore/identity_store.hpp"
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

[[nodiscard]] bool load_identity(client::cli_options const &options,
                                 crypto::identity_keypair &out,
                                 std::string &error_out)
{
    client::identity_store store{options.identity_path};
    std::string passphrase;
    if (!client::read_passphrase(passphrase, error_out)) {
        return false;
    }
    if (store.has_stored_identity()) {
        return store.unlock_identity(passphrase, out, error_out);
    }
    std::cout << "aucune identite dans " << options.identity_path
              << ", creation d'une nouvelle paire de cles.\n"
                 "ATTENTION : cette cle EST le compte. Perdue, elle ne se "
                 "recupere pas, et personne ne peut la reinitialiser.\n";
    return store.create_identity(passphrase, out, error_out);
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
    if (command == "dm-send") {
        return client::run_dm_send(context, arguments, error_out);
    }
    if (command == "dm-fetch") {
        return client::run_dm_fetch(context, error_out);
    }
    error_out = "commande inconnue : " + command;
    return false;
}

// Etablit le canal, authentifie, enregistre si besoin, puis execute.
[[nodiscard]] bool run_cli(client::cli_options const &options,
                           crypto::identity_keypair const &identity,
                           std::string &error_out)
{
    crypto::x25519_public_key server_key{};
    if (!decode_server_key(options.server_key_hex, server_key, error_out)) {
        return false;
    }
    client::server_connection connection{server_key};
    if (!connection.open_session(options.host, options.port, error_out)) {
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
    crypto::identity_keypair identity;
    if (!load_identity(options, identity, failure)) {
        std::cerr << failure << '\n';
        return 3;
    }
    if (!run_cli(options, identity, failure)) {
        std::cerr << failure << '\n';
        return 4;
    }
    return 0;
}
