#include "common/crypto/sodium_runtime.hpp"
#include "common/util/hex_codec.hpp"
#include "common/util/logger.hpp"
#include "server/config/config_parser.hpp"
#include "server/config/config_validator.hpp"
#include "server/db/database_handle.hpp"
#include "server/db/migration_runner.hpp"
#include "server/net/server_runtime.hpp"
#include "server/net/server_static_key.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

using namespace hypercom;

constexpr int EXIT_CONFIGURATION_ERROR = 2;
constexpr int EXIT_RUNTIME_ERROR = 3;

[[nodiscard]] bool load_validated_config(std::string const &path,
                                         server::server_config &out)
{
    std::vector<server::config_error> errors;
    if (!server::parse_config_file(path, out, errors)) {
        std::cerr << server::format_config_errors(path, errors);
        return false;
    }
    std::vector<std::string> problems;
    std::vector<std::string> warnings;
    bool const accepted = server::validate_config(out, problems, warnings);
    // Warnings are shown even when the configuration is rejected: they may
    // explain the error that follows.
    for (std::string const &warning : warnings) {
        std::cerr << "AVERTISSEMENT " << warning << '\n';
    }
    if (!accepted) {
        std::cerr << "configuration refusee : " << path << '\n';
        for (std::string const &problem : problems) {
            std::cerr << "  " << problem << '\n';
        }
        return false;
    }
    return true;
}

// Prints the server's public key at startup. This is the one clients pin:
// it must be communicated over a trusted channel, never fetched from the
// server itself -- otherwise the pinning protects against nothing.
void announce_server_key(crypto::x25519_public_key const &public_key,
                         util::logger &logger)
{
    std::string encoded;
    util::encode_hex(public_key, encoded);
    logger.write_entry(util::log_level::info,
                       "cle statique du serveur : " + encoded);
}

// Writes the same information as above in a form the client knows how to
// read (--connect-file), so the admin can pass it along without retyping
// it.
//
// The file contains nothing secret and changes NOTHING about the trust
// model: it still has to be transmitted over a secure channel. Fetching it
// from the server it describes would defeat the whole point of pinning.
//
// Only the clearnet listener appears in it. The onion port is on loopback
// and must never be advertised.
void write_connect_file(server::server_config const &config,
                        crypto::x25519_public_key const &public_key,
                        util::logger &logger)
{
    if (config.paths.connect_file_path.empty() || !config.clearnet.enabled) {
        return;
    }
    std::ofstream file{config.paths.connect_file_path, std::ios::trunc};
    if (!file) {
        logger.write_entry(util::log_level::warning,
                           "fichier de connexion non ecrit : " +
                               config.paths.connect_file_path);
        return;
    }
    std::string encoded;
    util::encode_hex(public_key, encoded);
    // bind_address is a LISTENING address: 0.0.0.0 means "all interfaces"
    // and isn't reachable by anyone. The server can't guess its public
    // address, hence advertised_host in the config. Fixing the file by
    // hand wouldn't help: it gets rewritten on every startup.
    std::string const &host = config.clearnet.advertised_host.empty()
                                  ? config.clearnet.bind_address
                                  : config.clearnet.advertised_host;
    file << "host=" << host << '\n'
         << "port=" << config.clearnet.port << '\n'
         << "server_key=" << encoded << '\n';
    if (host == "0.0.0.0") {
        logger.write_entry(
            util::log_level::warning,
            "fichier de connexion : host=0.0.0.0 n'est joignable "
            "par personne. Declarer advertised_host dans "
            "[clearnet] avant de transmettre le fichier.");
    }
    logger.write_entry(util::log_level::info,
                       "fichier de connexion : " +
                           config.paths.connect_file_path);
}

[[nodiscard]] int run_server(server::server_config const &config,
                             util::logger &logger)
{
    server::database_handle database;
    std::string failure;
    if (!database.open_database(config.paths.database_path, failure)) {
        logger.write_entry(util::log_level::error, failure);
        return EXIT_RUNTIME_ERROR;
    }
    if (!server::apply_pending_migrations(
            database, config.paths.migrations_directory, logger, failure)) {
        logger.write_entry(util::log_level::error, failure);
        return EXIT_RUNTIME_ERROR;
    }
    crypto::x25519_public_key static_public{};
    crypto::x25519_secret_key static_secret{};
    if (!server::load_or_create_server_key(config.paths.server_key_path,
                                           static_public, static_secret,
                                           failure)) {
        logger.write_entry(util::log_level::error, failure);
        return EXIT_RUNTIME_ERROR;
    }
    announce_server_key(static_public, logger);
    write_connect_file(config, static_public, logger);
    server::server_runtime runtime{config, logger, database, static_public,
                                   static_secret};
    if (!runtime.start_listeners(failure) ||
        !runtime.run_until_stopped(failure)) {
        logger.write_entry(util::log_level::error, failure);
        return EXIT_RUNTIME_ERROR;
    }
    logger.write_entry(util::log_level::info, "arret propre");
    return 0;
}

} // namespace

int main(int argc, char **argv)
{
    if (!crypto::initialize_sodium()) {
        std::cerr << "libsodium n'a pas pu s'initialiser, arret\n";
        return EXIT_RUNTIME_ERROR;
    }
    std::string const config_path =
        argc > 1 ? std::string{argv[1]} : std::string{"hypercom.conf"};
    server::server_config config;
    if (!load_validated_config(config_path, config)) {
        return EXIT_CONFIGURATION_ERROR;
    }
    std::ofstream log_file;
    if (!config.logging.file_path.empty()) {
        log_file.open(config.logging.file_path, std::ios::app);
    }
    std::ostream &sink = log_file.is_open() ? log_file : std::cout;
    util::logger logger{config.logging.level, config.logging.log_peer_addresses,
                        sink};
    return run_server(config, logger);
}
