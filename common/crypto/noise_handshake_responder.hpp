#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "common/crypto/key_types.hpp"
#include "common/crypto/noise_symmetric_state.hpp"

namespace hypercom::crypto {

// Cote serveur du motif NK. Symetrique de noise_handshake_initiator.
//
// Le serveur ne connait rien du client au niveau transport : NK n'authentifie
// que le repondeur. L'identite du client est etablie ensuite, au niveau
// applicatif, par la signature du defi (BRIEF.md 6). Cette separation est
// voulue : elle laisse un client se connecter anonymement, par exemple pour
// simplement lire un forum public.
class noise_handshake_responder {
public:
    noise_handshake_responder(x25519_public_key const &static_public,
                              x25519_secret_key const &static_secret);

    [[nodiscard]] bool read_first_message(
        std::span<std::uint8_t const> input,
        std::vector<std::uint8_t> &payload_out);

    [[nodiscard]] bool write_second_message(
        std::span<std::uint8_t const> payload,
        std::vector<std::uint8_t> &out);

    // Le repondeur emet avec la SECONDE cle et recoit avec la premiere :
    // l'inverse exact de l'initiateur.
    [[nodiscard]] bool export_transport_keys(symmetric_key &send_key,
                                             symmetric_key &receive_key) const;

    [[nodiscard]] bool is_complete() const;

private:
    noise_symmetric_state state_;
    x25519_secret_key static_secret_;
    x25519_public_key remote_ephemeral_;
    bool first_message_read_;
    bool complete_;
};

} // namespace hypercom::crypto
