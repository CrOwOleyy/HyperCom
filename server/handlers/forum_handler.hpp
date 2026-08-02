#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Creation et listage des forums.
//
// Le serveur ne refuse un forum que sur la forme du nom ou son unicite. Il n'a
// aucune liste de sujets interdits, aucune validation editoriale, et il n'est
// pas prevu d'en ajouter : c'est le point du projet.

[[nodiscard]] bool handle_forum_create_request(handler_context &context,
                                               proto::byte_reader &reader);

[[nodiscard]] bool handle_forum_list_request(handler_context &context,
                                             proto::byte_reader &reader);

} // namespace hypercom::server
