#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

#include <cstdint>
#include <vector>

namespace hypercom::proto {

// Everything the server keeps about a private message.
//
// The internal ciphertext format is described in common/crypto/dm_envelope,
// not here: the server has no key to open it, and the Poly1305 tag makes
// any tampering detectable.
//
// Known limitation: it still sees who writes to whom, and when. Hiding that
// would require a mix-net -- out of scope for v1, see docs/THREAT_MODEL.md.
// No timestamp: the send date lives inside the ciphertext, so the server
// keeps no timestamped log of exchanges. The client recovers it by
// decrypting.
struct dm_envelope_record {
    std::uint64_t id = 0;
    wire_public_key sender_pubkey{};
    std::vector<std::uint8_t> ciphertext;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
