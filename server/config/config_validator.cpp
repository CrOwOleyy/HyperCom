#include "server/config/config_validator.hpp"

#include "common/protocol/protocol_limits.hpp"

#include <cstdint>
#include <string>

namespace hypercom::server {
namespace {

void check_listener(listener_config const &listener, char const *name,
                    std::vector<std::string> &problems)
{
    if (!listener.enabled) {
        return;
    }
    if (listener.bind_address.empty()) {
        problems.emplace_back(std::string{"["} + name +
                              "] bind_address est requis quand enabled=true");
    }
    if (listener.port == 0) {
        problems.emplace_back(std::string{"["} + name +
                              "] port est requis quand enabled=true");
    }
}

void check_limits(limits_config const &limits,
                  std::vector<std::string> &problems)
{
    if (limits.max_connections == 0) {
        problems.emplace_back(
            "[limits] max_connections doit etre superieur a 0");
    }
    if (limits.max_connections_per_address > limits.max_connections) {
        problems.emplace_back(
            "[limits] max_connections_per_address depasse max_connections");
    }
    // The frame cap is a protocol constant, not a setting: allowing it
    // beyond that would mean a compliant client could send frames the
    // parser rejects, or the other way around.
    if (limits.max_frame_size > proto::MAX_FRAME_SIZE) {
        problems.emplace_back(
            "[limits] max_frame_size depasse le plafond du protocole (1 MiB)");
    }
    if (limits.max_frame_size < proto::FRAME_HEADER_SIZE + 1) {
        problems.emplace_back("[limits] max_frame_size est trop petit");
    }
    // The handshake stays bounded in all cases: a connection that never
    // finishes it is the cheapest descriptor leak to trigger.
    if (limits.handshake_timeout_seconds == 0) {
        problems.emplace_back(
            "[limits] handshake_timeout_seconds ne peut pas etre nul : une "
            "socket qui ne finit jamais son handshake est une fuite de "
            "descripteur");
    }
    // idle_timeout_seconds == 0 is valid and means disabled: the protocol
    // deliberately has no application-level timeout, only TCP keepalive
    // reclaims an authenticated session whose peer has vanished.
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
        // A warning, not an error: the administrator has the right to
        // enable this setting, they just need to know what they're doing.
        warnings.emplace_back(
            "[logging] log_peer_addresses=true : le serveur va journaliser des "
            "adresses IP, contrairement au defaut du projet. Retirer ce "
            "reglage pour revenir au comportement non surveille.");
        // One year, not an arbitrary value: it's the legal floor (art.
        // L.34-1 CPCE, art. 6-II LCEN, decree of 2025-10-21). Below that,
        // logging IPs only gives the appearance of compliance.
        constexpr std::uint32_t LEGAL_RETENTION_MINIMUM_DAYS = 365;
        if (config.logging.retention_days < LEGAL_RETENTION_MINIMUM_DAYS) {
            warnings.emplace_back(
                "[logging] retention_days=" +
                std::to_string(config.logging.retention_days) +
                " est sous le plancher legal francais d'un an pour les "
                "donnees de connexion. Journaliser sans le conserver assez "
                "longtemps n'apporte pas la conformite que log_peer_addresses "
                "laisse croire.");
        }
    }
    return problems.empty();
}

} // namespace hypercom::server
