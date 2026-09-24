#include "common/crypto/dm_session_keys.hpp"

#include "common/crypto/hkdf_sha256.hpp"
#include "common/crypto/secure_memory.hpp"
#include "common/crypto/x25519_exchange.hpp"

#include <algorithm>
#include <array>
#include <string_view>

namespace hypercom::crypto {
namespace {

constexpr std::string_view DM_SESSION_DOMAIN = "hypercom-dm-x3dh-v1";

[[nodiscard]] bool combine_shared_secrets(symmetric_key const &first,
                                          symmetric_key const &second,
                                          symmetric_key const &third,
                                          symmetric_key &out)
{
    std::array<std::uint8_t, SYMMETRIC_KEY_SIZE * 3> material{};
    auto const cursor = material.begin();
    std::copy(first.begin(), first.end(), cursor);
    std::copy(second.begin(), second.end(), cursor + SYMMETRIC_KEY_SIZE);
    std::copy(third.begin(), third.end(), cursor + (SYMMETRIC_KEY_SIZE * 2));
    symmetric_key pseudo_random{};
    std::span<std::uint8_t const> const domain{
        reinterpret_cast<std::uint8_t const *>(DM_SESSION_DOMAIN.data()),
        DM_SESSION_DOMAIN.size()};
    bool succeeded = extract_pseudo_random_key(domain, material, pseudo_random);
    if (succeeded) {
        succeeded = expand_key_block(pseudo_random, {}, 0x01, out);
    }
    wipe_bytes(material);
    wipe_bytes(pseudo_random);
    return succeeded;
}

} // namespace

bool derive_sender_session_key(identity_keypair const &sender_identity,
                               x25519_secret_key const &sender_ephemeral_secret,
                               ed25519_public_key const &recipient_identity,
                               x25519_public_key const &recipient_prekey,
                               symmetric_key &out)
{
    x25519_secret_key sender_identity_x{};
    x25519_public_key recipient_identity_x{};
    if (!convert_identity_secret_to_x25519(sender_identity.get_secret_key(),
                                           sender_identity_x) ||
        !convert_identity_public_to_x25519(recipient_identity,
                                           recipient_identity_x)) {
        wipe_bytes(sender_identity_x);
        return false;
    }
    symmetric_key first{};
    symmetric_key second{};
    symmetric_key third{};
    bool succeeded =
        compute_shared_secret(sender_identity_x, recipient_prekey, first) &&
        compute_shared_secret(sender_ephemeral_secret, recipient_identity_x,
                              second) &&
        compute_shared_secret(sender_ephemeral_secret, recipient_prekey, third);
    if (succeeded) {
        succeeded = combine_shared_secrets(first, second, third, out);
    }
    wipe_bytes(sender_identity_x);
    wipe_bytes(first);
    wipe_bytes(second);
    wipe_bytes(third);
    return succeeded;
}

bool derive_recipient_session_key(
    identity_keypair const &recipient_identity,
    x25519_secret_key const &recipient_prekey_secret,
    ed25519_public_key const &sender_identity,
    x25519_public_key const &sender_ephemeral, symmetric_key &out)
{
    x25519_secret_key recipient_identity_x{};
    x25519_public_key sender_identity_x{};
    if (!convert_identity_secret_to_x25519(recipient_identity.get_secret_key(),
                                           recipient_identity_x) ||
        !convert_identity_public_to_x25519(sender_identity,
                                           sender_identity_x)) {
        wipe_bytes(recipient_identity_x);
        return false;
    }
    symmetric_key first{};
    symmetric_key second{};
    symmetric_key third{};
    // Same three products as on the sender side, computed in the other order.
    bool succeeded =
        compute_shared_secret(recipient_prekey_secret, sender_identity_x,
                              first) &&
        compute_shared_secret(recipient_identity_x, sender_ephemeral, second) &&
        compute_shared_secret(recipient_prekey_secret, sender_ephemeral, third);
    if (succeeded) {
        succeeded = combine_shared_secrets(first, second, third, out);
    }
    wipe_bytes(recipient_identity_x);
    wipe_bytes(first);
    wipe_bytes(second);
    wipe_bytes(third);
    return succeeded;
}

} // namespace hypercom::crypto
