#include "client/keystore/identity_store.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

#include "common/crypto/keystore_envelope.hpp"
#include "common/crypto/secure_memory.hpp"

namespace hypercom::client {
namespace {

[[nodiscard]] bool write_sealed_file(std::string const &path,
                                     std::vector<std::uint8_t> const &sealed,
                                     std::string &error_out)
{
    std::error_code failure;
    std::filesystem::path const target{path};
    if (target.has_parent_path()) {
        std::filesystem::create_directories(target.parent_path(), failure);
    }
    std::ofstream output{path, std::ios::binary | std::ios::trunc};
    if (!output) {
        error_out = "ecriture impossible : " + path;
        return false;
    }
    output.write(reinterpret_cast<char const *>(sealed.data()),
                 static_cast<std::streamsize>(sealed.size()));
    if (!output) {
        error_out = "ecriture incomplete : " + path;
        return false;
    }
    std::filesystem::permissions(target,
                                 std::filesystem::perms::owner_read
                                     | std::filesystem::perms::owner_write,
                                 std::filesystem::perm_options::replace,
                                 failure);
    return true;
}

[[nodiscard]] bool read_whole_file(std::string const &path,
                                   std::vector<std::uint8_t> &out)
{
    std::ifstream input{path, std::ios::binary | std::ios::ate};
    if (!input) {
        return false;
    }
    auto const size = input.tellg();
    if (size <= 0) {
        return false;
    }
    out.resize(static_cast<std::size_t>(size));
    input.seekg(0);
    input.read(reinterpret_cast<char *>(out.data()),
               static_cast<std::streamsize>(out.size()));
    return static_cast<bool>(input);
}

} // namespace

identity_store::identity_store(std::string path) : path_{std::move(path)} {}

bool identity_store::has_stored_identity() const
{
    std::error_code failure;
    return std::filesystem::is_regular_file(path_, failure);
}

std::string const &identity_store::get_path() const
{
    return path_;
}

bool identity_store::create_identity(std::string_view passphrase,
                                     crypto::identity_keypair &out,
                                     std::string &error_out)
{
    if (has_stored_identity()) {
        error_out = "une identite existe deja dans " + path_
                    + " : l'ecraser reviendrait a perdre le compte associe, "
                      "sans aucun moyen de le recuperer";
        return false;
    }
    if (passphrase.empty()) {
        error_out = "une passphrase vide ne chiffre rien";
        return false;
    }
    if (!crypto::identity_keypair::generate_random(out)) {
        error_out = "generation de la paire de cles impossible";
        return false;
    }
    std::vector<std::uint8_t> sealed;
    if (!crypto::seal_identity_secret(passphrase, out.get_secret_key(),
                                      sealed)) {
        error_out = "scellement de la cle privee impossible";
        return false;
    }
    bool const written = write_sealed_file(path_, sealed, error_out);
    crypto::wipe_bytes(sealed);
    return written;
}

bool identity_store::unlock_identity(std::string_view passphrase,
                                     crypto::identity_keypair &out,
                                     std::string &error_out)
{
    std::vector<std::uint8_t> sealed;
    if (!read_whole_file(path_, sealed)) {
        error_out = "identite illisible : " + path_;
        return false;
    }
    crypto::ed25519_secret_key secret{};
    if (!crypto::open_identity_secret(passphrase, sealed, secret)) {
        // Le poly1305 ne distingue pas mauvaise passphrase et fichier altere,
        // et c'est tres bien ainsi : le message ne renseigne pas un attaquant.
        crypto::wipe_bytes(secret);
        error_out = "passphrase incorrecte ou fichier d'identite altere";
        return false;
    }
    // La graine Ed25519 occupe les 32 premiers octets de la cle privee
    // libsodium : elle suffit a reconstruire la paire complete.
    crypto::ed25519_seed seed{};
    std::copy_n(secret.begin(), seed.size(), seed.begin());
    bool const derived = crypto::identity_keypair::derive_from_seed(seed, out);
    crypto::wipe_bytes(seed);
    crypto::wipe_bytes(secret);
    if (!derived) {
        error_out = "reconstruction de la paire de cles impossible";
    }
    return derived;
}

} // namespace hypercom::client
