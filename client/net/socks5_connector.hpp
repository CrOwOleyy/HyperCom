#pragma once

#include <cstdint>
#include <string>

#include "client/net/tcp_client_socket.hpp"

namespace hypercom::client {

// Negociation SOCKS5 (RFC 1928) sur une socket DEJA connectee au proxy.
//
// C'est ce qui permet d'atteindre un service cache .onion : Tor n'est pas
// dans le DNS, un getaddrinfo() echouerait. Le client se connecte au proxy
// local de Tor, lui demande d'ouvrir la connexion vers l'adresse .onion, et
// la socket devient ensuite un tunnel transparent -- le handshake Noise se
// deroule au-dessus sans rien savoir de tout ca.
//
// Seul le mode « sans authentification » est gere : le proxy vit en boucle
// locale sur la machine de l'utilisateur, il n'y a personne a authentifier.
[[nodiscard]] bool perform_socks5_connect(tcp_client_socket &socket,
                                          std::string const &target_host,
                                          std::uint16_t target_port,
                                          std::string &error_out);

} // namespace hypercom::client
