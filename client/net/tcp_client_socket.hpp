#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace hypercom::client {

// Socket TCP portable Windows / Linux.
//
// Elle est bloquante, avec un delai d'expiration en lecture. C'est un choix
// assume : le client n'a qu'une seule connexion, et une boucle d'evenements
// complete ne servirait a rien. Le delai court laisse l'UI en mode immediat
// interroger le reseau a chaque image sans jamais se figer.
class tcp_client_socket {
public:
    tcp_client_socket();

    ~tcp_client_socket();

    // Reconnectable : un appel sur une socket deja ouverte ferme la precedente
    // avant de recommencer. C'est ce qui permet la reconnexion par
    // re-handshake complet sans exposer de methode de fermeture publique, que
    // la regle O3 ne laisserait pas passer.
    [[nodiscard]] bool connect_to_host(std::string const &host,
                                       std::uint16_t port,
                                       std::string &error_out);

    // Ecrit la totalite du tampon, en bouclant sur les ecritures partielles.
    [[nodiscard]] bool send_all(std::span<std::uint8_t const> data);

    // Ajoute ce qui est disponible. received_any distingue le silence normal
    // (delai ecoule) d'une connexion fermee, qui est signalee par false.
    [[nodiscard]] bool receive_available(std::vector<std::uint8_t> &destination,
                                         bool &received_any);

private:
    void close_handle();

    // Type large volontaire : SOCKET fait 64 bits sous Windows, int sous POSIX.
    std::intptr_t handle_;
};

} // namespace hypercom::client
