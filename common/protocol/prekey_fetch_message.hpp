#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

#include <cstdint>

namespace hypercom::proto {

struct prekey_fetch_request {
    wire_public_key target_pubkey{};

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// The recipient verifies the signature with owner_pubkey before using the
// prekey. A server that swapped in its own prekey to place itself as an
// interceptor would fail to produce a valid signature -- it doesn't own
// the identity key of the targeted person.
struct prekey_bundle_response {
    wire_public_key owner_pubkey{};
    wire_public_key prekey{};
    wire_signature signature{};
    std::uint64_t created_at = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
