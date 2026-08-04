#include "server/net/connection_registry.hpp"

namespace hypercom::server {

bool insert_connection(connection_registry &registry,
                       std::unique_ptr<client_connection> entry)
{
    if (entry == nullptr) {
        return false;
    }
    if (registry.connections.size() >= registry.max_connections) {
        return false;
    }
    std::string const &address = entry->session.peer_address;
    auto const existing = registry.address_counts.find(address);
    if (existing != registry.address_counts.end()
        && existing->second >= registry.max_connections_per_address) {
        return false;
    }
    int const descriptor = entry->socket.get_descriptor();
    registry.address_counts[address] += 1;
    registry.connections.emplace(descriptor, std::move(entry));
    return true;
}

void remove_connection(connection_registry &registry, int descriptor)
{
    auto const entry = registry.connections.find(descriptor);
    if (entry == registry.connections.end()) {
        return;
    }
    auto const counted =
        registry.address_counts.find(entry->second->session.peer_address);
    if (counted != registry.address_counts.end()) {
        if (counted->second <= 1) {
            // L'entree est retiree plutot que laissee a zero : la table ne doit
            // pas accumuler les adresses deja vues.
            registry.address_counts.erase(counted);
        } else {
            counted->second -= 1;
        }
    }
    registry.connections.erase(entry);
}

client_connection *find_connection(connection_registry &registry,
                                   int descriptor)
{
    auto const entry = registry.connections.find(descriptor);
    return entry == registry.connections.end() ? nullptr
                                               : entry->second.get();
}

std::vector<int> collect_expired_descriptors(
    connection_registry const &registry, std::uint64_t now,
    limits_config const &limits)
{
    std::vector<int> expired;
    for (auto const &entry : registry.connections) {
        session_state const &session = entry.second->session;
        bool const handshaking =
            session.phase != session_phase::authenticated;
        // idle_timeout_seconds == 0 signifie desactive : une session
        // authentifiee reste ouverte tant que le pair est la, seul le
        // keepalive TCP recupere une connexion vraiment morte (BRIEF.md 9,
        // "pas de timeout applicatif"). Le handshake, lui, reste borne : une
        // connexion qui ne finit jamais son Noise est le cout d'attaque le
        // plus bas qui soit.
        if (!handshaking && limits.idle_timeout_seconds == 0) {
            continue;
        }
        std::uint64_t const budget =
            handshaking ? limits.handshake_timeout_seconds
                        : limits.idle_timeout_seconds;
        if (now > session.last_activity_at
            && now - session.last_activity_at > budget) {
            expired.push_back(entry.first);
        }
    }
    return expired;
}

} // namespace hypercom::server
