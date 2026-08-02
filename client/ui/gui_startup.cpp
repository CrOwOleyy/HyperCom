#include "client/ui/gui_startup.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

#include "client/keystore/identity_store.hpp"
#include "common/util/hex_codec.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace hypercom::client {

bool prepare_session(cli_options const &options,
                     crypto::identity_keypair &identity,
                     crypto::x25519_public_key &server_key,
                     std::string &error_out)
{
    std::vector<std::uint8_t> decoded;
    if (!util::decode_hex(options.server_key_hex, decoded)
        || decoded.size() != server_key.size()) {
        error_out = "--server-key doit faire 64 caracteres hexadecimaux.\n\n"
                    "Le serveur affiche cette cle au demarrage.";
        return false;
    }
    std::copy(decoded.begin(), decoded.end(), server_key.begin());
    identity_store store{options.identity_path};
    std::string passphrase;
    if (!read_passphrase(passphrase, error_out)) {
        return false;
    }
    if (store.has_stored_identity()) {
        return store.unlock_identity(passphrase, identity, error_out);
    }
    return store.create_identity(passphrase, identity, error_out);
}

void report_startup_failure(std::string const &message)
{
#if defined(_WIN32)
    std::string const body = message
                             + "\n\nUsage :\n  hypercom_client.exe "
                               "--server-key <64 caracteres hex> "
                               "[--host <adresse>] [--port <port>]\n\n"
                               "La passphrase se lit dans la variable "
                               "d'environnement HYPERCOM_PASSPHRASE.";
    MessageBoxA(nullptr, body.c_str(), "Hypercom",
                MB_OK | MB_ICONWARNING | MB_SETFOREGROUND);
#else
    std::cerr << message << '\n';
#endif
}

} // namespace hypercom::client
