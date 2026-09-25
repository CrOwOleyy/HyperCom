#pragma once

#include "common/crypto/identity_keypair.hpp"
#include "common/crypto/key_types.hpp"

namespace hypercom::crypto {

// Simplified X3DH. Three Diffie-Hellman exchanges, one secret:
//
//   DH1 = DH(IK_sender, SPK_recipient)     authenticates the sender
//   DH2 = DH(EK_sender, IK_recipient)      authenticates the recipient
//   DH3 = DH(EK_sender, SPK_recipient)     contributes the ephemeral part
//
// root = HKDF(DH1 || DH2 || DH3)
//
// Dropping DH1 would let the sender be impersonated; dropping DH2 would let
// anyone holding a stolen prekey impersonate the recipient; dropping DH3
// would tie the session to long-lived keys. All three are necessary, none
// is decorative.
//
// The two functions compute exactly the same value: only the roles of the
// keys are swapped. This property is what makes the protocol usable
// without any prior round trip -- the sender can write to someone who is
// offline.

[[nodiscard]] bool
derive_sender_session_key(identity_keypair const &sender_identity,
                          x25519_secret_key const &sender_ephemeral_secret,
                          ed25519_public_key const &recipient_identity,
                          x25519_public_key const &recipient_prekey,
                          symmetric_key &out);

[[nodiscard]] bool
derive_recipient_session_key(identity_keypair const &recipient_identity,
                             x25519_secret_key const &recipient_prekey_secret,
                             ed25519_public_key const &sender_identity,
                             x25519_public_key const &sender_ephemeral,
                             symmetric_key &out);

} // namespace hypercom::crypto
