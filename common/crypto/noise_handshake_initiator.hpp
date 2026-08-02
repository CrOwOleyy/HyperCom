#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "common/crypto/key_types.hpp"
#include "common/crypto/noise_symmetric_state.hpp"

namespace hypercom::crypto {

// Cote client du motif NK :
//
//   <- s                (connu a l'avance, epingle)
//   ...
//   -> e, es            write_first_message
//   <- e, ee            read_second_message
//
// Le client connait la cle statique du serveur avant d'ouvrir la connexion.
// C'est ce qui remplace la chaine de certificats : il n'y a pas d'autorite a
// interroger, seulement une cle a comparer.
class noise_handshake_initiator {
public:
    explicit noise_handshake_initiator(
        x25519_public_key const &server_static_public);

    [[nodiscard]] bool write_first_message(
        std::span<std::uint8_t const> payload,
        std::vector<std::uint8_t> &out);

    // Efface la cle ephemere des sa derniere utilisation, sans attendre la
    // destruction de l'objet : c'est ce qui donne la confidentialite
    // persistante du handshake.
    [[nodiscard]] bool read_second_message(
        std::span<std::uint8_t const> input,
        std::vector<std::uint8_t> &payload_out);

    // L'initiateur emet avec la premiere cle et recoit avec la seconde.
    [[nodiscard]] bool export_transport_keys(symmetric_key &send_key,
                                             symmetric_key &receive_key) const;

    [[nodiscard]] bool is_complete() const;

private:
    noise_symmetric_state state_;
    x25519_public_key remote_static_;
    x25519_public_key ephemeral_public_;
    x25519_secret_key ephemeral_secret_;
    bool first_message_sent_;
    bool complete_;
};

} // namespace hypercom::crypto
