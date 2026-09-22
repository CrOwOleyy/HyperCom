#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "client/net/tcp_client_socket.hpp"
#include "common/crypto/noise_handshake_initiator.hpp"
#include "common/crypto/noise_transport.hpp"
#include "common/protocol/frame_codec.hpp"

namespace hypercom::client {

// Ou joindre le serveur, et par quel chemin.
//
// socks5_host vide = connexion directe. Rempli, la socket s'ouvre vers le
// proxy qui relaie ensuite vers host:port -- c'est le seul moyen d'atteindre
// un service cache .onion. Le handshake Noise se deroule identiquement dans
// les deux cas : il ne sait pas par ou passent ses octets, et l'epinglage de
// la cle serveur protege donc exactement pareil.
struct server_endpoint {
    std::string host;
    std::uint16_t port = 0;
    std::string socks5_host;
    std::uint16_t socks5_port = 0;
};

// Connexion au serveur : TCP, puis handshake Noise_NK, puis trames chiffrees.
//
// La cle statique du serveur est fournie a la construction : c'est
// l'epinglage. Elle doit venir d'un canal de confiance -- affichee par
// l'administrateur, transmise de la main a la main -- et surtout pas du serveur
// lui-meme, sinon l'epinglage ne protege de rien.
//
// Un serveur substitue echouera au dechiffrement du second message du
// handshake, sans qu'aucun secret n'ait ete revele.
class server_connection {
public:
    explicit server_connection(
        crypto::x25519_public_key const &server_static_public);

    // Sert aussi de reconnexion : rappeler open_session sur une connexion
    // tombee repart d'un handshake Noise complet, avec une cle ephemere
    // fraiche. Aucun jeton de reprise n'est conserve d'une session a l'autre,
    // et le serveur n'a donc rien qui permette de recoudre deux connexions.
    // La contrepartie assumee est qu'il faut refaire le defi-reponse.
    [[nodiscard]] bool open_session(server_endpoint const &endpoint,
                                    std::string &error_out);

    [[nodiscard]] bool send_frame(proto::message_type type,
                                  std::span<std::uint8_t const> payload);

    [[nodiscard]] bool receive_frame(proto::frame_header &header,
                                     std::vector<std::uint8_t> &payload,
                                     std::string &error_out);

    [[nodiscard]] bool is_open() const;

private:
    [[nodiscard]] bool perform_handshake(std::string &error_out);

    [[nodiscard]] bool pull_next_message(std::vector<std::uint8_t> &out,
                                         std::string &error_out);

    tcp_client_socket socket_;
    crypto::x25519_public_key server_static_public_;
    crypto::noise_handshake_initiator handshake_;
    std::optional<crypto::noise_transport> transport_;
    std::vector<std::uint8_t> input_buffer_;
    bool open_;
};

} // namespace hypercom::client
