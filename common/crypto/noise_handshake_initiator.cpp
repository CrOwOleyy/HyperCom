#include "common/crypto/noise_handshake_initiator.hpp"

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

noise_handshake_initiator::noise_handshake_initiator(
    x25519_public_key const &server_static_public)
    : state_{}, remote_static_{server_static_public}, ephemeral_public_{},
      ephemeral_secret_{}, first_message_sent_{false}, complete_{false}
{
    initialize_symmetric_state(NOISE_PROTOCOL_NAME, state_);
    mix_hash(state_, as_bytes(NOISE_PROLOGUE));
    // Pre-message NK : la cle statique du repondeur entre dans le hachage des
    // deux cotes. Un client qui epingle la mauvaise cle divergera ici et
    // echouera au dechiffrement du second message, sans jamais rien reveler.
    mix_hash(state_, remote_static_);
}

bool noise_handshake_initiator::write_first_message(
    std::span<std::uint8_t const> payload, std::vector<std::uint8_t> &out)
{
    if (first_message_sent_) {
        return false;
    }
    if (!generate_x25519_keypair(ephemeral_public_, ephemeral_secret_)) {
        return false;
    }
    mix_hash(state_, ephemeral_public_);
    symmetric_key shared{};
    if (!compute_shared_secret(ephemeral_secret_, remote_static_, shared)) {
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
    out.assign(ephemeral_public_.begin(), ephemeral_public_.end());
    out.insert(out.end(), sealed_payload.begin(), sealed_payload.end());
    first_message_sent_ = true;
    return true;
}

bool noise_handshake_initiator::read_second_message(
    std::span<std::uint8_t const> input, std::vector<std::uint8_t> &payload_out)
{
    if (!first_message_sent_ || complete_) {
        return false;
    }
    if (input.size() < NOISE_HANDSHAKE_MESSAGE_TWO_SIZE) {
        return false;
    }
    x25519_public_key remote_ephemeral{};
    std::copy_n(input.begin(), remote_ephemeral.size(),
                remote_ephemeral.begin());
    mix_hash(state_, remote_ephemeral);
    symmetric_key shared{};
    bool const exchanged =
        compute_shared_secret(ephemeral_secret_, remote_ephemeral, shared);
    // La cle ephemere a joue son role : elle disparait immediatement, meme si
    // l'echange a echoue.
    wipe_bytes(ephemeral_secret_);
    if (!exchanged) {
        return false;
    }
    bool const mixed = mix_key(state_, shared);
    wipe_bytes(shared);
    if (!mixed) {
        return false;
    }
    if (!decrypt_and_hash(state_, input.subspan(remote_ephemeral.size()),
                          payload_out)) {
        return false;
    }
    complete_ = true;
    return true;
}

bool noise_handshake_initiator::export_transport_keys(
    symmetric_key &send_key, symmetric_key &receive_key) const
{
    if (!complete_) {
        return false;
    }
    return split_transport_keys(state_, send_key, receive_key);
}

bool noise_handshake_initiator::is_complete() const
{
    return complete_;
}

} // namespace hypercom::crypto
