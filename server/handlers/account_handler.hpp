#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Publication et distribution des prekeys X25519 signees.
//
// Le serveur ne verifie pas la signature au depot. Une prekey mal signee ne
// penalise que son proprietaire, qui ne recevra plus rien. La verification qui
// compte est celle du destinataire : c'est la seule qui protege contre un
// serveur malveillant.

[[nodiscard]] bool handle_prekey_publish_request(handler_context &context,
                                                 proto::byte_reader &reader);

[[nodiscard]] bool handle_prekey_fetch_request(handler_context &context,
                                               proto::byte_reader &reader);

} // namespace hypercom::server
