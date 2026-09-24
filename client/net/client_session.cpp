#include "client/net/client_session.hpp"

#include "client/net/message_exchange.hpp"
#include "common/protocol/auth_message.hpp"
#include "common/protocol/hello_message.hpp"
#include "common/protocol/register_message.hpp"

#include <vector>

namespace hypercom::client {

client_session::client_session(server_connection &connection,
                               crypto::identity_keypair const &identity)
    : connection_{connection},
      identity_{identity},
      challenge_nonce_{},
      handle_{},
      user_id_{0},
      needs_registration_{true}
{}

bool client_session::needs_registration() const
{
    return needs_registration_;
}

std::string const &client_session::get_handle() const
{
    return handle_;
}

bool client_session::sign_and_send_response(std::string &error_out)
{
    std::vector<std::uint8_t> signing_input;
    proto::build_auth_signing_input(challenge_nonce_,
                                    identity_.get_public_key(), signing_input);
    proto::auth_response response;
    if (!identity_.sign_message(signing_input, response.signature)) {
        error_out = "signature du defi impossible";
        return false;
    }
    if (!send_typed_message(connection_, proto::message_type::auth_response,
                            response)) {
        error_out = "envoi de la reponse d'authentification impossible";
        return false;
    }
    return true;
}

bool client_session::authenticate(std::string &error_out)
{
    proto::hello_request hello;
    hello.client_pubkey = identity_.get_public_key();
    if (!send_typed_message(connection_, proto::message_type::hello_request,
                            hello)) {
        error_out = "envoi du hello impossible";
        return false;
    }
    proto::auth_challenge challenge;
    if (!receive_typed_message(connection_, proto::message_type::auth_challenge,
                               challenge, error_out)) {
        return false;
    }
    challenge_nonce_ = challenge.nonce;
    if (!sign_and_send_response(error_out)) {
        return false;
    }
    // Two possible responses: auth_accepted if the key already has an
    // account, status_ok if it has just been proven but doesn't have one
    // yet.
    proto::frame_header header{};
    std::vector<std::uint8_t> payload;
    if (!connection_.receive_frame(header, payload, error_out)) {
        return false;
    }
    proto::byte_reader reader{payload};
    if (header.type == proto::message_type::auth_accepted) {
        proto::auth_accepted accepted;
        if (!accepted.read_from(reader)) {
            error_out = "reponse d'authentification mal formee";
            return false;
        }
        handle_ = accepted.handle;
        user_id_ = accepted.user_id;
        needs_registration_ = false;
        return true;
    }
    if (header.type == proto::message_type::status_ok) {
        needs_registration_ = true;
        return true;
    }
    error_out = "authentification refusee par le serveur";
    return false;
}

bool client_session::register_handle(std::string_view handle,
                                     std::string &error_out)
{
    if (!proto::validate_handle(handle)) {
        error_out = "pseudo invalide : 3 a 32 caracteres, lettres, "
                    "chiffres, tiret ou souligne, ne commencant pas par un "
                    "chiffre";
        return false;
    }
    proto::register_request request;
    request.handle = std::string{handle};
    if (!send_typed_message(connection_, proto::message_type::register_request,
                            request)) {
        error_out = "envoi de l'enregistrement impossible";
        return false;
    }
    proto::auth_accepted accepted;
    if (!receive_typed_message(connection_, proto::message_type::auth_accepted,
                               accepted, error_out)) {
        return false;
    }
    handle_ = accepted.handle;
    user_id_ = accepted.user_id;
    needs_registration_ = false;
    return true;
}

} // namespace hypercom::client
