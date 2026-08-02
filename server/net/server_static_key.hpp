#pragma once

#include <string>

#include "common/crypto/key_types.hpp"

namespace hypercom::server {

// Paire statique X25519 du serveur, celle que le client epingle (motif NK).
//
// Elle est generee au premier demarrage si le fichier n'existe pas, avec les
// droits 0600. Cette cle est l'equivalent d'un certificat serveur : la
// remplacer casse l'epinglage de tous les clients, exactement comme prevu --
// c'est ce qui rend une substitution de serveur visible plutot que silencieuse.
[[nodiscard]] bool load_or_create_server_key(
    std::string const &path, crypto::x25519_public_key &public_key,
    crypto::x25519_secret_key &secret_key, std::string &error_out);

} // namespace hypercom::server
