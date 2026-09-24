#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/error_code.hpp"

#include <cstdint>
#include <string>

namespace hypercom::proto {

// Generic success response. reference_id carries the identifier of the
// created object when there is one (forum, post, comment), 0 otherwise.
struct status_ok_response {
    std::uint64_t reference_id = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// detail is meant for the human. Hooking client logic onto this text
// would be a mistake: it can change, unlike code.
struct status_error_response {
    error_code code = error_code::internal_error;
    std::string detail;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
