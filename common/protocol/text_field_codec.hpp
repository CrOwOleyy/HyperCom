#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace hypercom::proto {

// Text field: [u32 size][UTF-8 bytes].
//
// The same u32 prefix serves both text and blobs. Two extra bytes per
// string, but a single decoding path to audit and fuzz.

// Whatever the server accepts, it will serve back as-is to other
// clients. Hence the rejection of:
//   - invalid, overlong, or out-of-plane Unicode UTF-8
//   - surrogate code points (U+D800..U+DFFF)
//   - C0 controls, except tab and line feed
//   - carriage return, to stay consistent with rule G6
[[nodiscard]] bool validate_text_field(std::string_view text);

// Reads, bounds-checks, validates. Only writes the output if all three
// succeed.
[[nodiscard]] bool read_text_field(byte_reader &reader, std::string &out,
                                   std::size_t maximum_length);

void write_text_field(byte_writer &writer, std::string_view text);

} // namespace hypercom::proto
