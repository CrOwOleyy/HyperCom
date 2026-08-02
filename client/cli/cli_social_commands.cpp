#include "client/cli/cli_social_commands.hpp"

#include <algorithm>
#include <iostream>

#include "client/net/message_exchange.hpp"
#include "common/protocol/friend_message.hpp"
#include "common/protocol/profile_get_message.hpp"
#include "common/protocol/profile_set_message.hpp"
#include "common/util/hex_codec.hpp"

namespace hypercom::client {
namespace {

[[nodiscard]] bool parse_public_key(std::string const &text,
                                    proto::wire_public_key &out,
                                    std::string &error_out)
{
    std::vector<std::uint8_t> decoded;
    if (!util::decode_hex(text, decoded) || decoded.size() != out.size()) {
        error_out = "cle publique invalide : 64 caracteres hexadecimaux "
                    "attendus";
        return false;
    }
    std::copy(decoded.begin(), decoded.end(), out.begin());
    return true;
}

} // namespace

bool run_profile_set(cli_context &context,
                     std::vector<std::string> const &arguments,
                     std::string &error_out)
{
    if (arguments.size() < 2) {
        error_out = "usage : profile-set <nom_affiche> <bio>";
        return false;
    }
    proto::profile_set_request request;
    request.display_name = arguments[0];
    request.bio = arguments[1];
    if (!send_typed_message(context.connection,
                            proto::message_type::profile_set_request,
                            request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::status_ok_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::status_ok, response,
                               error_out)) {
        return false;
    }
    std::cout << "profil mis a jour\n";
    return true;
}

bool run_profile_get(cli_context &context,
                     std::vector<std::string> const &arguments,
                     std::string &error_out)
{
    if (arguments.empty()) {
        error_out = "usage : profile-get <pubkey_hex>";
        return false;
    }
    proto::profile_get_request request;
    if (!parse_public_key(arguments[0], request.target_pubkey, error_out)) {
        return false;
    }
    if (!send_typed_message(context.connection,
                            proto::message_type::profile_get_request,
                            request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::profile_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::profile_response, response,
                               error_out)) {
        return false;
    }
    std::cout << "@" << response.profile.handle << "  ("
              << response.profile.display_name << ")\n"
              << response.profile.bio << '\n';
    return true;
}

bool run_friend_add(cli_context &context,
                    std::vector<std::string> const &arguments,
                    std::string &error_out)
{
    if (arguments.empty()) {
        error_out = "usage : friend-add <pubkey_hex>";
        return false;
    }
    proto::friend_add_request request;
    request.status = proto::friendship_status::accepted;
    if (!parse_public_key(arguments[0], request.target_pubkey, error_out)) {
        return false;
    }
    if (!send_typed_message(context.connection,
                            proto::message_type::friend_add_request,
                            request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::status_ok_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::status_ok, response,
                               error_out)) {
        return false;
    }
    std::cout << "ami ajoute\n";
    return true;
}

bool run_friend_list(cli_context &context, std::string &error_out)
{
    if (!context.connection.send_frame(
            proto::message_type::friend_list_request, {})) {
        error_out = "envoi impossible";
        return false;
    }
    proto::friend_list_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::friend_list_response,
                               response, error_out)) {
        return false;
    }
    std::cout << response.friends.size() << " ami(s) :\n";
    for (proto::friend_record const &entry : response.friends) {
        std::string encoded;
        util::encode_hex(entry.pubkey, encoded);
        std::cout << "  @" << entry.handle << "  " << encoded << '\n';
    }
    return true;
}

} // namespace hypercom::client
