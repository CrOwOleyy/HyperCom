#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

#include <string>
#include <string_view>

namespace hypercom::proto {

// Account creation, sent after authentication: the challenge signature
// has already proven possession of the private key, all that's left is
// choosing a handle. No email, no phone, no password.
struct register_request {
    std::string handle;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Restricted ASCII: letters, digits, hyphen, underscore. Between 3 and
// MAX_HANDLE_LENGTH characters, must not start with a digit.
//
// The restriction exists to block homoglyphs. An "alice" written in
// Cyrillic displays identically but designates a different account, and
// here nobody is around to arbitrate an impersonation.
[[nodiscard]] bool validate_handle(std::string_view handle);

} // namespace hypercom::proto
