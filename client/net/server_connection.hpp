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

    [[nodiscard]] bool open_session(std::string const &host,
                                    std::uint16_t port,
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
    crypto::noise_handshake_initiator handshake_;
    std::optional<crypto::noise_transport> transport_;
    std::vector<std::uint8_t> input_buffer_;
    bool open_;
};

} // namespace hypercom::client
