#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Session opening and challenge-response authentication.
//
// Full sequence, on top of the already-established Noise channel:
//   1. hello_request     the client announces its public key
//   2. auth_challenge    the server returns a 32-byte nonce
//   3. auth_response     the client signs the nonce with its private key
//   4. auth_accepted     the server verifies the signature
//
// No password is ever transmitted, stored, or even exists server-side.
// Returning false closes the connection, it does not signal a business
// error -- those go out as status_error and the session continues.

[[nodiscard]] bool handle_hello_request(handler_context &context,
                                        proto::byte_reader &reader);

[[nodiscard]] bool handle_auth_response(handler_context &context,
                                        proto::byte_reader &reader);

[[nodiscard]] bool handle_register_request(handler_context &context,
                                           proto::byte_reader &reader);

[[nodiscard]] bool handle_ping_request(handler_context &context,
                                       proto::byte_reader &reader);

} // namespace hypercom::server
