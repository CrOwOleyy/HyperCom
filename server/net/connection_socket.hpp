#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "server/net/unique_descriptor.hpp"

namespace hypercom::server {

// Plomberie d'octets d'une connexion : lecture non bloquante, file d'ecriture.
// Elle ne sait rien de Noise ni du protocole -- c'est voulu, ca la rend
// testable seule et ca garde le chiffrement dans un seul endroit.
class connection_socket {
public:
    explicit connection_socket(int descriptor);

    // Ajoute a destination tout ce qui est disponible. Renvoie false sur fin de
    // flux ou erreur fatale : la connexion doit alors etre fermee.
    [[nodiscard]] bool read_available(std::vector<std::uint8_t> &destination);

    // Ecrit ce qu'elle peut sans bloquer. has_remaining indique s'il reste des
    // octets en attente, auquel cas la boucle doit surveiller EPOLLOUT.
    [[nodiscard]] bool flush_pending_writes(bool &has_remaining);

    void queue_bytes(std::span<std::uint8_t const> data);

    [[nodiscard]] int get_descriptor() const;

private:
    unique_descriptor descriptor_;
    std::vector<std::uint8_t> pending_output_;
};

} // namespace hypercom::server
