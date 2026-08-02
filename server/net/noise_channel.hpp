#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "common/crypto/noise_handshake_responder.hpp"
#include "common/crypto/noise_transport.hpp"

namespace hypercom::server {

// Cote serveur du canal chiffre. Tout ce qui traverse la connexion passe par
// ici : le protocole applicatif n'est jamais visible sur le fil, pas meme le
// type de la trame.
class noise_channel {
public:
    noise_channel(crypto::x25519_public_key const &static_public,
                  crypto::x25519_secret_key const &static_secret);

    // Consomme le premier message du client et produit la reponse. Un echec
    // signifie handshake invalide : la connexion se ferme sans autre reponse,
    // et surtout sans message d'erreur qui distinguerait les causes.
    [[nodiscard]] bool accept_handshake_message(
        std::span<std::uint8_t const> input, std::vector<std::uint8_t> &reply);

    [[nodiscard]] bool is_established() const;

    [[nodiscard]] bool open_message(std::span<std::uint8_t const> ciphertext,
                                    std::vector<std::uint8_t> &plaintext);

    [[nodiscard]] bool seal_message(std::span<std::uint8_t const> plaintext,
                                    std::vector<std::uint8_t> &ciphertext);

private:
    crypto::noise_handshake_responder handshake_;
    std::optional<crypto::noise_transport> transport_;
};

} // namespace hypercom::server
