#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

#include <string>

namespace hypercom::proto {

// The customizable profile, the MySpace side of the project.
//
// theme_json is stored and served back as-is. The server doesn't
// interpret it and only validates UTF-8 and size -- so it's up to the
// client to defend itself against a hostile theme.
struct profile_set_request {
    std::string display_name;
    std::string bio;
    std::string theme_json;
    std::string banner_reference;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
