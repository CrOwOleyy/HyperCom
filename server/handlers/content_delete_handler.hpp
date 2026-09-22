#pragma once

#include "common/protocol/byte_reader.hpp"
#include "server/handlers/handler_context.hpp"

namespace hypercom::server {

// Retrait de son propre contenu.
//
// Fichier separe de content_handler : celui-ci est deja a quatre fonctions
// exposees, et la regle O3 en plafonne cinq.
//
// Il n'existe deliberement aucun equivalent cote protocole : aucun message
// reseau ne permet d'effacer le contenu d'autrui, la propriete se verifie
// dans le SQL, pas par un droit qu'un client pourrait exercer. C'est la porte
// que le projet existe pour ne pas avoir ouverte au reseau.
//
// La seule exception, posee au BRIEF.md 13, vit ailleurs : la commande
// d'administration `reports delete-post`, sur le socket Unix local -- jamais
// joignable depuis le reseau, reservee a la reaction a un signalement legal.
// post_repository::admin_delete_post() n'est appelee que depuis la, jamais
// depuis un handler qui lit un message client.

[[nodiscard]] bool handle_post_delete_request(handler_context &context,
                                              proto::byte_reader &reader);

[[nodiscard]] bool handle_comment_delete_request(handler_context &context,
                                                 proto::byte_reader &reader);

} // namespace hypercom::server
