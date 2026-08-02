#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "server/config/server_config.hpp"
#include "server/net/client_connection.hpp"

namespace hypercom::server {

// Registre des connexions vivantes.
//
// address_counts sert uniquement au plafond par adresse. Il vit en memoire, ne
// contient que des connexions en cours, et disparait avec le processus : ce
// n'est pas un journal, et rien ici n'est ecrit sur disque.
struct connection_registry {
    std::uint32_t max_connections = 0;
    std::uint32_t max_connections_per_address = 0;
    std::unordered_map<int, std::unique_ptr<client_connection>> connections;
    std::unordered_map<std::string, std::uint32_t> address_counts;
};

[[nodiscard]] bool insert_connection(
    connection_registry &registry, std::unique_ptr<client_connection> entry);

void remove_connection(connection_registry &registry, int descriptor);

[[nodiscard]] client_connection *find_connection(
    connection_registry &registry, int descriptor);

// Connexions a fermer : handshake qui traine, ou silence prolonge. Sans ce
// balayage, une connexion ouverte puis abandonnee immobiliserait un
// descripteur pour toujours -- c'est l'attaque la moins couteuse qui soit.
[[nodiscard]] std::vector<int> collect_expired_descriptors(
    connection_registry const &registry, std::uint64_t now,
    limits_config const &limits);

} // namespace hypercom::server
