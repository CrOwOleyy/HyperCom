#include "server/config/config_validator.hpp"

#include <cstdint>
#include <string>

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
    // Le handshake reste borne dans tous les cas : une connexion qui ne le
    // termine jamais est la fuite de descripteur la moins chere a provoquer.
    if (limits.handshake_timeout_seconds == 0) {
        problems.emplace_back(
            "[limits] handshake_timeout_seconds ne peut pas etre nul : une "
            "socket qui ne finit jamais son handshake est une fuite de "
            "descripteur");
    }
    // idle_timeout_seconds == 0 est valide et signifie desactive : le
    // protocole n'a pas de timeout applicatif par choix (BRIEF.md 9), seul le
    // keepalive TCP recupere une session authentifiee dont le pair a
    // disparu.
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
        // Un an, pas une valeur arbitraire : c'est le plancher legal
        // (art. L.34-1 CPCE, art. 6-II LCEN, decret du 21/10/2025). En
        // dessous, journaliser des IP n'apporte la conformite qu'en apparence
        // (BRIEF.md 13).
        constexpr std::uint32_t LEGAL_RETENTION_MINIMUM_DAYS = 365;
        if (config.logging.retention_days < LEGAL_RETENTION_MINIMUM_DAYS) {
            warnings.emplace_back(
                "[logging] retention_days="
                + std::to_string(config.logging.retention_days)
                + " est sous le plancher legal francais d'un an pour les "
                  "donnees de connexion. Journaliser sans le conserver assez "
                  "longtemps n'apporte pas la conformite que log_peer_addresses "
                  "laisse croire.");
        }
    }
    return problems.empty();
}

} // namespace hypercom::server
