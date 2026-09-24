#include "common/crypto/dm_envelope.hpp"
#include "common/crypto/dm_message_chain.hpp"
#include "common/crypto/dm_session_keys.hpp"
#include "common/crypto/identity_keypair.hpp"
#include "common/crypto/keystore_envelope.hpp"
#include "common/crypto/noise_handshake_initiator.hpp"
#include "common/crypto/noise_handshake_responder.hpp"
#include "common/crypto/noise_transport.hpp"
#include "common/crypto/signature_verifier.hpp"
#include "common/crypto/sodium_runtime.hpp"
#include "common/crypto/x25519_exchange.hpp"
#include "tests/test_harness.hpp"

#include <string>
#include <vector>

namespace {

using namespace hypercom;

[[nodiscard]] std::span<std::uint8_t const> as_bytes(std::string_view text)
{
    return {reinterpret_cast<std::uint8_t const *>(text.data()), text.size()};
}

// The full handshake, then a message in EACH direction. Testing only one
// direction would let a transport key swap slip through, which is the
// easiest mistake to make in Split().
void check_noise_handshake(tests::test_report &report)
{
    crypto::x25519_public_key server_public{};
    crypto::x25519_secret_key server_secret{};
    HYPERCOM_CHECK(
        report, crypto::generate_x25519_keypair(server_public, server_secret));
    crypto::noise_handshake_initiator client{server_public};
    crypto::noise_handshake_responder server{server_public, server_secret};
    std::vector<std::uint8_t> first;
    std::vector<std::uint8_t> received;
    HYPERCOM_CHECK(report, client.write_first_message({}, first));
    HYPERCOM_CHECK(report, server.read_first_message(first, received));
    std::vector<std::uint8_t> second;
    HYPERCOM_CHECK(report, server.write_second_message({}, second));
    HYPERCOM_CHECK(report, client.read_second_message(second, received));
    HYPERCOM_CHECK(report, client.is_complete() && server.is_complete());
    crypto::symmetric_key client_send{};
    crypto::symmetric_key client_receive{};
    crypto::symmetric_key server_send{};
    crypto::symmetric_key server_receive{};
    HYPERCOM_CHECK(report,
                   client.export_transport_keys(client_send, client_receive));
    HYPERCOM_CHECK(report,
                   server.export_transport_keys(server_send, server_receive));
    crypto::noise_transport client_channel{client_send, client_receive};
    crypto::noise_transport server_channel{server_send, server_receive};
    std::vector<std::uint8_t> sealed;
    std::vector<std::uint8_t> opened;
    HYPERCOM_CHECK(report, client_channel.encrypt_message(
                               as_bytes("vers le serveur"), sealed));
    HYPERCOM_CHECK(report, server_channel.decrypt_message(sealed, opened));
    HYPERCOM_CHECK(report, std::string(opened.begin(), opened.end()) ==
                               "vers le serveur");
    HYPERCOM_CHECK(report, server_channel.encrypt_message(
                               as_bytes("vers le client"), sealed));
    HYPERCOM_CHECK(report, client_channel.decrypt_message(sealed, opened));
    HYPERCOM_CHECK(report, std::string(opened.begin(), opened.end()) ==
                               "vers le client");
}

// A client that pins the wrong key must NOT establish a session.
void check_wrong_pinned_key_is_refused(tests::test_report &report)
{
    crypto::x25519_public_key real_public{};
    crypto::x25519_secret_key real_secret{};
    crypto::x25519_public_key impostor_public{};
    crypto::x25519_secret_key impostor_secret{};
    HYPERCOM_CHECK(report,
                   crypto::generate_x25519_keypair(real_public, real_secret));
    HYPERCOM_CHECK(report, crypto::generate_x25519_keypair(impostor_public,
                                                           impostor_secret));
    crypto::noise_handshake_initiator client{impostor_public};
    crypto::noise_handshake_responder server{real_public, real_secret};
    std::vector<std::uint8_t> first;
    std::vector<std::uint8_t> payload;
    HYPERCOM_CHECK(report, client.write_first_message({}, first));
    // The server can't even open the first message: the handshake hash
    // already diverges at the pre-message.
    HYPERCOM_CHECK(report, !server.read_first_message(first, payload));
}

// Both sides must derive the same root, with no prior exchange at all.
void check_dm_round_trip(tests::test_report &report)
{
    crypto::identity_keypair alice;
    crypto::identity_keypair bob;
    HYPERCOM_CHECK(report, crypto::identity_keypair::generate_random(alice));
    HYPERCOM_CHECK(report, crypto::identity_keypair::generate_random(bob));
    crypto::x25519_public_key bob_prekey_public{};
    crypto::x25519_secret_key bob_prekey_secret{};
    HYPERCOM_CHECK(report, crypto::generate_x25519_keypair(bob_prekey_public,
                                                           bob_prekey_secret));
    crypto::x25519_public_key ephemeral_public{};
    crypto::x25519_secret_key ephemeral_secret{};
    HYPERCOM_CHECK(report, crypto::generate_x25519_keypair(ephemeral_public,
                                                           ephemeral_secret));
    crypto::symmetric_key sender_root{};
    crypto::symmetric_key recipient_root{};
    HYPERCOM_CHECK(report, crypto::derive_sender_session_key(
                               alice, ephemeral_secret, bob.get_public_key(),
                               bob_prekey_public, sender_root));
    HYPERCOM_CHECK(report, crypto::derive_recipient_session_key(
                               bob, bob_prekey_secret, alice.get_public_key(),
                               ephemeral_public, recipient_root));
    HYPERCOM_CHECK(report, sender_root == recipient_root);
    crypto::dm_message_chain sender_chain{sender_root};
    crypto::dm_message_chain recipient_chain{recipient_root};
    crypto::symmetric_key sender_key{};
    crypto::symmetric_key recipient_key{};
    HYPERCOM_CHECK(report, sender_chain.derive_next_message_key(sender_key));
    HYPERCOM_CHECK(report,
                   recipient_chain.derive_next_message_key(recipient_key));
    HYPERCOM_CHECK(report, sender_key == recipient_key);
    crypto::dm_envelope_header header;
    header.sender_identity = alice.get_public_key();
    header.sender_ephemeral = ephemeral_public;
    std::vector<std::uint8_t> envelope;
    HYPERCOM_CHECK(report,
                   crypto::seal_dm_envelope(header, sender_key,
                                            as_bytes("secret"), envelope));
    std::vector<std::uint8_t> opened;
    HYPERCOM_CHECK(report,
                   crypto::open_dm_envelope(envelope, recipient_key, opened));
    HYPERCOM_CHECK(report,
                   std::string(opened.begin(), opened.end()) == "secret");
    // A single modified byte in the authenticated header must make opening
    // fail: the server can't replay it under a different counter.
    envelope[1] ^= 0x01;
    HYPERCOM_CHECK(report,
                   !crypto::open_dm_envelope(envelope, recipient_key, opened));
}

// The previous chain key must vanish with every step forward.
void check_forward_secrecy_of_chain(tests::test_report &report)
{
    crypto::symmetric_key root{};
    crypto::fill_random_bytes(root);
    crypto::dm_message_chain chain{root};
    crypto::symmetric_key const first_chain_key = chain.get_chain_key();
    crypto::symmetric_key message_key{};
    HYPERCOM_CHECK(report, chain.derive_next_message_key(message_key));
    HYPERCOM_CHECK(report, chain.get_chain_key() != first_chain_key);
    HYPERCOM_CHECK(report, chain.get_counter() == 1);
}

void check_keystore_round_trip(tests::test_report &report)
{
    crypto::identity_keypair identity;
    HYPERCOM_CHECK(report, crypto::identity_keypair::generate_random(identity));
    std::vector<std::uint8_t> sealed;
    HYPERCOM_CHECK(report,
                   crypto::seal_identity_secret(
                       "bonne passphrase", identity.get_secret_key(), sealed));
    crypto::ed25519_secret_key recovered{};
    HYPERCOM_CHECK(report, crypto::open_identity_secret("bonne passphrase",
                                                        sealed, recovered));
    HYPERCOM_CHECK(report, recovered == identity.get_secret_key());
    HYPERCOM_CHECK(
        report, !crypto::open_identity_secret("mauvaise", sealed, recovered));
    sealed[sealed.size() - 1] ^= 0x01;
    HYPERCOM_CHECK(report, !crypto::open_identity_secret("bonne passphrase",
                                                         sealed, recovered));
}

void check_signature_verification(tests::test_report &report)
{
    crypto::identity_keypair identity;
    HYPERCOM_CHECK(report, crypto::identity_keypair::generate_random(identity));
    crypto::ed25519_signature signature{};
    HYPERCOM_CHECK(report, identity.sign_message(as_bytes("defi"), signature));
    HYPERCOM_CHECK(report,
                   crypto::verify_signature(identity.get_public_key(),
                                            as_bytes("defi"), signature));
    HYPERCOM_CHECK(report, !crypto::verify_signature(identity.get_public_key(),
                                                     as_bytes("autre defi"),
                                                     signature));
}

} // namespace

int main()
{
    if (!hypercom::crypto::initialize_sodium()) {
        return 1;
    }
    hypercom::tests::test_report report;
    check_noise_handshake(report);
    check_wrong_pinned_key_is_refused(report);
    check_dm_round_trip(report);
    check_forward_secrecy_of_chain(report);
    check_keystore_round_trip(report);
    check_signature_verification(report);
    return report.summarize("cryptographie");
}
