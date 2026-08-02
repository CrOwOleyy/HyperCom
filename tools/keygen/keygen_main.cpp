#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "common/crypto/identity_keypair.hpp"
#include "common/crypto/keystore_envelope.hpp"
#include "common/crypto/secure_memory.hpp"
#include "common/crypto/sodium_runtime.hpp"
#include "common/crypto/x25519_exchange.hpp"
#include "common/util/hex_codec.hpp"

// Generation de cles hors ligne.
//
//   hypercom_keygen server <chemin>     paire statique X25519 du serveur
//   hypercom_keygen identity <chemin>   identite Ed25519 chiffree
//
// Sert surtout a preparer une machine avant deploiement, sans lancer le
// serveur ni le client.

namespace {

using namespace hypercom;

[[nodiscard]] bool write_binary_file(std::string const &path,
                                     std::vector<std::uint8_t> const &content)
{
    std::ofstream output{path, std::ios::binary | std::ios::trunc};
    if (!output) {
        return false;
    }
    output.write(reinterpret_cast<char const *>(content.data()),
                 static_cast<std::streamsize>(content.size()));
    return static_cast<bool>(output);
}

[[nodiscard]] int generate_server_key(std::string const &path)
{
    crypto::x25519_public_key public_key{};
    crypto::x25519_secret_key secret_key{};
    if (!crypto::generate_x25519_keypair(public_key, secret_key)) {
        std::cerr << "generation impossible\n";
        return 1;
    }
    std::vector<std::uint8_t> const content{secret_key.begin(),
                                            secret_key.end()};
    if (!write_binary_file(path, content)) {
        std::cerr << "ecriture impossible : " << path << '\n';
        return 1;
    }
    std::string encoded;
    util::encode_hex(public_key, encoded);
    std::cout << "cle privee ecrite dans " << path
              << "\nMETTRE LES DROITS A 0600 : chmod 600 " << path
              << "\n\ncle publique a communiquer aux clients :\n"
              << encoded
              << "\n\nElle doit voyager par un canal de confiance. La faire "
                 "recuperer\ndepuis le serveur lui-meme annulerait tout "
                 "l'interet de l'epinglage.\n";
    return 0;
}

[[nodiscard]] int generate_identity(std::string const &path)
{
    std::cout << "passphrase : " << std::flush;
    std::string passphrase;
    if (!std::getline(std::cin, passphrase) || passphrase.empty()) {
        std::cerr << "passphrase requise\n";
        return 1;
    }
    crypto::identity_keypair identity;
    if (!crypto::identity_keypair::generate_random(identity)) {
        std::cerr << "generation impossible\n";
        return 1;
    }
    std::vector<std::uint8_t> sealed;
    if (!crypto::seal_identity_secret(passphrase, identity.get_secret_key(),
                                      sealed)
        || !write_binary_file(path, sealed)) {
        std::cerr << "ecriture impossible : " << path << '\n';
        return 1;
    }
    crypto::wipe_bytes(sealed);
    std::string encoded;
    util::encode_hex(identity.get_public_key(), encoded);
    std::cout << "identite ecrite dans " << path << "\ncle publique :\n"
              << encoded
              << "\n\nCette cle EST le compte. Perdue, elle ne se recupere "
                 "pas :\naucun administrateur ne peut la reinitialiser.\n";
    return 0;
}

} // namespace

int main(int argc, char **argv)
{
    if (!crypto::initialize_sodium()) {
        std::cerr << "libsodium n'a pas pu s'initialiser\n";
        return 1;
    }
    if (argc < 3) {
        std::cerr << "usage : hypercom_keygen <server|identity> <chemin>\n";
        return 2;
    }
    std::string const mode{argv[1]};
    std::string const path{argv[2]};
    if (mode == "server") {
        return generate_server_key(path);
    }
    if (mode == "identity") {
        return generate_identity(path);
    }
    std::cerr << "mode inconnu : " << mode << '\n';
    return 2;
}
