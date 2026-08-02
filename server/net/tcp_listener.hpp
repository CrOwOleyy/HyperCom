#pragma once

#include <cstdint>
#include <string>

#include "server/net/unique_descriptor.hpp"

namespace hypercom::server {

// Socket d'ecoute. Le meme code sert au clearnet et au service cache : Tor se
// contente de relayer vers un listener sur la boucle locale, il n'y a donc
// aucune logique specifique a l'oignon.
class tcp_listener {
public:
    tcp_listener();

    [[nodiscard]] bool open_listener(std::string const &address,
                                     std::uint16_t port,
                                     std::string &error_out);

    // Renvoie -1 quand il n'y a plus rien a accepter. peer_address n'est
    // renseignee que pour etre passee au logger, qui la masquera si la
    // politique de journalisation ne l'autorise pas.
    [[nodiscard]] int accept_connection(std::string &peer_address) const;

    [[nodiscard]] int get_descriptor() const;

private:
    unique_descriptor descriptor_;
};

} // namespace hypercom::server
