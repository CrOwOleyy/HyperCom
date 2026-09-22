#include "client/keystore/server_identity.hpp"

#include <algorithm>
#include <string_view>

#include "common/crypto/hkdf_sha256.hpp"
#include "common/crypto/secure_memory.hpp"

namespace hypercom::client {
namespace {

// Separation de domaine. Une graine qui servirait aussi ailleurs ne doit jamais
// produire la meme sortie qu'ici : ce sel fige l'usage.
constexpr std::string_view IDENTITY_DERIVATION_SALT =
    "hypercom-server-identity-v1";

[[nodiscard]] std::span<std::uint8_t const> as_bytes(std::string_view text)
{
    return {reinterpret_cast<std::uint8_t const *>(text.data()), text.size()};
}

} // namespace

bool derive_server_identity(crypto::ed25519_seed const &master_seed,
                            crypto::x25519_public_key const &server_key,
                            crypto::identity_keypair &out)
{
    crypto::symmetric_key pseudo_random_key{};
    if (!crypto::extract_pseudo_random_key(as_bytes(IDENTITY_DERIVATION_SALT),
                                           master_seed, pseudo_random_key)) {
        crypto::wipe_bytes(pseudo_random_key);
        return false;
    }
    crypto::symmetric_key derived{};
    bool succeeded =
        crypto::expand_key_block(pseudo_random_key, server_key, 1, derived);
    crypto::wipe_bytes(pseudo_random_key);
    if (succeeded) {
        // symmetric_key et ed25519_seed font tous deux 32 octets, mais restent
        // deux types distincts : la copie est explicite, comme dans
        // identity_store::unlock_identity.
        crypto::ed25519_seed seed{};
        std::copy(derived.begin(), derived.end(), seed.begin());
        succeeded = crypto::identity_keypair::derive_from_seed(seed, out);
        crypto::wipe_bytes(seed);
    }
    crypto::wipe_bytes(derived);
    return succeeded;
}

} // namespace hypercom::client
