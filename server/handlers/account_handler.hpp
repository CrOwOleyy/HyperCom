#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Publishing and distribution of signed X25519 prekeys.
//
// The server does not verify the signature on upload. A badly signed prekey
// only penalizes its own owner, who simply won't receive anything anymore.
// The verification that matters is the recipient's: it's the only one that
// protects against a malicious server.

[[nodiscard]] bool handle_prekey_publish_request(handler_context &context,
                                                 proto::byte_reader &reader);

[[nodiscard]] bool handle_prekey_fetch_request(handler_context &context,
                                               proto::byte_reader &reader);

} // namespace hypercom::server
