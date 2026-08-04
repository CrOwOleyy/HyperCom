#pragma once

#include <cstdint>

#include "common/crypto/key_types.hpp"

namespace hypercom::tests {

// Repondeur Noise minimal sur la boucle locale.
//
// Il ne parle pas le protocole applicatif : il etablit le canal, renvoie en
// echo une trame chiffree, puis ferme. C'est tout ce qu'il faut pour verifier
// qu'une session s'ouvre -- et surtout qu'elle se ROUVRE sur le meme objet
// client, ce qu'aucun test ne couvrait.
//
// POSIX seulement : le serveur du projet se developpe et se teste sous WSL2,
// et doubler ce fichier pour Winsock n'apporterait rien.

struct loopback_server_result {
    int sessions_served = 0;
    bool every_session_succeeded = true;
};

// Ouvre et met en ecoute une socket sur 127.0.0.1, port attribue par l'OS.
[[nodiscard]] bool open_loopback_listener(int &descriptor_out,
                                          std::uint16_t &port_out);

// Sert session_count sessions successives, puis ferme la socket d'ecoute.
// Prevu pour tourner dans un thread pendant que le client se connecte.
void serve_noise_sessions(int listener_descriptor,
                          crypto::x25519_public_key const &static_public,
                          crypto::x25519_secret_key const &static_secret,
                          int session_count,
                          loopback_server_result &result);

} // namespace hypercom::tests
