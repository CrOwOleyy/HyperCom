#include "server/net/server_static_key.hpp"

#include "common/crypto/x25519_exchange.hpp"

#include <filesystem>
#include <fstream>
#include <sys/stat.h>

namespace hypercom::server {
namespace {

[[nodiscard]] bool read_secret_file(std::string const &path,
                                    crypto::x25519_secret_key &out)
{
    std::ifstream input{path, std::ios::binary};
    if (!input) {
        return false;
    }
    input.read(reinterpret_cast<char *>(out.data()),
               static_cast<std::streamsize>(out.size()));
    // A file shorter than expected is a corrupted file, not a key that
    // needs completing: it's rejected rather than deriving a truncated
    // identity.
    return input.gcount() == static_cast<std::streamsize>(out.size());
}

[[nodiscard]] bool write_secret_file(std::string const &path,
                                     crypto::x25519_secret_key const &secret,
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
    output.write(reinterpret_cast<char const *>(secret.data()),
                 static_cast<std::streamsize>(secret.size()));
    output.close();
#if defined(_WIN32)
    std::filesystem::permissions(target,
                                 std::filesystem::perms::owner_read |
                                     std::filesystem::perms::owner_write,
                                 failure);
#else
    if (::chmod(path.c_str(), S_IRUSR | S_IWUSR) != 0) {
        error_out = "chmod 0600 impossible sur " + path;
        return false;
    }
#endif
    return true;
}

} // namespace

bool load_or_create_server_key(std::string const &path,
                               crypto::x25519_public_key &public_key,
                               crypto::x25519_secret_key &secret_key,
                               std::string &error_out)
{
    if (read_secret_file(path, secret_key)) {
        // The public key is recomputed from the private one: it doesn't
        // need to be stored, so it can't get out of sync.
        if (!crypto::compute_public_from_secret(secret_key, public_key)) {
            error_out = "cle serveur illisible ou invalide : " + path;
            return false;
        }
        return true;
    }
    if (!crypto::generate_x25519_keypair(public_key, secret_key)) {
        error_out = "generation de la cle serveur impossible";
        return false;
    }
    return write_secret_file(path, secret_key, error_out);
}

} // namespace hypercom::server
