#pragma once

#include <cstdint>
#include <vector>

#include "server/net/connection_socket.hpp"
#include "server/net/noise_channel.hpp"
#include "server/net/session_state.hpp"

namespace hypercom::server {

// Une connexion = sa plomberie, son canal chiffre, son etat applicatif.
//
// Structure sans aucune methode. La regle O3
// plafonne les classes a cinq methodes publiques, or une connexion touche a
// trop de choses pour tenir dans ce budget. En separant les trois
// responsabilites en trois types deja complets, l'agregat n'a plus rien a
// faire lui-meme -- ce qui est exactement ce que la regle cherche a obtenir.
struct client_connection {
    connection_socket socket;
    noise_channel channel;
    session_state session;
    std::vector<std::uint8_t> input_buffer;
};

} // namespace hypercom::server
