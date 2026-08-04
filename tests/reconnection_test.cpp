#include <cstdint>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

#include "client/net/server_connection.hpp"
#include "common/crypto/sodium_runtime.hpp"
#include "common/crypto/x25519_exchange.hpp"
#include "common/protocol/frame_codec.hpp"
#include "tests/loopback_noise_server.hpp"
#include "tests/test_harness.hpp"

// Reconnexion par re-handshake complet.
//
// Le choix retenu est de ne conserver AUCUN jeton de reprise : une reconnexion
// est une session neuve, avec une cle ephemere neuve. Le serveur n'a donc rien
// qui lui permette de recoudre deux connexions d'une meme personne.
//
// Ce que ce test verifie concretement : le meme objet server_connection peut
// etre rouvert apres une coupure. C'est le cas du client graphique, qui vit
// longtemps -- la CLI, elle, relance un processus par commande et n'a jamais
// exerce ce chemin.

namespace {

using namespace hypercom;

// Ouvre une session, envoie une trame, verifie l'echo. C'est le cycle complet
// d'une session, celui qu'on veut pouvoir rejouer a l'identique.
[[nodiscard]] bool run_one_session(client::server_connection &connection,
                                   std::uint16_t port, std::uint64_t token,
                                   std::string &error_out)
{
    if (!connection.open_session("127.0.0.1", port, error_out)) {
        return false;
    }
    std::vector<std::uint8_t> const payload{
        static_cast<std::uint8_t>(token), 0xAA, 0xBB};
    if (!connection.send_frame(proto::message_type::ping_request, payload)) {
        error_out = "envoi de la trame impossible";
        return false;
    }
    proto::frame_header header{};
    std::vector<std::uint8_t> received;
    if (!connection.receive_frame(header, received, error_out)) {
        return false;
    }
    if (header.type != proto::message_type::ping_request
        || received != payload) {
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
    HYPERCOM_CHECK(report,
                   crypto::generate_x25519_keypair(server_public,
                                                   server_secret));
    int listener = -1;
    std::uint16_t port = 0;
    HYPERCOM_CHECK(report, tests::open_loopback_listener(listener, port));
    tests::loopback_server_result outcome;
    std::thread server{tests::serve_noise_sessions, listener, server_public,
                       server_secret, 2, std::ref(outcome)};
    client::server_connection connection{server_public};
    std::string error;
    HYPERCOM_CHECK(report, run_one_session(connection, port, 1, error));
    // Le serveur a ferme : la connexion doit se savoir tombee, puis se rouvrir
    // sur le MEME objet. C'est precisement ce qui echouait avant, faute de
    // remise a zero du handshake et de la socket.
    HYPERCOM_CHECK(report, run_one_session(connection, port, 2, error));
    if (!error.empty()) {
        std::fputs(("  detail : " + error + "\n").c_str(), stderr);
    }
    server.join();
    HYPERCOM_CHECK(report, outcome.sessions_served == 2);
    HYPERCOM_CHECK(report, outcome.every_session_succeeded);
}

// Une cle epinglee differente doit faire echouer le handshake, et ne doit pas
// laisser la connexion se croire ouverte.
void check_wrong_pinned_key(tests::test_report &report)
{
    crypto::x25519_public_key server_public{};
    crypto::x25519_secret_key server_secret{};
    crypto::x25519_public_key impostor_public{};
    crypto::x25519_secret_key impostor_secret{};
    HYPERCOM_CHECK(report,
                   crypto::generate_x25519_keypair(server_public,
                                                   server_secret));
    HYPERCOM_CHECK(report,
                   crypto::generate_x25519_keypair(impostor_public,
                                                   impostor_secret));
    int listener = -1;
    std::uint16_t port = 0;
    HYPERCOM_CHECK(report, tests::open_loopback_listener(listener, port));
    tests::loopback_server_result outcome;
    std::thread server{tests::serve_noise_sessions, listener, server_public,
                       server_secret, 1, std::ref(outcome)};
    client::server_connection connection{impostor_public};
    std::string error;
    HYPERCOM_CHECK(report,
                   !connection.open_session("127.0.0.1", port, error));
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
