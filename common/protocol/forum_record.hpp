#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

#include <cstdint>
#include <string>

namespace hypercom::proto {

// A forum as it travels over the wire.
//
// The founder is designated by their public key rather than a database
// identifier, so the client can verify who they're dealing with without
// depending on a lookup table controlled by the server.
struct forum_record {
    std::uint64_t id = 0;
    std::string name;
    std::string description;
    wire_public_key founder_pubkey{};
    std::string founder_handle;
    std::string theme_json;
    std::uint64_t created_at = 0;
    std::uint32_t post_count = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
