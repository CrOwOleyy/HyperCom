#include "client/dm/dm_courier.hpp"

#include "common/crypto/dm_envelope.hpp"
#include "common/crypto/dm_message_chain.hpp"
#include "common/crypto/dm_session_keys.hpp"
#include "common/crypto/hkdf_sha256.hpp"
#include "common/crypto/secure_memory.hpp"
#include "common/crypto/signature_verifier.hpp"
#include "common/crypto/x25519_exchange.hpp"
#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/prekey_publish_message.hpp"
#include "common/protocol/text_field_codec.hpp"
#include "common/util/unix_clock.hpp"

#include <algorithm>
#include <string_view>

namespace hypercom::client {
namespace {

constexpr std::string_view PREKEY_DERIVATION_INFO = "hypercom-prekey-derive-v1";
constexpr std::uint8_t DM_PAYLOAD_VERSION = 1;

[[nodiscard]] std::span<std::uint8_t const> as_bytes(std::string_view text)
{
    return {reinterpret_cast<std::uint8_t const *>(text.data()), text.size()};
}

// Encrypted content of a message: [u8 version][u64 sent-at timestamp]
// [text].
//
// The timestamp is INSIDE the ciphertext, not in a database column. The
// server therefore stores an envelope whose timestamp it doesn't even
// know -- it can no longer keep a time-stamped log of who exchanges
// messages with whom.
void build_dm_payload(std::uint64_t sent_at, std::string_view text,
                      std::vector<std::uint8_t> &out)
{
    proto::byte_writer writer{out};
    writer.write_integer(DM_PAYLOAD_VERSION);
    writer.write_integer(sent_at);
    writer.write_fixed_bytes(as_bytes(text));
}

[[nodiscard]] bool parse_dm_payload(std::span<std::uint8_t const> payload,
                                    std::uint64_t &sent_at_out,
                                    std::string &text_out)
{
    proto::byte_reader reader{payload};
    std::uint8_t version = 0;
    if (!reader.read_integer(version) || version != DM_PAYLOAD_VERSION ||
        !reader.read_integer(sent_at_out)) {
        return false;
    }
    std::vector<std::uint8_t> remaining(reader.count_remaining_bytes());
    if (!reader.read_fixed_bytes(remaining)) {
        return false;
    }
    text_out.assign(remaining.begin(), remaining.end());
    // The text comes from a peer, not the server: it's validated like
    // anything arriving from outside.
    return proto::validate_text_field(text_out);
}

// A single message key is enough: each send creates a new ephemeral key,
// hence a new root, hence a fresh ratchet with the counter at zero.
[[nodiscard]] bool derive_first_message_key(crypto::symmetric_key const &root,
                                            crypto::symmetric_key &out)
{
    crypto::dm_message_chain chain{root};
    return chain.derive_next_message_key(out);
}

} // namespace

bool derive_local_prekey(crypto::identity_keypair const &identity,
                         crypto::x25519_public_key &public_out,
                         crypto::x25519_secret_key &secret_out)
{
    // The first 32 bytes of the Ed25519 private key are the seed.
    crypto::ed25519_secret_key const &secret = identity.get_secret_key();
    crypto::symmetric_key seed_material{};
    std::copy_n(secret.begin(), seed_material.size(), seed_material.begin());
    crypto::symmetric_key derived{};
    bool const expanded = crypto::expand_key_block(
        seed_material, as_bytes(PREKEY_DERIVATION_INFO), 0x01, derived);
    crypto::wipe_bytes(seed_material);
    if (!expanded) {
        crypto::wipe_bytes(derived);
        return false;
    }
    std::copy(derived.begin(), derived.end(), secret_out.begin());
    crypto::wipe_bytes(derived);
    return crypto::compute_public_from_secret(secret_out, public_out);
}

bool verify_prekey_bundle(proto::prekey_bundle_response const &bundle)
{
    std::vector<std::uint8_t> signing_input;
    proto::build_prekey_signing_input(bundle.owner_pubkey, bundle.prekey,
                                      signing_input);
    return crypto::verify_signature(bundle.owner_pubkey, signing_input,
                                    bundle.signature);
}

bool seal_direct_message(crypto::identity_keypair const &sender,
                         proto::prekey_bundle_response const &recipient_bundle,
                         std::string_view text, std::vector<std::uint8_t> &out,
                         std::string &error_out)
{
    if (!verify_prekey_bundle(recipient_bundle)) {
        error_out = "prekey du destinataire non signee par son identite : "
                    "le serveur tente peut-etre de s'interposer";
        return false;
    }
    crypto::x25519_public_key ephemeral_public{};
    crypto::x25519_secret_key ephemeral_secret{};
    if (!crypto::generate_x25519_keypair(ephemeral_public, ephemeral_secret)) {
        error_out = "generation de la cle ephemere impossible";
        return false;
    }
    crypto::symmetric_key root{};
    bool succeeded = crypto::derive_sender_session_key(
        sender, ephemeral_secret, recipient_bundle.owner_pubkey,
        recipient_bundle.prekey, root);
    crypto::wipe_bytes(ephemeral_secret);
    crypto::symmetric_key message_key{};
    if (succeeded) {
        succeeded = derive_first_message_key(root, message_key);
    }
    crypto::wipe_bytes(root);
    if (succeeded) {
        crypto::dm_envelope_header header;
        header.sender_identity = sender.get_public_key();
        header.sender_ephemeral = ephemeral_public;
        header.counter = 0;
        std::vector<std::uint8_t> payload;
        build_dm_payload(util::get_unix_timestamp(), text, payload);
        succeeded = crypto::seal_dm_envelope(header, message_key, payload, out);
    }
    crypto::wipe_bytes(message_key);
    if (!succeeded) {
        error_out = "chiffrement du message impossible";
    }
    return succeeded;
}

bool open_direct_message(crypto::identity_keypair const &recipient,
                         std::span<std::uint8_t const> envelope,
                         std::string &text_out, std::uint64_t &sent_at_out,
                         std::string &error_out)
{
    crypto::dm_envelope_header header;
    if (!crypto::parse_dm_envelope_header(envelope, header)) {
        error_out = "enveloppe illisible";
        return false;
    }
    crypto::x25519_public_key prekey_public{};
    crypto::x25519_secret_key prekey_secret{};
    if (!derive_local_prekey(recipient, prekey_public, prekey_secret)) {
        error_out = "derivation de la prekey locale impossible";
        return false;
    }
    crypto::symmetric_key root{};
    bool succeeded = crypto::derive_recipient_session_key(
        recipient, prekey_secret, header.sender_identity,
        header.sender_ephemeral, root);
    crypto::wipe_bytes(prekey_secret);
    crypto::symmetric_key message_key{};
    if (succeeded) {
        succeeded = derive_first_message_key(root, message_key);
    }
    crypto::wipe_bytes(root);
    std::vector<std::uint8_t> plaintext;
    if (succeeded) {
        succeeded = crypto::open_dm_envelope(envelope, message_key, plaintext);
    }
    crypto::wipe_bytes(message_key);
    if (!succeeded) {
        error_out = "dechiffrement impossible : message altere, ou destine a "
                    "quelqu'un d'autre";
        return false;
    }
    if (!parse_dm_payload(plaintext, sent_at_out, text_out)) {
        error_out = "contenu du message illisible";
        return false;
    }
    return true;
}

} // namespace hypercom::client
