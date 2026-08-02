#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Amis et « top 8 ».
//
// La relation est declarative et unilaterale : personne n'a a accepter, et le
// serveur n'arbitre rien. Ajouter quelqu'un revient a le noter dans son propre
// carnet -- ce que le client affiche ensuite comme il veut.

[[nodiscard]] bool handle_friend_add_request(handler_context &context,
                                             proto::byte_reader &reader);

[[nodiscard]] bool handle_friend_list_request(handler_context &context,
                                              proto::byte_reader &reader);

[[nodiscard]] bool handle_top8_set_request(handler_context &context,
                                           proto::byte_reader &reader);

[[nodiscard]] bool handle_top8_get_request(handler_context &context,
                                           proto::byte_reader &reader);

} // namespace hypercom::server
