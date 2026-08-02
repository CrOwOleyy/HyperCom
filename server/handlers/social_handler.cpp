#include "server/handlers/social_handler.hpp"

#include <algorithm>

#include "common/protocol/friend_message.hpp"
#include "common/protocol/top8_message.hpp"
#include "server/db/friend_repository.hpp"
#include "server/db/top8_repository.hpp"
#include "server/db/user_repository.hpp"
#include "server/handlers/response_builder.hpp"
#include "server/handlers/session_guard.hpp"

namespace hypercom::server {
namespace {

[[nodiscard]] bool is_empty_slot(proto::wire_public_key const &key)
{
    return std::all_of(key.begin(), key.end(),
                       [](std::uint8_t byte) { return byte == 0; });
}

// Traduit les cles publiques des huit cases en identifiants internes. Renvoie
// false des qu'une cle est inconnue : mieux vaut refuser tout le top 8 que d'en
// ecrire une version amputee que l'utilisateur n'a pas demandee.
[[nodiscard]] bool resolve_top8_entries(
    user_repository &users, proto::top8_set_request const &request,
    std::vector<top8_entry> &out)
{
    for (std::size_t index = 0; index < request.slots.size(); ++index) {
        if (is_empty_slot(request.slots[index])) {
            continue;
        }
        user_row target;
        if (!users.find_by_pubkey(request.slots[index], target)) {
            return false;
        }
        out.push_back({static_cast<std::uint8_t>(index), target.id});
    }
    return true;
}

} // namespace

bool handle_friend_add_request(handler_context &context,
                               proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::friend_add_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    user_repository users{context.database};
    user_row target;
    if (!users.find_by_pubkey(request.target_pubkey, target)) {
        return send_status_error(context.connection,
                                 proto::error_code::not_found);
    }
    if (target.id == context.connection.session.user_id) {
        return send_status_error(context.connection,
                                 proto::error_code::invalid_field);
    }
    friend_repository friends{context.database};
    if (!friends.replace_friendship(context.connection.session.user_id,
                                    target.id, request.status)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_status_ok(context.connection,
                          static_cast<std::uint64_t>(target.id));
}

bool handle_friend_list_request(handler_context &context,
                                proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    // La requete n'a aucun champ : la liste demandee est toujours celle de la
    // session. Le lecteur n'est la que pour la coherence de signature.
    static_cast<void>(reader);
    friend_repository friends{context.database};
    proto::friend_list_response response;
    if (!friends.list_friends(context.connection.session.user_id,
                              proto::MAX_FRIEND_ITEMS, response.friends)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_message(context.connection,
                        proto::message_type::friend_list_response, response);
}

bool handle_top8_set_request(handler_context &context,
                             proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    proto::top8_set_request request;
    if (!request.read_from(reader)) {
        return false;
    }
    user_repository users{context.database};
    std::vector<top8_entry> entries;
    if (!resolve_top8_entries(users, request, entries)) {
        return send_status_error(context.connection,
                                 proto::error_code::not_found);
    }
    top8_repository slots{context.database};
    if (!slots.replace_slots(context.connection.session.user_id, entries)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_status_ok(context.connection, 0);
}

bool handle_top8_get_request(handler_context &context,
                             proto::byte_reader &reader)
{
    if (!require_registered_session(context)) {
        return true;
    }
    static_cast<void>(reader);
    top8_repository slots{context.database};
    proto::top8_response response;
    if (!slots.list_slots(context.connection.session.user_id, response)) {
        return send_status_error(context.connection,
                                 proto::error_code::internal_error);
    }
    return send_message(context.connection, proto::message_type::top8_response,
                        response);
}

} // namespace hypercom::server
