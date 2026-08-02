#include "server/handlers/session_handler.hpp"

#include "common/crypto/signature_verifier.hpp"
#include "common/crypto/sodium_runtime.hpp"
#include "common/protocol/auth_message.hpp"
#include "common/protocol/hello_message.hpp"
#include "common/protocol/motd_message.hpp"
#include "common/protocol/ping_message.hpp"
#include "common/protocol/register_message.hpp"
#include "common/util/unix_clock.hpp"
#include "server/db/motd_repository.hpp"
#include "server/db/profile_repository.hpp"
#include "server/db/user_repository.hpp"
#include "server/handlers/response_builder.hpp"

namespace hypercom::server {
namespace {

[[nodiscard]] bool send_accepted_session(handler_context &context,
                                         user_row const &user)
{
    session_state &session = context.connection.session;
    session.phase = session_phase::authenticated;
    session.user_id = user.id;
    session.handle = user.handle;
    // Aucune ecriture en base a l'authentification : enregistrer « untel s'est
    // connecte a telle heure » serait un journal de presence, et il serait
    // conserve avec le disque.
    proto::auth_accepted accepted;
    accepted.user_id = static_cast<std::uint64_t>(user.id);
    accepted.handle = user.handle;
    accepted.server_time = util::get_unix_timestamp();
    if (!send_message(context.connection, proto::message_type::auth_accepted,
                      accepted)) {
        return false;
    }
    motd_repository motd{context.database};
    proto::motd_push announcement;
    if (!motd.find_active_motd(announcement)) {
        return true;
    }
    return send_message(context.connection, proto::message_type::motd_push,
                        announcement);
}

} // namespace

bool handle_hello_request(handler_context &context,
                          proto::byte_reader &reader)
{
    session_state &session = context.connection.session;
    if (session.phase != session_phase::awaiting_hello) {
        return send_status_error(context.connection,
                                 proto::error_code::already_authenticated);
    }
    proto::hello_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    if (request.protocol_version != proto::PROTOCOL_VERSION) {
        return send_status_error(context.connection,
                                 proto::error_code::unsupported_version);
    }
    session.announced_pubkey = request.client_pubkey;
    crypto::fill_random_bytes(session.challenge_nonce);
    user_repository users{context.database};
    user_row existing;
    proto::auth_challenge challenge;
    challenge.nonce = session.challenge_nonce;
    challenge.account_exists =
        users.find_by_pubkey(request.client_pubkey, existing) ? 1U : 0U;
    challenge.server_time = util::get_unix_timestamp();
    session.phase = session_phase::awaiting_auth;
    return send_message(context.connection,
                        proto::message_type::auth_challenge, challenge);
}

bool handle_auth_response(handler_context &context,
                          proto::byte_reader &reader)
{
    session_state &session = context.connection.session;
    if (session.phase != session_phase::awaiting_auth) {
        return send_status_error(context.connection,
                                 proto::error_code::not_authenticated);
    }
    proto::auth_response response;
    if (!response.read_from(reader)) {
        return false;
    }
    std::vector<std::uint8_t> signed_input;
    proto::build_auth_signing_input(session.challenge_nonce,
                                    session.announced_pubkey, signed_input);
    if (!crypto::verify_signature(session.announced_pubkey, signed_input,
                                  response.signature)) {
        // Fermeture immediate : rejouer un defi avec une autre signature ne
        // doit pas etre possible sur la meme connexion.
        return false;
    }
    user_repository users{context.database};
    user_row existing;
    if (users.find_by_pubkey(session.announced_pubkey, existing)) {
        return send_accepted_session(context, existing);
    }
    // Cle prouvee mais sans compte : seul register_request est desormais
    // recevable. user_id reste a 0, ce que verifie require_registered_session.
    session.phase = session_phase::authenticated;
    session.user_id = 0;
    return send_status_ok(context.connection, 0);
}

bool handle_register_request(handler_context &context,
                             proto::byte_reader &reader)
{
    session_state &session = context.connection.session;
    if (session.phase != session_phase::authenticated
        || session.user_id != 0) {
        return send_status_error(context.connection,
                                 proto::error_code::already_authenticated);
    }
    if (!context.config.registration_open) {
        return send_status_error(context.connection,
                                 proto::error_code::permission_denied);
    }
    proto::register_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    if (!proto::validate_handle(request.handle)) {
        return send_status_error(context.connection,
                                 proto::error_code::invalid_field);
    }
    user_repository users{context.database};
    std::int64_t created_id = 0;
    if (!users.create_user(session.announced_pubkey, request.handle,
                           created_id)) {
        // La contrainte UNIQUE a tranche : le pseudo est deja pris.
        return send_status_error(context.connection,
                                 proto::error_code::handle_unavailable);
    }
    user_row created;
    if (!users.find_by_id(created_id, created)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    profile_repository profiles{context.database};
    static_cast<void>(profiles.replace_profile(created_id, {}));
    return send_accepted_session(context, created);
}

bool handle_ping_request(handler_context &context, proto::byte_reader &reader)
{
    proto::ping_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    proto::ping_response response;
    response.token = request.token;
    response.server_time = util::get_unix_timestamp();
    return send_message(context.connection, proto::message_type::ping_response,
                        response);
}

} // namespace hypercom::server
