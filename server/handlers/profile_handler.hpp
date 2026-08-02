#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Profils personnalisables -- le cote MySpace de la v1.
//
// Le profil est public par nature : n'importe quelle session authentifiee peut
// lire celui de n'importe qui. Il n'y a pas de reglage de visibilite, et c'est
// coherent avec le reste : ce qui doit rester prive passe par les DM chiffres,
// pas par un drapeau que le serveur promettrait de respecter.

[[nodiscard]] bool handle_profile_get_request(handler_context &context,
                                              proto::byte_reader &reader);

[[nodiscard]] bool handle_profile_set_request(handler_context &context,
                                              proto::byte_reader &reader);

} // namespace hypercom::server
