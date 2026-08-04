#include "client/cli/cli_top8_commands.hpp"

#include <iostream>

#include "client/net/message_exchange.hpp"
#include "common/protocol/status_message.hpp"
#include "common/protocol/top8_message.hpp"
#include "common/util/hex_codec.hpp"

namespace hypercom::client {
namespace {

constexpr char const *EMPTY_SLOT_MARKER = "-";

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

[[nodiscard]] bool find_slot_detail(proto::top8_response const &response,
                                    std::size_t slot,
                                    proto::friend_record &out)
{
    for (proto::friend_record const &record : response.details) {
        if (record.pubkey == response.slots[slot]) {
            out = record;
            return true;
        }
    }
    return false;
}

} // namespace

bool run_top8_set(cli_context &context,
                  std::vector<std::string> const &arguments,
                  std::string &error_out)
{
    if (arguments.size() != proto::TOP8_SLOT_COUNT) {
        error_out = "usage : top8-set <8 cles hex ou '-' pour une case vide>";
        return false;
    }
    proto::top8_set_request request;
    for (std::size_t index = 0; index < arguments.size(); ++index) {
        if (arguments[index] == EMPTY_SLOT_MARKER) {
            continue;
        }
        if (!parse_public_key(arguments[index], request.slots[index],
                              error_out)) {
            return false;
        }
    }
    if (!send_typed_message(context.connection,
                            proto::message_type::top8_set_request, request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::status_ok_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::status_ok, response,
                               error_out)) {
        return false;
    }
    std::cout << "top 8 enregistre\n";
    return true;
}

bool run_top8_get(cli_context &context,
                  std::vector<std::string> const &arguments,
                  std::string &error_out)
{
    if (arguments.empty()) {
        error_out = "usage : top8-get <pubkey_hex>";
        return false;
    }
    proto::top8_get_request request;
    if (!parse_public_key(arguments[0], request.target_pubkey, error_out)) {
        return false;
    }
    if (!send_typed_message(context.connection,
                            proto::message_type::top8_get_request, request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::top8_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::top8_response, response,
                               error_out)) {
        return false;
    }
    for (std::size_t index = 0; index < response.slots.size(); ++index) {
        proto::friend_record detail;
        if (find_slot_detail(response, index, detail)) {
            std::cout << "  " << (index + 1) << ". @" << detail.handle
                      << "  (" << detail.display_name << ")\n";
        } else {
            std::cout << "  " << (index + 1) << ". --\n";
        }
    }
    return true;
}

} // namespace hypercom::client
