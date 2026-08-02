#include "server/config/config_validator.hpp"

#include "common/protocol/protocol_limits.hpp"

namespace hypercom::server {
namespace {

void check_listener(listener_config const &listener, char const *name,
                    std::vector<std::string> &problems)
{
    if (!listener.enabled) {
        return;
    }
    if (listener.bind_address.empty()) {
        problems.emplace_back(std::string{"["} + name
                              + "] bind_address est requis quand enabled=true");
    }
    if (listener.port == 0) {
        problems.emplace_back(std::string{"["} + name
                              + "] port est requis quand enabled=true");
    }
}

void check_limits(limits_config const &limits,
                  std::vector<std::string> &problems)
{
    if (limits.max_connections == 0) {
        problems.emplace_back("[limits] max_connections doit etre superieur a 0");
    }
    if (limits.max_connections_per_address > limits.max_connections) {
        problems.emplace_back(
            "[limits] max_connections_per_address depasse max_connections");
    }
    // Le plafond de trame est une constante de protocole, pas un reglage :
    // l'autoriser au-dela signifierait qu'un client conforme peut envoyer des
    // trames que le parseur refuse, ou l'inverse.
    if (limits.max_frame_size > proto::MAX_FRAME_SIZE) {
        problems.emplace_back(
            "[limits] max_frame_size depasse le plafond du protocole (1 MiB)");
    }
    if (limits.max_frame_size < proto::FRAME_HEADER_SIZE + 1) {
        problems.emplace_back("[limits] max_frame_size est trop petit");
    }
    if (limits.handshake_timeout_seconds == 0
        || limits.idle_timeout_seconds == 0) {
        problems.emplace_back(
            "[limits] les delais d'expiration ne peuvent pas etre nuls : "
            "une socket sans timeout est une fuite de descripteur");
    }
}

} // namespace

bool validate_config(server_config const &config,
                     std::vector<std::string> &problems,
                     std::vector<std::string> &warnings)
{
    if (!config.clearnet.enabled && !config.onion.enabled) {
        problems.emplace_back(
            "aucun listener actif : activer [clearnet] ou [onion]");
    }
    check_listener(config.clearnet, "clearnet", problems);
    check_listener(config.onion, "onion", problems);
    check_limits(config.limits, problems);
    if (config.paths.database_path.empty()) {
        problems.emplace_back("[paths] database est requis");
    }
    if (config.paths.server_key_path.empty()) {
        problems.emplace_back("[paths] server_key est requis");
    }
    if (config.logging.log_peer_addresses) {
        // Un avertissement, pas une erreur : l'administrateur a le droit
        // d'activer ce reglage, il doit juste savoir ce qu'il fait.
        warnings.emplace_back(
            "[logging] log_peer_addresses=true : le serveur va journaliser des "
            "adresses IP, contrairement au defaut du projet. Retirer ce "
            "reglage pour revenir au comportement non surveille.");
    }
    return problems.empty();
}

} // namespace hypercom::server
