#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

#include <cstdint>

namespace hypercom::proto {

// Removal of one's own content. The target is designated only by its
// identifier: the author is NOT a field of the message, it comes from the
// authenticated session. A sender therefore cannot claim authorship of
// something they didn't write.
//
// The response is status_ok or status_error, with no dedicated structure:
// there's nothing to return besides an acknowledgment.

struct post_delete_request {
    std::uint64_t post_id = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct comment_delete_request {
    std::uint64_t comment_id = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
