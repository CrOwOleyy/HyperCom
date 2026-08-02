#include "common/crypto/signature_verifier.hpp"

#include <sodium.h>

namespace hypercom::crypto {

bool verify_signature(ed25519_public_key const &public_key,
                      std::span<std::uint8_t const> message,
                      ed25519_signature const &signature)
{
    return crypto_sign_verify_detached(signature.data(), message.data(),
                                       message.size(), public_key.data())
        == 0;
}

} // namespace hypercom::crypto
