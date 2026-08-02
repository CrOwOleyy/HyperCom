#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "common/crypto/sodium_runtime.hpp"
#include "common/util/hex_codec.hpp"
#include "common/util/logger.hpp"
#include "server/config/config_parser.hpp"
#include "server/config/config_validator.hpp"
#include "server/db/database_handle.hpp"
#include "server/db/migration_runner.hpp"
#include "server/net/server_runtime.hpp"
#include "server/net/server_static_key.hpp"

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
    // Les avertissements s'affichent meme quand la configuration est refusee :
    // ils peuvent expliquer l'erreur qui suit.
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

// Affiche la cle publique du serveur au demarrage. C'est elle que les clients
// epinglent : elle doit etre communiquee par un canal de confiance, jamais
// recuperee depuis le serveur lui-meme -- sinon l'epinglage ne protege de rien.
void announce_server_key(crypto::x25519_public_key const &public_key,
                         util::logger &logger)
{
    std::string encoded;
    util::encode_hex(public_key, encoded);
    logger.write_entry(util::log_level::info,
                       "cle statique du serveur : " + encoded);
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
    server::server_runtime runtime{config, logger, database, static_public,
                                   static_secret};
    if (!runtime.start_listeners(failure)
        || !runtime.run_until_stopped(failure)) {
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
    util::logger logger{config.logging.level,
                        config.logging.log_peer_addresses, sink};
    return run_server(config, logger);
}
