#include "client/cli/cli_report_commands.hpp"

#include <algorithm>
#include <charconv>
#include <iostream>

#include "client/net/message_exchange.hpp"
#include "common/protocol/report_message.hpp"
#include "common/protocol/status_message.hpp"
#include "common/util/hex_codec.hpp"

namespace hypercom::client {
namespace {

[[nodiscard]] bool parse_identifier(std::string const &text,
                                    std::uint64_t &out)
{
    auto const result =
        std::from_chars(text.data(), text.data() + text.size(), out);
    return result.ec == std::errc{};
}

[[nodiscard]] bool parse_public_key(std::string const &text,
                                    proto::wire_public_key &out)
{
    std::vector<std::uint8_t> decoded;
    if (!util::decode_hex(text, decoded) || decoded.size() != out.size()) {
        return false;
    }
    std::copy(decoded.begin(), decoded.end(), out.begin());
    return true;
}

} // namespace

bool run_report_post(cli_context &context,
                     std::vector<std::string> const &arguments,
                     std::string &error_out)
{
    if (arguments.empty()) {
        error_out = "usage : report-post <post_id> [motif]";
        return false;
    }
    proto::report_post_request request;
    if (!parse_identifier(arguments[0], request.post_id)) {
        error_out = "identifiant invalide";
        return false;
    }
    if (arguments.size() > 1) {
        request.reason = arguments[1];
    }
    if (!send_typed_message(context.connection,
                            proto::message_type::report_post_request,
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
    std::cout << "post signale\n";
    return true;
}

bool run_report_account(cli_context &context,
                        std::vector<std::string> const &arguments,
                        std::string &error_out)
{
    if (arguments.empty()) {
        error_out = "usage : report-account <pubkey_hex> [motif]";
        return false;
    }
    proto::report_account_request request;
    if (!parse_public_key(arguments[0], request.target_pubkey)) {
        error_out = "cle publique invalide : 64 caracteres hexadecimaux "
                    "attendus";
        return false;
    }
    if (arguments.size() > 1) {
        request.reason = arguments[1];
    }
    if (!send_typed_message(context.connection,
                            proto::message_type::report_account_request,
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
    std::cout << "compte signale\n";
    return true;
}

} // namespace hypercom::client
