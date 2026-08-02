#pragma once

#include <cstddef>
#include <string_view>

#include "common/crypto/key_types.hpp"

namespace hypercom::crypto {

// Suite Noise retenue, en remplacement de TLS (BRIEF.md 5).
//
//   Noise_NK_25519_ChaChaPoly_SHA256
//
// NK : le serveur est authentifie par sa cle statique, connue du client a
// l'avance (epinglee ou saisie a la premiere connexion). Le client reste
// anonyme au niveau transport et s'authentifie ensuite au niveau applicatif.
// Ni X.509, ni autorite de certification, ni parseur de certificats.
//
// Le nom fait exactement 32 octets, soit HASHLEN : il est donc utilise tel quel
// comme etat de hachage initial, sans passer par SHA-256, conformement a la
// specification.
//
// COMPROMIS A CONNAITRE : ce code est une implementation maison d'un protocole
// specifie, pas une crypto maison -- les primitives viennent toutes de
// libsodium. Le risque residuel est une erreur dans l'enchainement des etapes,
// pas dans les primitives. C'est pour cela que noise_handshake_test verifie le
// deroule complet et que le sens des cles de transport est teste dans les deux
// directions. Une validation contre les vecteurs officiels Noise reste a faire
// et est notee dans docs/THREAT_MODEL.md.
constexpr std::string_view NOISE_PROTOCOL_NAME =
    "Noise_NK_25519_ChaChaPoly_SHA256";

// Le prologue est mixe dans le hachage par les deux pairs. Il lie la session a
// cette application et a cette version : un handshake Hypercom ne peut pas
// etre rejoue vers un autre service partageant la meme suite Noise.
constexpr std::string_view NOISE_PROLOGUE = "hypercom-v1";

constexpr std::size_t NOISE_HANDSHAKE_MESSAGE_ONE_SIZE =
    X25519_PUBLIC_KEY_SIZE + AEAD_TAG_SIZE;
constexpr std::size_t NOISE_HANDSHAKE_MESSAGE_TWO_SIZE =
    X25519_PUBLIC_KEY_SIZE + AEAD_TAG_SIZE;

} // namespace hypercom::crypto
