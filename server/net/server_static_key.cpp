#include "server/net/server_static_key.hpp"

#include "common/crypto/x25519_exchange.hpp"

#include <cerrno>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <unistd.h>

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

[[nodiscard]] bool write_all(int descriptor, std::uint8_t const *data,
                             std::size_t size)
{
    std::size_t written = 0;
    while (written < size) {
        ssize_t const result =
            ::write(descriptor, data + written, size - written);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        written += static_cast<std::size_t>(result);
    }
    return true;
}

// Opens the file at 0600 right from creation instead of chmod'ing it
// afterward. The server only builds on Linux, so a single POSIX path
// is enough here (see client/keystore/identity_store.cpp for the same
// fix with the Windows branch it also needs).
[[nodiscard]] bool write_secret_file(std::string const &path,
                                     crypto::x25519_secret_key const &secret,
                                     std::string &error_out)
{
    std::error_code failure;
    std::filesystem::path const target{path};
    if (target.has_parent_path()) {
        std::filesystem::create_directories(target.parent_path(), failure);
    }
    int const descriptor = ::open(path.c_str(), O_CREAT | O_TRUNC | O_WRONLY,
                                  0600);
    if (descriptor < 0) {
        error_out = "ecriture impossible : " + path;
        return false;
    }
    bool const ok = write_all(descriptor, secret.data(), secret.size());
    ::close(descriptor);
    if (!ok) {
        error_out = "ecriture incomplete : " + path;
        return false;
    }
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
