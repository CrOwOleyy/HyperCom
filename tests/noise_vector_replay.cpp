#include "tests/noise_vector_replay.hpp"

#include "common/crypto/noise_cipher_state.hpp"
#include "common/crypto/noise_payload_codec.hpp"
#include "common/crypto/x25519_exchange.hpp"
#include "tests/noise_official_vector_data.hpp"

namespace hypercom::tests {
namespace {

namespace vector = noise_vector;

// Reproduit noise_handshake_initiator::write_first_message, avec une cle
// ephemere imposee plutot que tiree au hasard.
[[nodiscard]] std::vector<std::uint8_t> run_initiator_first_message(
    test_report &report, crypto::noise_symmetric_state &state,
    crypto::x25519_secret_key const &ephemeral_secret,
    crypto::x25519_public_key const &responder_static_public,
    std::span<std::uint8_t const> payload)
{
    crypto::x25519_public_key ephemeral_public{};
    HYPERCOM_CHECK(report, crypto::compute_public_from_secret(
                               ephemeral_secret, ephemeral_public));
    mix_hash(state, ephemeral_public);
    crypto::symmetric_key shared{};
    HYPERCOM_CHECK(report, crypto::compute_shared_secret(
                               ephemeral_secret, responder_static_public,
                               shared));
    HYPERCOM_CHECK(report, mix_key(state, shared));
    std::vector<std::uint8_t> sealed;
    HYPERCOM_CHECK(report, encrypt_and_hash(state, payload, sealed));
    std::vector<std::uint8_t> wire{ephemeral_public.begin(),
                                   ephemeral_public.end()};
    wire.insert(wire.end(), sealed.begin(), sealed.end());
    return wire;
}

// Reproduit noise_handshake_responder::read_first_message.
void run_responder_read_first(test_report &report,
                              crypto::noise_symmetric_state &state,
                              crypto::x25519_secret_key const &static_secret,
                              std::span<std::uint8_t const> wire,
                              crypto::x25519_public_key &remote_ephemeral_out,
                              std::vector<std::uint8_t> &payload_out)
{
    std::copy_n(wire.begin(), remote_ephemeral_out.size(),
               remote_ephemeral_out.begin());
    mix_hash(state, remote_ephemeral_out);
    crypto::symmetric_key shared{};
    HYPERCOM_CHECK(report, crypto::compute_shared_secret(
                               static_secret, remote_ephemeral_out, shared));
    HYPERCOM_CHECK(report, mix_key(state, shared));
    HYPERCOM_CHECK(report,
                   decrypt_and_hash(
                       state, wire.subspan(remote_ephemeral_out.size()),
                       payload_out));
}

// Reproduit noise_handshake_responder::write_second_message.
[[nodiscard]] std::vector<std::uint8_t> run_responder_second_message(
    test_report &report, crypto::noise_symmetric_state &state,
    crypto::x25519_secret_key const &ephemeral_secret,
    crypto::x25519_public_key const &remote_ephemeral,
    std::span<std::uint8_t const> payload)
{
    crypto::x25519_public_key ephemeral_public{};
    HYPERCOM_CHECK(report, crypto::compute_public_from_secret(
                               ephemeral_secret, ephemeral_public));
    mix_hash(state, ephemeral_public);
    crypto::symmetric_key shared{};
    HYPERCOM_CHECK(report, crypto::compute_shared_secret(
                               ephemeral_secret, remote_ephemeral, shared));
    HYPERCOM_CHECK(report, mix_key(state, shared));
    std::vector<std::uint8_t> sealed;
    HYPERCOM_CHECK(report, encrypt_and_hash(state, payload, sealed));
    std::vector<std::uint8_t> wire{ephemeral_public.begin(),
                                   ephemeral_public.end()};
    wire.insert(wire.end(), sealed.begin(), sealed.end());
    return wire;
}

// Reproduit noise_handshake_initiator::read_second_message.
void run_initiator_read_second(test_report &report,
                               crypto::noise_symmetric_state &state,
                               crypto::x25519_secret_key const &ephemeral_secret,
                               std::span<std::uint8_t const> wire,
                               std::vector<std::uint8_t> &payload_out)
{
    crypto::x25519_public_key remote_ephemeral{};
    std::copy_n(wire.begin(), remote_ephemeral.size(),
               remote_ephemeral.begin());
    mix_hash(state, remote_ephemeral);
    crypto::symmetric_key shared{};
    HYPERCOM_CHECK(report, crypto::compute_shared_secret(
                               ephemeral_secret, remote_ephemeral, shared));
    HYPERCOM_CHECK(report, mix_key(state, shared));
    HYPERCOM_CHECK(report,
                   decrypt_and_hash(state, wire.subspan(remote_ephemeral.size()),
                                    payload_out));
}

// Une seule direction de transport : chiffre chez l'emetteur, dechiffre chez
// le destinataire, compare les deux au vecteur.
void check_transport_message(test_report &report,
                             crypto::noise_cipher_state &sender,
                             crypto::noise_cipher_state &receiver,
                             vector::wire_message const &expected)
{
    std::vector<std::uint8_t> const payload =
        load_bytes(report, expected.payload_hex);
    std::vector<std::uint8_t> const expected_wire =
        load_bytes(report, expected.ciphertext_hex);
    std::vector<std::uint8_t> sealed;
    HYPERCOM_CHECK(report, sender.encrypt_with_ad({}, payload, sealed));
    HYPERCOM_CHECK(report, sealed == expected_wire);
    std::vector<std::uint8_t> opened;
    HYPERCOM_CHECK(report,
                   receiver.decrypt_with_ad({}, expected_wire, opened));
    HYPERCOM_CHECK(report, opened == payload);
}

} // namespace

std::vector<std::uint8_t> load_bytes(test_report &report,
                                     std::string_view hex)
{
    std::vector<std::uint8_t> decoded;
    HYPERCOM_CHECK(report, util::decode_hex(hex, decoded));
    return decoded;
}

loaded_vector_keys load_vector_keys(test_report &report)
{
    loaded_vector_keys keys;
    load_fixed(report, vector::INIT_EPHEMERAL_SECRET_HEX,
              keys.init_ephemeral_secret);
    load_fixed(report, vector::RESP_STATIC_SECRET_HEX,
              keys.resp_static_secret);
    load_fixed(report, vector::RESP_EPHEMERAL_SECRET_HEX,
              keys.resp_ephemeral_secret);
    load_fixed(report, vector::EXPECTED_HANDSHAKE_HASH_HEX,
              keys.expected_handshake_hash);
    keys.prologue = load_bytes(report, vector::PROLOGUE_HEX);
    crypto::x25519_public_key expected_static_public{};
    load_fixed(report, vector::EXPECTED_RESP_STATIC_PUBLIC_HEX,
              expected_static_public);
    HYPERCOM_CHECK(report, crypto::compute_public_from_secret(
                               keys.resp_static_secret,
                               keys.resp_static_public));
    HYPERCOM_CHECK(report, keys.resp_static_public == expected_static_public);
    return keys;
}

handshake_result run_handshake(test_report &report,
                               loaded_vector_keys const &keys)
{
    handshake_result result;
    initialize_symmetric_state(vector::PROTOCOL_NAME, result.initiator);
    initialize_symmetric_state(vector::PROTOCOL_NAME, result.responder);
    mix_hash(result.initiator, keys.prologue);
    mix_hash(result.responder, keys.prologue);
    mix_hash(result.initiator, keys.resp_static_public);
    mix_hash(result.responder, keys.resp_static_public);
    std::vector<std::uint8_t> const message0_payload =
        load_bytes(report, vector::MESSAGES[0].payload_hex);
    std::vector<std::uint8_t> const message0_wire = run_initiator_first_message(
        report, result.initiator, keys.init_ephemeral_secret,
        keys.resp_static_public, message0_payload);
    HYPERCOM_CHECK(report, message0_wire == load_bytes(
                               report, vector::MESSAGES[0].ciphertext_hex));
    crypto::x25519_public_key remote_ephemeral{};
    std::vector<std::uint8_t> decoded_payload0;
    run_responder_read_first(report, result.responder, keys.resp_static_secret,
                             message0_wire, remote_ephemeral,
                             decoded_payload0);
    HYPERCOM_CHECK(report, decoded_payload0 == message0_payload);
    std::vector<std::uint8_t> const message1_payload =
        load_bytes(report, vector::MESSAGES[1].payload_hex);
    std::vector<std::uint8_t> const message1_wire =
        run_responder_second_message(report, result.responder,
                                     keys.resp_ephemeral_secret,
                                     remote_ephemeral, message1_payload);
    HYPERCOM_CHECK(report, message1_wire == load_bytes(
                               report, vector::MESSAGES[1].ciphertext_hex));
    std::vector<std::uint8_t> decoded_payload1;
    run_initiator_read_second(report, result.initiator,
                              keys.init_ephemeral_secret, message1_wire,
                              decoded_payload1);
    HYPERCOM_CHECK(report, decoded_payload1 == message1_payload);
    HYPERCOM_CHECK(report,
                   result.initiator.handshake_hash
                       == result.responder.handshake_hash);
    HYPERCOM_CHECK(report, result.initiator.handshake_hash
                              == keys.expected_handshake_hash);
    return result;
}

void check_transport(test_report &report, handshake_result const &handshake)
{
    crypto::symmetric_key init_send{};
    crypto::symmetric_key init_recv{};
    crypto::symmetric_key resp_recv{};
    crypto::symmetric_key resp_send{};
    HYPERCOM_CHECK(report, split_transport_keys(handshake.initiator, init_send,
                                                init_recv));
    HYPERCOM_CHECK(report, split_transport_keys(handshake.responder, resp_recv,
                                                resp_send));
    HYPERCOM_CHECK(report, init_send == resp_recv);
    HYPERCOM_CHECK(report, init_recv == resp_send);
    crypto::noise_cipher_state init_send_cipher;
    crypto::noise_cipher_state init_recv_cipher;
    crypto::noise_cipher_state resp_send_cipher;
    crypto::noise_cipher_state resp_recv_cipher;
    init_send_cipher.initialize_key(init_send);
    init_recv_cipher.initialize_key(init_recv);
    resp_send_cipher.initialize_key(resp_send);
    resp_recv_cipher.initialize_key(resp_recv);
    check_transport_message(report, init_send_cipher, resp_recv_cipher,
                            vector::MESSAGES[2]);
    check_transport_message(report, resp_send_cipher, init_recv_cipher,
                            vector::MESSAGES[3]);
    check_transport_message(report, init_send_cipher, resp_recv_cipher,
                            vector::MESSAGES[4]);
    check_transport_message(report, resp_send_cipher, init_recv_cipher,
                            vector::MESSAGES[5]);
}

} // namespace hypercom::tests
