#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "client/net/server_connection.hpp"
#include "common/crypto/identity_keypair.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::client {

// Authentification applicative par defi-reponse, au-dessus du canal Noise.
//
// Aucun mot de passe ne circule : le serveur envoie un nonce, le client le
// signe. La passphrase locale ne sert qu'a dechiffrer la cle privee sur ce
// disque, elle n'est jamais transmise et le serveur n'en connait pas
// l'existence.
class client_session {
public:
    client_session(server_connection &connection,
                   crypto::identity_keypair const &identity);

    // Renvoie true meme si la cle n'a pas encore de compte : consulter ensuite
    // needs_registration() pour savoir s'il faut choisir un pseudo.
    [[nodiscard]] bool authenticate(std::string &error_out);

    [[nodiscard]] bool register_handle(std::string_view handle,
                                       std::string &error_out);

    [[nodiscard]] bool needs_registration() const;

    [[nodiscard]] std::string const &get_handle() const;

private:
    [[nodiscard]] bool sign_and_send_response(std::string &error_out);

    server_connection &connection_;
    crypto::identity_keypair const &identity_;
    proto::wire_nonce challenge_nonce_;
    std::string handle_;
    std::uint64_t user_id_;
    bool needs_registration_;
};

} // namespace hypercom::client
