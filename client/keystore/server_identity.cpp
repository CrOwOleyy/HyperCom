#include "client/keystore/server_identity.hpp"

#include "common/crypto/hkdf_sha256.hpp"
#include "common/crypto/secure_memory.hpp"

#include <algorithm>
#include <string_view>

namespace hypercom::client {
namespace {

// Domain separation. A seed that's also used elsewhere must never produce
// the same output as it does here: this salt locks down the usage.
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
        // symmetric_key and ed25519_seed are both 32 bytes, but remain
        // distinct types: the copy is explicit, as in
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
