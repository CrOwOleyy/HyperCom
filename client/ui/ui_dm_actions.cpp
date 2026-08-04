#include "client/ui/ui_dm_actions.hpp"

#include <algorithm>

#include "client/dm/dm_courier.hpp"
#include "client/net/message_exchange.hpp"
#include "client/ui/connection_guard.hpp"
#include "common/protocol/dm_ack_message.hpp"
#include "common/protocol/dm_fetch_message.hpp"
#include "common/protocol/dm_send_message.hpp"
#include "common/protocol/prekey_fetch_message.hpp"
#include "common/protocol/prekey_publish_message.hpp"
#include "common/util/hex_codec.hpp"
#include "common/util/unix_clock.hpp"

namespace hypercom::client {
namespace {

void report_failure(ui_state &state, std::string const &message)
{
    state.status_message = message;
    state.status_is_error = true;
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

void publish_own_prekey(cli_context &context, ui_state &state)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::prekey_publish_request request;
    crypto::x25519_secret_key prekey_secret{};
    if (!derive_local_prekey(context.identity, request.prekey,
                             prekey_secret)) {
        report_failure(state, "derivation de la prekey impossible");
        return;
    }
    std::vector<std::uint8_t> signing_input;
    proto::build_prekey_signing_input(context.identity.get_public_key(),
                                      request.prekey, signing_input);
    if (!context.identity.sign_message(signing_input, request.signature)) {
        report_failure(state, "signature de la prekey impossible");
        return;
    }
    std::string failure;
    proto::status_ok_response response;
    if (!send_typed_message(context.connection,
                            proto::message_type::prekey_publish_request,
                            request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::status_ok, response,
                                  failure)) {
        report_failure(state, failure);
        return;
    }
    state.status_message = "prekey publiee : on peut vous ecrire";
    state.status_is_error = false;
}

void refresh_inbox(cli_context &context, ui_state &state)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::dm_fetch_request request;
    std::string failure;
    proto::dm_list_response response;
    if (!send_typed_message(context.connection,
                            proto::message_type::dm_fetch_request, request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::dm_list_response,
                                  response, failure)) {
        report_failure(state, failure);
        return;
    }
    proto::dm_ack_request acknowledgement;
    for (proto::dm_envelope_record const &envelope : response.envelopes) {
        decrypted_message entry;
        util::encode_hex(envelope.sender_pubkey, entry.sender_hex);
        std::string reason;
        // La date vient du chiffre, pas du serveur : elle est donc absente si
        // le message n'a pas pu etre ouvert.
        entry.readable = open_direct_message(context.identity,
                                             envelope.ciphertext, entry.text,
                                             entry.received_at, reason);
        if (!entry.readable) {
            entry.text = reason;
        } else {
            acknowledgement.envelope_ids.push_back(envelope.id);
        }
        state.inbox.push_back(std::move(entry));
    }
    if (acknowledgement.envelope_ids.empty()) {
        return;
    }
    proto::status_ok_response acknowledged;
    if (send_typed_message(context.connection,
                           proto::message_type::dm_ack_request,
                           acknowledgement)) {
        static_cast<void>(receive_typed_message(
            context.connection, proto::message_type::status_ok, acknowledged,
            failure));
    }
}

void submit_direct_message(cli_context &context, ui_state &state)
{
    if (!ensure_connected(context, state)) {
        return;
    }
    proto::dm_send_request request;
    if (!parse_public_key(state.dm_recipient_input,
                          request.recipient_pubkey)) {
        report_failure(state, "cle publique du destinataire invalide");
        return;
    }
    proto::prekey_fetch_request fetch;
    fetch.target_pubkey = request.recipient_pubkey;
    std::string failure;
    proto::prekey_bundle_response bundle;
    if (!send_typed_message(context.connection,
                            proto::message_type::prekey_fetch_request, fetch)
        || !receive_typed_message(context.connection,
                                  proto::message_type::prekey_bundle_response,
                                  bundle, failure)) {
        report_failure(state, failure);
        return;
    }
    // Le chiffrement precede l'envoi. Ce qui quitte cette machine est deja
    // opaque, et le serveur n'aura jamais rien d'autre que ces octets.
    if (!seal_direct_message(context.identity, bundle, state.dm_text_input,
                             request.ciphertext, failure)) {
        report_failure(state, failure);
        return;
    }
    proto::status_ok_response response;
    if (!send_typed_message(context.connection,
                            proto::message_type::dm_send_request, request)
        || !receive_typed_message(context.connection,
                                  proto::message_type::status_ok, response,
                                  failure)) {
        report_failure(state, failure);
        return;
    }
    state.dm_text_input[0] = '\0';
    state.status_message = "message chiffre et depose";
    state.status_is_error = false;
}

} // namespace hypercom::client
