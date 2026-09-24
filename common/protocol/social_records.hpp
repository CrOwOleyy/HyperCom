#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

#include <cstdint>
#include <string>

namespace hypercom::proto {

// banner_reference designates a blob by its hash, never by a server URL.
// In v1 the field is transmitted and stored but nothing is hosted -- this
// is what will allow wiring up P2P in v2 without a migration or format
// change.
struct profile_record {
    wire_public_key pubkey{};
    std::string handle;
    std::string display_name;
    std::string bio;
    std::string theme_json;
    std::string banner_reference;
    // No last_seen, no created_at: the server records neither the last
    // login nor an account's age, so the protocol has no field to carry
    // them. See migrations 0003 and 0004.

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

enum class friendship_status : std::uint8_t {
    requested = 0,
    accepted = 1,
    blocked = 2,
};

struct friend_record {
    wire_public_key pubkey{};
    std::string handle;
    std::string display_name;
    friendship_status status = friendship_status::requested;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
