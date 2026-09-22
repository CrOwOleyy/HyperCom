#include "server/handlers/content_delete_handler.hpp"

#include "common/protocol/content_delete_message.hpp"
#include "server/db/comment_repository.hpp"
#include "server/db/post_repository.hpp"
#include "server/handlers/response_builder.hpp"
#include "server/handlers/session_guard.hpp"

namespace hypercom::server {
namespace {

// secure_delete nettoie le fichier principal, mais le WAL garde l'image des
// pages d'avant la modification : le texte retire y resterait lisible jusqu'au
// prochain point de controle automatique. Sur un projet qui promet que la
// suppression en est une, "il finira par disparaitre" ne suffit pas.
//
// Le cout est acceptable parce qu'une suppression est rare, contrairement a
// une lecture ou a une publication.
void checkpoint_after_deletion(handler_context &context)
{
    std::string error;
    if (!context.database.execute_script("PRAGMA wal_checkpoint(TRUNCATE);",
                                         error)) {
        // Le contenu est deja efface de la base : un point de controle qui
        // echoue retarde le nettoyage du WAL, il ne remet rien en ligne.
        context.logger.write_entry(util::log_level::warning,
                                   "point de controle WAL apres suppression : "
                                       + error);
    }
}

} // namespace

bool handle_post_delete_request(handler_context &context,
                                proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::post_delete_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    post_repository posts{context.database};
    // Un seul code pour "pas trouve", "deja supprime" et "pas le sien" : les
    // trois sont deja deductibles de la liste publique des posts, mais les
    // distinguer ici multiplierait les chemins sans rien apporter.
    if (!posts.delete_own_post(static_cast<std::int64_t>(request.post_id),
                               context.connection.session.user_id)) {
        return send_status_error(context.connection,
                                 proto::error_code::permission_denied);
    }
    checkpoint_after_deletion(context);
    return send_status_ok(context.connection, request.post_id);
}

bool handle_comment_delete_request(handler_context &context,
                                   proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::comment_delete_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    comment_repository comments{context.database};
    if (!comments.delete_own_comment(
            static_cast<std::int64_t>(request.comment_id),
            context.connection.session.user_id)) {
        return send_status_error(context.connection,
                                 proto::error_code::permission_denied);
    }
    checkpoint_after_deletion(context);
    return send_status_ok(context.connection, request.comment_id);
}

} // namespace hypercom::server
