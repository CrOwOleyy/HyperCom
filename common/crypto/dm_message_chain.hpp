#pragma once

#include "common/crypto/key_types.hpp"

#include <cstdint>
#include <vector>

namespace hypercom::crypto {

// Maximum number of messages we accept to skip at once. Otherwise an
// envelope announcing a far-off counter would force millions of
// derivations: a one-byte denial of service.
constexpr std::uint32_t MAX_SKIPPED_MESSAGE_KEYS = 1000;

// Symmetric ratchet. The chain key advances with every message and the
// previous state is wiped:
//
//   mk_i     = HKDF(ck_i, "message")
//   ck_{i+1} = HKDF(ck_i, "chain")      then ck_i is destroyed
//
// Concrete consequence: someone who seizes the machine today and obtains
// ck_n can NOT read back messages 0..n-1. This is the symmetric forward
// secrecy promised by BRIEF.md 6.
//
// Known v1 limitation: the full Diffie-Hellman ratchet, which would also
// protect FUTURE messages after a compromise, is coming in v2. The envelope
// format is already ready to accommodate it.
class dm_message_chain {
public:
    explicit dm_message_chain(symmetric_key const &initial_chain_key);

    // Advances by one step and returns the current message's key.
    [[nodiscard]] bool derive_next_message_key(symmetric_key &out);

    // Catches up on messages that arrived out of order. The skipped keys are
    // returned to the caller, who is responsible for keeping them as long as
    // needed.
    [[nodiscard]] bool advance_to_counter(std::uint32_t target,
                                          std::vector<symmetric_key> &skipped);

    [[nodiscard]] std::uint32_t get_counter() const;

    // For client-side persistence only. Whatever comes out of here must be
    // stored encrypted, never in plaintext.
    [[nodiscard]] symmetric_key const &get_chain_key() const;

private:
    symmetric_key chain_key_;
    std::uint32_t counter_;
};

} // namespace hypercom::crypto
