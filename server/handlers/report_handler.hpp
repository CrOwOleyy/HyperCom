#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Reception d'un signalement (BRIEF.md 13). Le serveur enregistre, il ne
// juge rien -- aucune action automatique n'en decoule, c'est l'admin qui lit
// la file via `reports` sur le socket local.
//
// report_account_request ne verifie pas que la cible est un ami ou un
// correspondant du signaleur : c'est volontaire, quelqu'un peut vouloir
// signaler un compte avant de lui avoir jamais parle.

[[nodiscard]] bool handle_report_post_request(handler_context &context,
                                              proto::byte_reader &reader);

[[nodiscard]] bool handle_report_account_request(handler_context &context,
                                                  proto::byte_reader &reader);

} // namespace hypercom::server
