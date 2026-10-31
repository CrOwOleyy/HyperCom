#include "client/keystore/master_seed_store.hpp"

#include "client/keystore/sealed_file.hpp"
#include "common/crypto/keystore_envelope.hpp"
#include "common/crypto/recovery_phrase.hpp"
#include "common/crypto/secure_memory.hpp"
#include "common/crypto/sodium_runtime.hpp"

#include <algorithm>
#include <filesystem>
#include <vector>

namespace hypercom::client {

master_seed_store::master_seed_store(std::string path) : path_{std::move(path)}
{}

bool master_seed_store::has_stored_seed() const
{
    std::error_code failure;
    return std::filesystem::is_regular_file(path_, failure);
}

bool master_seed_store::seal_to_disk(std::string_view passphrase,
                                     crypto::ed25519_seed const &seed,
                                     std::string &error_out)
{
    if (has_stored_seed()) {
        error_out = "une graine existe deja dans " + path_ +
                    " : l'ecraser reviendrait a perdre toutes les identites "
                    "qui en derivent, sur tous les serveurs";
        return false;
    }
    if (passphrase.empty()) {
        error_out = "passphrase vide";
        return false;
    }
    std::vector<std::uint8_t> sealed;
    if (!crypto::seal_blob(passphrase, seed, sealed)) {
        error_out = "scellement de la graine impossible";
        return false;
    }
    bool const written = write_sealed_file(path_, sealed, error_out);
    crypto::wipe_bytes(sealed);
    return written;
}

bool master_seed_store::create_seed(std::string_view passphrase,
                                    crypto::ed25519_seed &out,
                                    std::string &phrase_out,
                                    std::string &error_out)
{
    crypto::recovery_entropy entropy{};
    crypto::generate_recovery_entropy(entropy);
    std::string phrase;
    bool ready = crypto::derive_master_seed(entropy, out) &&
                 crypto::entropy_to_phrase(entropy, phrase);
    crypto::wipe_bytes(entropy);
    if (!ready) {
        error_out = "derivation de la graine impossible";
    } else if (!seal_to_disk(passphrase, out, error_out)) {
        ready = false;
    }
    if (ready) {
        phrase_out = std::move(phrase);
    } else {
        crypto::wipe_bytes(out);
    }
    return ready;
}

bool master_seed_store::restore_seed(std::string_view passphrase,
                                     std::string_view phrase,
                                     crypto::ed25519_seed &out,
                                     std::string &error_out)
{
    crypto::recovery_entropy entropy{};
    if (!crypto::phrase_to_entropy(phrase, entropy)) {
        error_out = "phrase de recuperation invalide : 12 mots attendus, "
                    "verifier l'orthographe et l'ordre";
        return false;
    }
    bool ready = crypto::derive_master_seed(entropy, out);
    crypto::wipe_bytes(entropy);
    if (!ready) {
        error_out = "derivation de la graine impossible";
    } else if (!seal_to_disk(passphrase, out, error_out)) {
        ready = false;
    }
    if (!ready) {
        crypto::wipe_bytes(out);
    }
    return ready;
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
    // A wrong passphrase and a tampered file produce the same failure:
    // poly1305 doesn't distinguish them, and there's nothing to gain by
    // pretending otherwise.
    bool const opened = crypto::open_blob(passphrase, sealed, plaintext) &&
                        plaintext.size() == out.size();
    if (opened) {
        std::copy(plaintext.begin(), plaintext.end(), out.begin());
    } else {
        error_out = "passphrase incorrecte ou graine alteree";
    }
    crypto::wipe_bytes(plaintext);
    return opened;
}

} // namespace hypercom::client
