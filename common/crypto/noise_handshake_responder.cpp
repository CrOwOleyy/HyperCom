#include "common/crypto/noise_handshake_responder.hpp"

#include <algorithm>
#include <string_view>

#include "common/crypto/noise_parameters.hpp"
#include "common/crypto/noise_payload_codec.hpp"
#include "common/crypto/secure_memory.hpp"
#include "common/crypto/x25519_exchange.hpp"

namespace hypercom::crypto {
namespace {

[[nodiscard]] std::span<std::uint8_t const> as_bytes(std::string_view text)
{
    return {reinterpret_cast<std::uint8_t const *>(text.data()), text.size()};
}

} // namespace

noise_handshake_responder::noise_handshake_responder(
    x25519_public_key const &static_public,
    x25519_secret_key const &static_secret)
    : state_{}, static_secret_{static_secret}, remote_ephemeral_{},
      first_message_read_{false}, complete_{false}
{
    initialize_symmetric_state(NOISE_PROTOCOL_NAME, state_);
    mix_hash(state_, as_bytes(NOISE_PROLOGUE));
    mix_hash(state_, static_public);
}

bool noise_handshake_responder::read_first_message(
    std::span<std::uint8_t const> input, std::vector<std::uint8_t> &payload_out)
{
    if (first_message_read_) {
        return false;
    }
    if (input.size() < NOISE_HANDSHAKE_MESSAGE_ONE_SIZE) {
        return false;
    }
    std::copy_n(input.begin(), remote_ephemeral_.size(),
                remote_ephemeral_.begin());
    mix_hash(state_, remote_ephemeral_);
    symmetric_key shared{};
    if (!compute_shared_secret(static_secret_, remote_ephemeral_, shared)) {
        return false;
    }
    bool const mixed = mix_key(state_, shared);
    wipe_bytes(shared);
    if (!mixed) {
        return false;
    }
    if (!decrypt_and_hash(state_, input.subspan(remote_ephemeral_.size()),
                          payload_out)) {
        return false;
    }
    first_message_read_ = true;
    return true;
}

bool noise_handshake_responder::write_second_message(
    std::span<std::uint8_t const> payload, std::vector<std::uint8_t> &out)
{
    if (!first_message_read_ || complete_) {
        return false;
    }
    x25519_public_key ephemeral_public{};
    x25519_secret_key ephemeral_secret{};
    if (!generate_x25519_keypair(ephemeral_public, ephemeral_secret)) {
        return false;
    }
    mix_hash(state_, ephemeral_public);
    symmetric_key shared{};
    bool const exchanged =
        compute_shared_secret(ephemeral_secret, remote_ephemeral_, shared);
    wipe_bytes(ephemeral_secret);
    if (!exchanged) {
        return false;
    }
    bool const mixed = mix_key(state_, shared);
    wipe_bytes(shared);
    if (!mixed) {
        return false;
    }
    std::vector<std::uint8_t> sealed_payload;
    if (!encrypt_and_hash(state_, payload, sealed_payload)) {
        return false;
    }
    out.assign(ephemeral_public.begin(), ephemeral_public.end());
    out.insert(out.end(), sealed_payload.begin(), sealed_payload.end());
    complete_ = true;
    return true;
}

bool noise_handshake_responder::export_transport_keys(
    symmetric_key &send_key, symmetric_key &receive_key) const
{
    if (!complete_) {
        return false;
    }
    // Ordre inverse de l'initiateur : sa cle d'emission est notre cle de
    // reception. Se tromper ici produit un canal qui s'etablit puis echoue au
    // premier message, d'ou le test bidirectionnel dans tests/.
    return split_transport_keys(state_, receive_key, send_key);
}

bool noise_handshake_responder::is_complete() const
{
    return complete_;
}

} // namespace hypercom::crypto
