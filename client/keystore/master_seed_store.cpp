#include "client/keystore/master_seed_store.hpp"

#include <algorithm>
#include <filesystem>
#include <vector>

#include "client/keystore/sealed_file.hpp"
#include "common/crypto/keystore_envelope.hpp"
#include "common/crypto/secure_memory.hpp"
#include "common/crypto/sodium_runtime.hpp"

namespace hypercom::client {

master_seed_store::master_seed_store(std::string path)
    : path_{std::move(path)}
{
}

bool master_seed_store::has_stored_seed() const
{
    std::error_code failure;
    return std::filesystem::is_regular_file(path_, failure);
}

bool master_seed_store::create_seed(std::string_view passphrase,
                                    crypto::ed25519_seed &out,
                                    std::string &error_out)
{
    if (has_stored_seed()) {
        error_out = "une graine existe deja dans " + path_
                    + " : l'ecraser reviendrait a perdre toutes les identites "
                      "qui en derivent, sur tous les serveurs";
        return false;
    }
    if (passphrase.empty()) {
        error_out = "passphrase vide";
        return false;
    }
    crypto::fill_random_bytes(out);
    std::vector<std::uint8_t> sealed;
    if (!crypto::seal_blob(passphrase, out, sealed)) {
        crypto::wipe_bytes(out);
        error_out = "scellement de la graine impossible";
        return false;
    }
    bool const written = write_sealed_file(path_, sealed, error_out);
    crypto::wipe_bytes(sealed);
    if (!written) {
        crypto::wipe_bytes(out);
    }
    return written;
}

bool master_seed_store::unlock_seed(std::string_view passphrase,
                                    crypto::ed25519_seed &out,
                                    std::string &error_out)
{
    std::vector<std::uint8_t> sealed;
    if (!read_sealed_file(path_, sealed)) {
        error_out = "graine illisible : " + path_;
        return false;
    }
    std::vector<std::uint8_t> plaintext;
    // Passphrase fausse et fichier altere donnent le meme echec : le poly1305
    // ne les distingue pas, et il n'y a rien a gagner a le faire croire.
    bool const opened = crypto::open_blob(passphrase, sealed, plaintext)
                        && plaintext.size() == out.size();
    if (opened) {
        std::copy(plaintext.begin(), plaintext.end(), out.begin());
    } else {
        error_out = "passphrase incorrecte ou graine alteree";
    }
    crypto::wipe_bytes(plaintext);
    return opened;
}

} // namespace hypercom::client
