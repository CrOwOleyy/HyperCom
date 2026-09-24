#include "client/cli/cli_dm_commands.hpp"

#include "client/dm/dm_courier.hpp"
#include "client/net/message_exchange.hpp"
#include "common/protocol/dm_ack_message.hpp"
#include "common/protocol/dm_fetch_message.hpp"
#include "common/protocol/dm_send_message.hpp"
#include "common/protocol/prekey_fetch_message.hpp"
#include "common/protocol/prekey_publish_message.hpp"
#include "common/util/hex_codec.hpp"

#include <algorithm>
#include <iostream>

namespace hypercom::client {
namespace {

[[nodiscard]] bool parse_public_key(std::string const &text,
                                    proto::wire_public_key &out,
                                    std::string &error_out)
{
    std::vector<std::uint8_t> decoded;
    if (!util::decode_hex(text, decoded) || decoded.size() != out.size()) {
        error_out = "cle publique invalide";
        return false;
    }
    std::copy(decoded.begin(), decoded.end(), out.begin());
    return true;
}

[[nodiscard]] bool fetch_recipient_bundle(cli_context &context,
                                          proto::wire_public_key const &target,
                                          proto::prekey_bundle_response &out,
                                          std::string &error_out)
{
    proto::prekey_fetch_request request;
    request.target_pubkey = target;
    if (!send_typed_message(context.connection,
                            proto::message_type::prekey_fetch_request,
                            request)) {
        error_out = "envoi impossible";
        return false;
    }
    return receive_typed_message(context.connection,
                                 proto::message_type::prekey_bundle_response,
                                 out, error_out);
}

} // namespace

bool run_prekey_publish(cli_context &context, std::string &error_out)
{
    proto::prekey_publish_request request;
    crypto::x25519_secret_key prekey_secret{};
    if (!derive_local_prekey(context.identity, request.prekey, prekey_secret)) {
        error_out = "derivation de la prekey impossible";
        return false;
    }
    std::vector<std::uint8_t> signing_input;
    proto::build_prekey_signing_input(context.identity.get_public_key(),
                                      request.prekey, signing_input);
    if (!context.identity.sign_message(signing_input, request.signature)) {
        error_out = "signature de la prekey impossible";
        return false;
    }
    if (!send_typed_message(context.connection,
                            proto::message_type::prekey_publish_request,
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
    std::cout << "prekey publiee\n";
    return true;
}

bool run_dm_send(cli_context &context,
                 std::vector<std::string> const &arguments,
                 std::string &error_out)
{
    if (arguments.size() < 2) {
        error_out = "usage : dm-send <pubkey_hex> <texte>";
        return false;
    }
    proto::dm_send_request request;
    if (!parse_public_key(arguments[0], request.recipient_pubkey, error_out)) {
        return false;
    }
    proto::prekey_bundle_response bundle;
    if (!fetch_recipient_bundle(context, request.recipient_pubkey, bundle,
                                error_out)) {
        return false;
    }
    // Encryption happens HERE, before anything is sent. What goes out on
    // the wire is already opaque, and will stay that way on the server's
    // disk.
    if (!seal_direct_message(context.identity, bundle, arguments[1],
                             request.ciphertext, error_out)) {
        return false;
    }
    if (!send_typed_message(context.connection,
                            proto::message_type::dm_send_request, request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::status_ok_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::status_ok, response,
                               error_out)) {
        return false;
    }
    std::cout << "message chiffre depose (" << request.ciphertext.size()
              << " octets opaques pour le serveur)\n";
    return true;
}

bool run_dm_fetch(cli_context &context, std::string &error_out)
{
    proto::dm_fetch_request request;
    if (!send_typed_message(context.connection,
                            proto::message_type::dm_fetch_request, request)) {
        error_out = "envoi impossible";
        return false;
    }
    proto::dm_list_response response;
    if (!receive_typed_message(context.connection,
                               proto::message_type::dm_list_response, response,
                               error_out)) {
        return false;
    }
    proto::dm_ack_request acknowledgement;
    std::cout << response.envelopes.size() << " message(s) :\n";
    for (proto::dm_envelope_record const &envelope : response.envelopes) {
        std::string sender;
        util::encode_hex(envelope.sender_pubkey, sender);
        std::string text;
        std::string failure;
        std::uint64_t sent_at = 0;
        if (open_direct_message(context.identity, envelope.ciphertext, text,
                                sent_at, failure)) {
            std::cout << "  de " << sender.substr(0, 16) << "... : " << text
                      << '\n';
            acknowledgement.envelope_ids.push_back(envelope.id);
        } else {
            std::cout << "  de " << sender.substr(0, 16) << "... : [illisible] "
                      << failure << '\n';
        }
    }
    if (acknowledgement.envelope_ids.empty()) {
        return true;
    }
    if (!send_typed_message(context.connection,
                            proto::message_type::dm_ack_request,
                            acknowledgement)) {
        error_out = "accuse de reception impossible";
        return false;
    }
    proto::status_ok_response acknowledged;
    return receive_typed_message(context.connection,
                                 proto::message_type::status_ok, acknowledged,
                                 error_out);
}

} // namespace hypercom::client
