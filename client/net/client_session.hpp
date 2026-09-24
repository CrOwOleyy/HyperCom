#pragma once

#include "client/net/server_connection.hpp"
#include "common/crypto/identity_keypair.hpp"
#include "common/protocol/wire_key.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace hypercom::client {

// Application-level challenge-response authentication, on top of the
// Noise channel.
//
// No password ever travels over the wire: the server sends a nonce, the
// client signs it. The local passphrase only serves to decrypt the
// private key on this disk, it's never transmitted, and the server
// doesn't even know it exists.
class client_session {
public:
    client_session(server_connection &connection,
                   crypto::identity_keypair const &identity);

    // Returns true even if the key doesn't have an account yet: check
    // needs_registration() afterward to know whether a handle needs to be
    // chosen.
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
