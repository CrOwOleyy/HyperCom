#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/forum_record.hpp"

#include <string>
#include <string_view>

namespace hypercom::proto {

// Anyone can create a forum and becomes its founder. The server only checks
// the name's uniqueness and shape -- there's no list of forbidden names,
// and none is planned.
struct forum_create_request {
    std::string name;
    std::string description;
    std::string theme_json;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct forum_info_response {
    forum_record forum;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Same logic as validate_handle: restricted ASCII, so that two forums can't
// carry visually identical names.
[[nodiscard]] bool validate_forum_name(std::string_view name);

} // namespace hypercom::proto
