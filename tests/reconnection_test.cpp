#include "client/net/server_connection.hpp"
#include "common/crypto/sodium_runtime.hpp"
#include "common/crypto/x25519_exchange.hpp"
#include "common/protocol/frame_codec.hpp"
#include "tests/loopback_noise_server.hpp"
#include "tests/test_harness.hpp"

#include <cstdint>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

// Reconnection via a full re-handshake.
//
// The chosen approach keeps NO resumption token: a reconnection is a brand
// new session, with a brand new ephemeral key. So the server has nothing
// that would let it stitch two connections from the same person together.
//
// What this test concretely checks: the same server_connection object can
// be reopened after a disconnect. That's the case for the graphical
// client, which is long-lived -- the CLI, on the other hand, spawns a new
// process per command and has never exercised this path.

namespace {

using namespace hypercom;

// Opens a session, sends a frame, checks the echo. This is the full cycle
// of a session, the one we want to be able to replay identically.
[[nodiscard]] bool run_one_session(client::server_connection &connection,
                                   std::uint16_t port, std::uint64_t token,
                                   std::string &error_out)
{
    client::server_endpoint const endpoint{
        .host = "127.0.0.1", .port = port, .socks5_host = "", .socks5_port = 0};
    if (!connection.open_session(endpoint, error_out)) {
        return false;
    }
    std::vector<std::uint8_t> const payload{static_cast<std::uint8_t>(token),
                                            0xAA, 0xBB};
    if (!connection.send_frame(proto::message_type::ping_request, payload)) {
        error_out = "envoi de la trame impossible";
        return false;
    }
    proto::frame_header header{};
    std::vector<std::uint8_t> received;
    if (!connection.receive_frame(header, received, error_out)) {
        return false;
    }
    if (header.type != proto::message_type::ping_request ||
        received != payload) {
        error_out = "echo different de ce qui a ete envoye";
        return false;
    }
    return true;
}

void check_reconnection(tests::test_report &report)
{
    HYPERCOM_CHECK(report, crypto::initialize_sodium());
    crypto::x25519_public_key server_public{};
    crypto::x25519_secret_key server_secret{};
    HYPERCOM_CHECK(
        report, crypto::generate_x25519_keypair(server_public, server_secret));
    int listener = -1;
    std::uint16_t port = 0;
    HYPERCOM_CHECK(report, tests::open_loopback_listener(listener, port));
    tests::loopback_server_result outcome;
    std::thread server{tests::serve_noise_sessions,
                       listener,
                       server_public,
                       server_secret,
                       2,
                       std::ref(outcome)};
    client::server_connection connection{server_public};
    std::string error;
    HYPERCOM_CHECK(report, run_one_session(connection, port, 1, error));
    // The server closed: the connection must know it's down, then reopen
    // on the SAME object. This is exactly what used to fail, for lack of
    // resetting the handshake and the socket.
    HYPERCOM_CHECK(report, run_one_session(connection, port, 2, error));
    if (!error.empty()) {
        std::fputs(("  detail : " + error + "\n").c_str(), stderr);
    }
    server.join();
    HYPERCOM_CHECK(report, outcome.sessions_served == 2);
    HYPERCOM_CHECK(report, outcome.every_session_succeeded);
}

// A different pinned key must make the handshake fail, and must not leave
// the connection believing it's open.
void check_wrong_pinned_key(tests::test_report &report)
{
    crypto::x25519_public_key server_public{};
    crypto::x25519_secret_key server_secret{};
    crypto::x25519_public_key impostor_public{};
    crypto::x25519_secret_key impostor_secret{};
    HYPERCOM_CHECK(
        report, crypto::generate_x25519_keypair(server_public, server_secret));
    HYPERCOM_CHECK(report, crypto::generate_x25519_keypair(impostor_public,
                                                           impostor_secret));
    int listener = -1;
    std::uint16_t port = 0;
    HYPERCOM_CHECK(report, tests::open_loopback_listener(listener, port));
    tests::loopback_server_result outcome;
    std::thread server{tests::serve_noise_sessions,
                       listener,
                       server_public,
                       server_secret,
                       1,
                       std::ref(outcome)};
    client::server_connection connection{impostor_public};
    client::server_endpoint const endpoint{
        .host = "127.0.0.1", .port = port, .socks5_host = "", .socks5_port = 0};
    std::string error;
    HYPERCOM_CHECK(report, !connection.open_session(endpoint, error));
    HYPERCOM_CHECK(report, !connection.is_open());
    server.join();
}

} // namespace

int main()
{
    hypercom::tests::test_report report;
    check_reconnection(report);
    check_wrong_pinned_key(report);
    return report.summarize("reconnexion");
}
